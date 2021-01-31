#ifndef SOURCE_SKYLINE_VISUALIZE_H_
#define SOURCE_SKYLINE_VISUALIZE_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <glog/logging.h>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "source/skyline/meshgraph.h"
#include "source/skyline/writeoff.h"

namespace vizualize {

// a \ b
std::vector<int> vector_exclude(std::vector<int> a, std::vector<int> b) {
  std::vector<int> res;
  std::sort(b.begin(), b.end());
  for (auto e : a) {
    auto it = std::lower_bound(b.begin(), b.end(), e);
    if (e == *it)
      continue;
    res.push_back(e);
  }
  return res;
}

void vizualize(const std::string& outfile,
               skyline::MeshGraph& meshgraph,
               const std::vector<int>& skyline_points,
               const skyline::RGBA& skyline_color,
               const std::vector<int>& ps,
               const skyline::RGBA& ps_color,
               const std::vector<int>& qs,
               const skyline::RGBA& qs_color,
               const bool sheped_mode) {
  skyline::WriteOFF outputOFF(outfile, true);

  // skyline
  std::vector<CGAL::Epick::Point_3> skyline_point_on_tin =
      meshgraph.ids_to_points(skyline_points);
  outputOFF.add_points_as_block(skyline_point_on_tin, skyline_color,
                                sheped_mode);

  // p
  std::vector<CGAL::Epick::Point_3> ps_cgal_point =
      meshgraph.ids_to_points(vector_exclude(ps, skyline_points));
  outputOFF.add_points_as_block(ps_cgal_point, ps_color, sheped_mode);

  // q
  std::vector<CGAL::Epick::Point_3> qs_cgal_point = meshgraph.ids_to_points(qs);
  outputOFF.add_points_as_block(qs_cgal_point, qs_color, sheped_mode);

  // TODO: vizualize TSI

  // TODO: vizualize LSI

  LOG(INFO) << "Vizualize result := "
            << (outputOFF.finish() ? "success" : "failed") << std::endl;
  LOG(INFO) << "Vizualized file is " << outfile << std::endl;
}

}  // namespace vizualize

#endif  // SOURCE_SKYLINE_VISUALIZE_H_
