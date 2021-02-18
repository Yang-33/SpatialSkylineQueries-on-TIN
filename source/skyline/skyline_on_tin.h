#ifndef SOURCE_SKYLINE_SKYLINE_ON_TIN_H_
#define SOURCE_SKYLINE_SKYLINE_ON_TIN_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <glog/logging.h>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "source/index/surface_index.h"
#include "source/skyline/dominance_dist_helper.h"
#include "source/skyline/meshgraph.h"
#include "source/skyline/minimum_bounding_box.h"
#include "source/util/enumurateref.h"
#include "source/util/vector.h"

namespace skyline {

namespace naive {

int naive_the_number_of_calculating_ds = 0;

int ds_counter() {
  return naive_the_number_of_calculating_ds;
}

bool is_dominated(int pnew,
                  MeshGraph& meshgraph,
                  const std::vector<int>& ps,
                  const std::vector<int>& qs) {
  bool dominated = false;
  for (auto p : ps) {
    if (p == pnew)
      continue;
    bool pnew_is_always_bigger_than_p = true;
    for (auto q : qs) {
      pnew_is_always_bigger_than_p &=
          (meshgraph.surface_distance(pnew, q,
                                      &naive_the_number_of_calculating_ds) >
           meshgraph.surface_distance(p, q,
                                      &naive_the_number_of_calculating_ds));
    }
    dominated |= pnew_is_always_bigger_than_p;
  }
  return dominated;
}

std::vector<int> solve_skyline_on_tin(MeshGraph& meshgraph,
                                      const std::vector<int>& ps,
                                      const std::vector<int>& qs) {
  std::vector<int> ans;
  for (auto pnew : ps) {
    if (is_dominated(pnew, meshgraph, ps, qs))
      continue;
    ans.push_back(pnew);
  }

  std::sort(ans.begin(), ans.end());

  LOG(INFO) << "|ps| := " << ps.size();
  LOG(INFO) << "|qs| := " << qs.size();
  LOG(INFO) << "|skyline| := " << ans.size();
  LOG(INFO) << "naive::skyline := " << util::vec_to_string(ans);
  LOG(INFO) << "naive::counter::ds := " << ds_counter();
  // CHECK_EQ(ds_counter(), ps.size() * (2 * (ps.size() - 1) * qs.size()))
  //     << "naive counter must be |p|*(2*(|p|-1)*|q|)";
  CHECK_EQ(ds_counter(), ps.size() * qs.size())
      << "naive counter must be |ps|*|qs|)";

  return ans;
}

}  // namespace naive

namespace fast {

std::vector<int> reorder_ps(DominanceDistHelper& dominance_checker,
                            const std::vector<int>& ps,
                            const std::vector<int>& qs,
                            const int reorder_flag) {
  auto p_sort_by_dist = [&](int q) {
    std::vector<std::tuple<double, double, int>> xs;
    for (auto p : ps) {
      xs.push_back({dominance_checker.euclid_distance(p, q),
                    dominance_checker.network_distance(p, q), p});
    }
    std::sort(xs.begin(), xs.end());

    std::vector<int> porder;
    for (auto [dl, du, id] : xs) {
      porder.push_back(id);
    }
    return porder;
  };

  // the number of segment, |q|
  std::vector<std::pair<int, int>> info;

  for (auto q : qs) {
    std::vector<int> p_order = p_sort_by_dist(q);

    int segment_count = 1;
    double seg_up = dominance_checker.network_distance(p_order.front(), q);

    for (auto p : p_order) {
      double low = dominance_checker.euclid_distance(p, q);
      double up = dominance_checker.network_distance(p, q);
      if (seg_up < low) {
        ++segment_count;
      }
      seg_up = std::max(seg_up, up);
    }
    info.push_back({segment_count, q});
  }

  std::sort(info.rbegin(), info.rend());
  int selected_q = info.front().second;
  if (reorder_flag == 2) {
    return p_sort_by_dist(selected_q);
  }
  LOG(INFO) << "ds in reorder := " << ps.size() - info.front().first + 1;

  {
    std::vector<int> order = p_sort_by_dist(selected_q);
    // Use q, create groups
    std::vector<std::vector<int>> segments_group;
    segments_group.push_back({});
    double seg_up =
        dominance_checker.network_distance(order.front(), selected_q);
    for (auto p : order) {
      double low = dominance_checker.euclid_distance(p, selected_q);
      double up = dominance_checker.network_distance(p, selected_q);
      if (seg_up < low) {
        segments_group.push_back({p});
      } else {
        segments_group.back().push_back(p);
      }
      seg_up = std::max(seg_up, up);
    }

    std::vector<std::pair<double, int>> hoge;
    for (auto p_group : segments_group) {
      if (p_group.size() == 1) {
        hoge.push_back(
            {dominance_checker.euclid_distance(p_group.front(), selected_q),
             p_group.front()});
      } else {
        // TODO :
        // in group, if it can't dominate each other, we shouldn't calc ds.
        for (auto p : p_group) {
          hoge.push_back(
              {dominance_checker.surface_distance(p, selected_q), p});
        }
      }
    }
    // sort by q;
    std::sort(hoge.begin(), hoge.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    std::vector<int> ans;
    for (auto pi : hoge) {
      ans.push_back(pi.second);
    }
    return ans;
  }
}

std::vector<int> solve_skyline_on_tin(MeshGraph& meshgraph,
                                      const std::vector<int>& ps,
                                      const std::vector<int>& qs,
                                      const int reorder_flag,
                                      const bool fast_lsi_flag,
                                      const bool useTSILSI,
                                      const bool useSibori,
                                      const bool useBB,
                                      const bool useIneq,
                                      const bool useNewLB,
                                      const bool useAll) {
  std::set<int> skyline;
  DominanceDistHelper dominance_checker(meshgraph, ps, qs);

  // q in LSI ?
  //   check out inequality before calc Ds corresponding all candidates
  //   calc Ds to find argmin
  if (useTSILSI) {
    // q in TSI ?
    surfaceindex::SurfaceIndexConstructor surface_index(meshgraph, ps, qs);
    std::vector<int> tsi_points = surface_index.points_in_tsi();
    skyline = std::set<int>{tsi_points.begin(), tsi_points.end()};
    LOG(INFO) << " TSI result size := " << skyline.size();


    auto lsi_candidates = surface_index.points_in_lsi();
    for (auto [lsi_candidate, q] : lsi_candidates) {
      if (fast_lsi_flag) {
        int nn_id = dominance_checker.nearest_search(q, lsi_candidate);
        skyline.insert(nn_id);
        LOG(INFO) << "LSI : add " << nn_id;
      } else {
        LOG(INFO) << "LSI INFO size:=" << lsi_candidate.size()
                  << ", q := " << q;
        int id = lsi_candidate.front();
        double ds = dominance_checker.surface_distance(id, q);
        for (auto p : lsi_candidate) {
          if (ds > dominance_checker.surface_distance(p, q)) {
            id = p;
            ds = dominance_checker.surface_distance(p, q);
          }
        }
        skyline.insert(id);
      }
    }
  }

  // p in MBR ?
  //   dominance check with inequality
  //   dominance check with Ds
  using MBR = Box;
  MBR mbr;
  mbr.update_minimum_bounding_box(meshgraph, {skyline.begin(), skyline.end()},
                                  qs);

  const std::vector<int> reordered_ps =
      (reorder_flag && useAll
           ? reorder_ps(dominance_checker, ps, qs, reorder_flag)
           : ps);

  std::vector<int> shibotta_ps = reordered_ps;
  if (useSibori) {
    // TODO: Use Rtree..
    shibotta_ps.clear();
    for (auto p : reordered_ps) {
      if (mbr.is_point_in_minimum_bounding_box(meshgraph, p)) {
        shibotta_ps.push_back(p);
      }else{
        // LOG(INFO) << "use mbr pruning";
      }
    }
  }

  if (!useBB) {
    mbr.disable();
  }
  if (!useIneq) {
    dominance_checker.disable_ineq();
  }
  if (!useNewLB) {
    dominance_checker.disable_newLB();
  }

  for (auto p : shibotta_ps) {
    if (skyline.count(p)) {
      continue;
    }
    if (!mbr.is_point_in_minimum_bounding_box(meshgraph, p)) {
      // LOG(INFO) << "use mbr pruning in loop";
      continue;
    }

    if (dominance_checker.is_dominated_with_inequality(p, skyline, qs)) {
      // LOG(INFO) << "use ineq";
      continue;
    }

    if (dominance_checker.is_not_dominated_with_inequality(p, skyline, qs) ||
        dominance_checker.is_not_dominated_with_surface_distance(p, skyline,
                                                                 qs)) {
      if (reorder_flag == 0 || reorder_flag == 2) {
        std::vector<int> dominated_points =
            dominance_checker.dominated_skylines_by_p(p, skyline, qs);
        for (auto dominated_point : dominated_points) {
          skyline.erase(dominated_point);
        }
      }
      skyline.insert(p);
      mbr.update_minimum_bounding_box(meshgraph, p, qs);
    }
  }

  std::vector<int> ans(skyline.begin(), skyline.end());

  LOG(INFO) << "|ps| := " << ps.size();
  LOG(INFO) << "|qs| := " << qs.size();
  LOG(INFO) << "|skyline| := " << ans.size();
  LOG(INFO) << "fast::skyline := " << util::vec_to_string(ans);
  LOG(INFO) << "fast::counter::ds := " << dominance_checker.ds_counter()
            << " / " << ps.size() * qs.size() << " ("
            << (double)dominance_checker.ds_counter() * 100 /
                   (ps.size() * qs.size())
            << " %)";
  return ans;
}

}  // namespace fast

}  // namespace skyline

#endif  // SOURCE_SKYLINE_SKYLINE_ON_TIN_H_
