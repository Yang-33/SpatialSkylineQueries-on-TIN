#ifndef SOURCE_QUERYPOINT_GENERATOR_H_
#define SOURCE_QUERYPOINT_GENERATOR_H_

#include <glog/logging.h>
#include <fstream>
#include <vector>

#include "source/skyline/meshgraph.h"
#include "source/util/enumurateref.h"
#include "source/util/id_io.h"

namespace querypoint {

double calcxyarea(const std::vector<int>& mbr_ids,
                  skyline::MeshGraph& meshgraph) {
  const double inf = 1e18;
  double max_x = -inf, min_x = inf;
  double max_y = -inf, min_y = inf;

  for (auto p : meshgraph.ids_to_points(mbr_ids)) {
    max_x = std::max(max_x, p.x());
    min_x = std::min(min_x, p.x());
    max_y = std::max(max_y, p.y());
    min_y = std::min(min_y, p.y());
  }
  return (max_x - min_x) * (max_y - min_y);
}

bool filled_mbr_percentage(std::vector<int>& mbrs,
                           const int new_id,
                           const double mbr_q_percentage,
                           skyline::MeshGraph& meshgraph,
                           const double area) {
  double area_upper = area * mbr_q_percentage / 100;
  mbrs.push_back(new_id);
  double new_area = calcxyarea(mbrs, meshgraph);
  mbrs.pop_back();

  return new_area <= area_upper;
}

std::vector<int> create_id_from_rand_permutation_with_percentage(
    const int select,
    const int seq_size,
    const double mbr_q_percentage,
    skyline::MeshGraph& meshgraph,
    bool qposition_square) {
  std::vector<int> ids = util::rand_permutation(seq_size);

  std::vector<int> all_id(meshgraph.tin_point_size());
  std::iota(all_id.begin(), all_id.end(), 0);

  double area = calcxyarea(all_id, meshgraph);
  LOG(INFO) << "area := " << area;

  // |select|
  std::vector<int> ans;
  ans.push_back(ids.front());

  if (qposition_square) {
    // sort by distance from |ans.front()|
    std::sort(ids.begin(), ids.end(), [&](const int& a, const int& b) {
      double da = meshgraph.euclid_distance(ans.front(), a);
      double db = meshgraph.euclid_distance(ans.front(), b);
      return da < db;
    });
    // limit by distance <= 2*sqrt(area*percentage)
    double upper_distance = 2 * std::sqrt(area * mbr_q_percentage / 100);
    for (auto [i, id] : util::enumerateref(ids)) {
      if (meshgraph.euclid_distance(ans.front(), id) > upper_distance) {
        ids.resize(i + 1);
        ids.shrink_to_fit();
        break;
      }
    }
    // shuffule sequence
    ids = util::shuffle_seq(ids);
  }

  {  // try adding ids to Q
    for (int i = 1; (i < (int)ids.size() && (int)ans.size() < select); ++i) {
      if (ids[i] == ans.front())
        continue;
      if (filled_mbr_percentage(ans, ids[i], mbr_q_percentage, meshgraph,
                                area)) {
        ans.push_back(ids[i]);
      }
    }
  }
  CHECK_EQ(ans.size(), select)
      << "Please run again after changing random seed.";
  return ans;
}

std::vector<int> create_or_read_querypoint(const std::string& filepath,
                                           const int size,
                                           const int tin_size,
                                           const double mbr_q_percentage,
                                           skyline::MeshGraph& meshgraph,
                                           bool forceupdate,
                                           bool qposition_square) {
  if (std::ifstream ifs(filepath); !ifs.is_open() || forceupdate) {
    std::vector<int> ids = create_id_from_rand_permutation_with_percentage(
        size, tin_size, mbr_q_percentage, meshgraph, qposition_square);
    util::write_id(filepath, ids);
    LOG(INFO) << filepath << " has been updated.";
  } else {
    LOG(INFO) << filepath << " exists.";
  }
  return util::read_id(filepath);
}

}  // namespace querypoint

#endif  // SOURCE_QUERYPOINT_GENERATOR_H_
