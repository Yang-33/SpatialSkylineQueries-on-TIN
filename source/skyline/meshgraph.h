#ifndef SOURCE_SKYLINE_MESHGRAPH_H_
#define SOURCE_SKYLINE_MESHGRAPH_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh_shortest_path.h>
#include <CGAL/squared_distance_3.h>
#include <assert.h>
#include <glog/logging.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "source/skyline/unionfind.h"
#include "source/skyline/writeoff.h"

namespace skyline {

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Triangle_mesh;
typedef CGAL::Surface_mesh_shortest_path_traits<Kernel, Triangle_mesh> Traits;
typedef CGAL::Surface_mesh_shortest_path<Traits> Surface_mesh_shortest_path;
typedef boost::graph_traits<Triangle_mesh> Graph_traits;
typedef Graph_traits::vertex_descriptor vertex_descriptor;
typedef Graph_traits::vertex_iterator vertex_iterator;

class MeshGraph {
 public:
  MeshGraph(const std::string& filepath, bool use_memo)
      : prebuilt_(false), use_memo_(use_memo), use_tin_filter_(false) {
    // check valid mesh
    std::ifstream input(filepath);
    if (!input || !(input >> mesh_) || mesh_.is_empty()) {
      assert(false && "not valid off file.");
    }

    // pre build
    create_vid_index();
    create_mesh_graph();
    create_face_edges();
    create_min_theta();
  }

  void prebuild(const std::vector<int>& network_sources,
                const std::vector<int>& qs) {
    precalc_network_distance(network_sources, qs);
    prebuilt_ = true;
  }

  void clear_cache() { surface_distance_memo_.clear(); }

  // N^2, Called (p, q, pointer::counter)
  std::pair<double, std::vector<Traits::Point_3>>
  surface_distance(int from, int to, int* counter) {
    // experimental
    if (use_memo_ && surface_distance_memo_.find({from, to}) !=
                         surface_distance_memo_.end()) {
      return {surface_distance_memo_[{from, to}], {}};
    }
    ++(*counter);
    std::pair<double,
              skyline::Surface_mesh_shortest_path::Source_point_iterator>
        res;
    std::vector<Traits::Point_3> points;

    if (use_tin_filter_) {
      auto [filtered_mesh, from_reid_vd, to_reid_vd] =
          take_filtered_mesh({from}, to);
      Surface_mesh_shortest_path shortest_paths(std::move(filtered_mesh));
      shortest_paths.add_source_point(from_reid_vd.front());
      res = shortest_paths.shortest_path_points_to_source_points(
          to_reid_vd, std::back_inserter(points));
    } else {
      Surface_mesh_shortest_path shortest_paths(mesh_);
      shortest_paths.add_source_point(id_to_vid(from));
      res = shortest_paths.shortest_path_points_to_source_points(
          id_to_vid(to), std::back_inserter(points));
    }
    if (use_memo_) {
      surface_distance_memo_[{from, to}] = res.first;
    }
    return {res.first, points};
  }

  // N^2,
  int nearest_search(int q, const std::vector<int>& ps, int* counter) {
    ++(*counter);
    std::pair<double,
              skyline::Surface_mesh_shortest_path::Source_point_iterator>
        res;
    std::vector<Traits::Point_3> points;

    if (use_tin_filter_) {
      auto [filtered_mesh, from_reid_vds, to_reid_vd] =
          take_filtered_mesh(ps, q);
      Surface_mesh_shortest_path shortest_paths(std::move(filtered_mesh));
      for (auto from_reid_vd : from_reid_vds) {
        shortest_paths.add_source_point(from_reid_vd);
      }
      res = shortest_paths.shortest_path_points_to_source_points(
          to_reid_vd, std::back_inserter(points));
    } else {
      Surface_mesh_shortest_path shortest_paths(mesh_);
      assert(shortest_paths.number_of_source_points() == 0);

      for (auto from : ps) {
        shortest_paths.add_source_point(id_to_vid(from));
      }
      res = shortest_paths.shortest_path_points_to_source_points(
          id_to_vid(q), std::back_inserter(points));
    }

    // Estimate shortest path p:
    int from = -1;
    for (auto p : ps) {
      auto point_p = id_to_point(p);
      if (points.front() == point_p || points.back() == point_p) {
        from = p;
        break;
      }
    }
    assert(from != -1);

    if (use_memo_) {
      surface_distance_memo_[{from, q}] = res.first;
    }
    return from;
  }

  // log N
  // Calc point to point distance
  double euclid_distance(int from, int to) {
    auto p1 = id_to_point(from);
    auto p2 = id_to_point(to);
    auto dist = std::sqrt(CGAL::squared_distance(p1, p2));
    return dist;
  }

  // 1
  // Return memo
  double network_distance(int p, int q) {
    assert(prebuilt_ == true);
    return network_distance_[p][q];
  }

  // deg(center)
  // Show adjacent vertexes from |Graph|
  std::vector<int> adjacent_vertexes(int center) { return graph_[center]; }

  // log N
  // TODO: speed up by improving complexity from log N to 1
  CGAL::Epick::Point_3 id_to_point(int id) {
    vertex_iterator vit;
    *vit = id_to_vid(id);
    return mesh_.point(*vit);
  }

  std::vector<CGAL::Epick::Point_3> ids_to_points(const std::vector<int>& ids) {
    std::vector<CGAL::Epick::Point_3> res;
    for (auto id : ids) {
      res.push_back(id_to_point(id));
    }
    return res;
  }

  std::vector<CGAL::Epick::Point_3> all_points() {
    std::vector<int> ids(N_);
    std::iota(ids.begin(), ids.end(), 0);
    return ids_to_points(ids);
  }

  int tin_point_size() const { return N_; }
  void use_tin_filter() { use_tin_filter_ = true; }

  double calc_theta_min() const { return theta_min_; }

 private:
  void precalc_network_distance_internal(const std::vector<int>& sources) {
    const long long DINF = 1e18;
    using Box = std::pair<double, int>;
    for (auto source : sources) {
      std::vector<double>& dist = network_distance_[source];
      dist = std::vector<double>(N_, DINF);
      std::priority_queue<Box, std::vector<Box>, std::greater<Box>> queue;

      auto IFPUSH = [&](const Box& box) {
        double cost = box.first;
        int v = box.second;
        if (dist[v] > cost) {
          dist[v] = cost;
          queue.push(Box(cost, v));
        }
      };

      IFPUSH(Box(0, source));
      while (!queue.empty()) {
        double d = queue.top().first;
        int v = queue.top().second;
        queue.pop();
        if (dist[v] < d) {
          continue;
        }
        for (int nx : graph_[v]) {
          double add_cost = euclid_distance(v, nx);
          IFPUSH(Box(d + add_cost, nx));
        }
      }
    }
  }
  // |sources| N log N
  void precalc_network_distance(const std::vector<int>& sources,
                                const std::vector<int>& qs) {
    network_distance_ = std::vector<std::vector<double>>(N_);
    precalc_network_distance_internal(sources);
    precalc_network_distance_internal(qs);
  }

  void create_vid_index() {
    vertex_iterator vit, vit_end;
    for (boost::tie(vit, vit_end) = vertices(mesh_); vit != vit_end; ++vit) {
      id_to_vid_.push_back(*vit);
    }
    N_ = (int)id_to_vid_.size();
  }

  void create_mesh_graph() {
    UnionFind connectivity(N_);
    graph_ = std::vector<std::vector<int>>(N_);
    for (int i = 0; i < (int)id_to_vid_.size(); ++i) {
      vid_to_id_[id_to_vid(i)] = i;
    }
    for (auto v : id_to_vid_) {
      CGAL::Vertex_around_target_circulator<Triangle_mesh> vbegin(
          mesh_.halfedge(v), mesh_),
          done(vbegin);
      int from = vid_to_id_[v];
      do {
        int to = vid_to_id_[*vbegin];
        graph_[from].push_back(to);
        connectivity.unionSet(from, to);
        *vbegin++;
      } while (vbegin != done);
    }
    CHECK_EQ(connectivity.component_size(), 1);
  }

  void create_face_edges() {
    for (Triangle_mesh::Face_index face_index : mesh_.faces()) {
      CGAL::Vertex_around_face_circulator<Triangle_mesh> vcirc(
          mesh_.halfedge(face_index), mesh_),
          done(vcirc);
      std::vector<int> edgeids;
      do {
        int id = vid_to_id_[*vcirc];
        edgeids.push_back(id);
        *vcirc++;
      } while (vcirc != done);
      face_edge_.push_back(edgeids);
    }
  }

  void create_min_theta() {
    auto calc_theta_cosine_theorem = [](const CGAL::Epick::Point_3& a,
                                        const CGAL::Epick::Point_3& b,
                                        const CGAL::Epick::Point_3& c) {
      double edge_a = std::sqrt(CGAL::squared_distance(b, c));
      double edge_b = std::sqrt(CGAL::squared_distance(c, a));
      double edge_c = std::sqrt(CGAL::squared_distance(a, b));

      double cosA = (edge_b * edge_b + edge_c * edge_c - edge_a * edge_a) /
                    (2 * edge_b * edge_c);
      return std::acos(cosA);
    };

    double theta_min = 3.14;

    for (auto edge_ids : face_edge_) {
      auto a = id_to_point(edge_ids[0]);
      auto b = id_to_point(edge_ids[1]);
      auto c = id_to_point(edge_ids[2]);

      double theta_a = calc_theta_cosine_theorem(a, b, c);
      double theta_b = calc_theta_cosine_theorem(b, c, a);
      double theta_c = calc_theta_cosine_theorem(c, a, b);
      theta_min = std::min({theta_min, theta_a, theta_b, theta_c});
    }
    theta_min_ = theta_min;
    if (false) {
      double lambda = std::min(
          {std::sin(theta_min) / 2, std::sin(theta_min), std::cos(theta_min)});
    }
  }

  // N, Called (p, q)
  std::tuple<Triangle_mesh, std::vector<vertex_descriptor>, vertex_descriptor>
  take_filtered_mesh(const std::vector<int>& ps, int q) {
    std::map<int, bool> used_point;
    auto usecheck = [&](int v) {
      return used_point.find(v) != used_point.end();
    };

    // Take point |k| that fills Dn(p, k) <= Dn(p,q)
    {
      for (auto p : ps) {
        double upper = network_distance(p, q);
        for (int k = 0; k < N_; ++k) {
          if (network_distance(p, k) <= upper &&
              network_distance(q, k) <= upper) {
            used_point[k] = true;
          }
        }
      }
    }

    // EDGE : take edge id
    std::vector<std::tuple<int, int, int>> edges;
    {
      for (auto edgeids : face_edge_) {
        if (usecheck(edgeids[0]) || usecheck(edgeids[1]) ||
            usecheck(edgeids[2])) {
          edges.push_back({edgeids[0], edgeids[1], edgeids[2]});
        }
      }
    }

    // EDGE : adjust
    for (auto [a, b, c] : edges) {
      used_point[a] = used_point[b] = used_point[c] = true;
    }

    // EDGE : compress point id
    // POINT : write point(x,y,z) by ascending order
    WriteOFF output("temp.off");
    std::map<int, int> reid;
    {
      int id = 0;
      for (int i = 0; i < N_; ++i) {
        if (usecheck(i)) {
          reid[i] = id++;
          output.add_point(id_to_point(i));
        }
      }
    }

    // EDGE : write temporary file
    for (auto [a, b, c] : edges) {
      int ra = reid[a];
      int rb = reid[b];
      int rc = reid[c];
      output.add_face({ra, rb, rc});
    }
    output.finish();

    // MESH : read this file as triangle_mesh, then return it.
    std::ifstream input("temp.off");
    Triangle_mesh mesh_ret;
    if (!input || !(input >> mesh_ret) || mesh_ret.is_empty()) {
      std::cerr << "Not a valid off file." << std::endl;
    }
    input.close();

    // MESH : return vertex_descriptor corresponding with |p| and |q|

    std::vector<vertex_descriptor> p_reid_vds;
    vertex_descriptor q_reid_vd;
    {
      vertex_iterator vit, vit_end;
      std::vector<vertex_descriptor> id_to_vid_for_filtered_mesh;
      for (boost::tie(vit, vit_end) = vertices(mesh_); vit != vit_end; ++vit) {
        id_to_vid_for_filtered_mesh.push_back(*vit);
      }
      for (auto p : ps) {
        int p_reid = reid[p];
        p_reid_vds.push_back(id_to_vid_for_filtered_mesh[p_reid]);
      }

      int q_reid = reid[q];
      q_reid_vd = id_to_vid_for_filtered_mesh[q_reid];
    }
    return {mesh_ret, p_reid_vds, q_reid_vd};
  }

  vertex_descriptor id_to_vid(int id) const { return id_to_vid_[id]; }
  int vid_to_id(const vertex_descriptor& vid) { return vid_to_id_[vid]; }

  Triangle_mesh mesh_;
  std::vector<vertex_descriptor> id_to_vid_;
  std::map<vertex_descriptor, int> vid_to_id_;

  int N_;
  std::vector<std::vector<int>> graph_;
  std::vector<std::vector<int>> face_edge_;
  std::vector<std::vector<double>> network_distance_;
  bool prebuilt_;

  double theta_min_;

  bool use_memo_;
  bool use_tin_filter_;
  std::map<std::pair<int, int>, double> surface_distance_memo_;
};

}  // namespace skyline

#endif  // SOURCE_SKYLINE_MESHGRAPH_H_
