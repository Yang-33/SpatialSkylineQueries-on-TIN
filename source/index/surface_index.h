#ifndef SOURCE_INDEX_SURFACE_INDEX_H_
#define SOURCE_INDEX_SURFACE_INDEX_H_

#include <utility>
#include <vector>

#include "source/skyline/meshgraph.h"
#include "source/util/enumurateref.h"

namespace surfaceindex {

class SurfaceIndexConstructor {
 public:
  SurfaceIndexConstructor(skyline::MeshGraph& meshgraph,
                          const std::vector<int>& ps,
                          const std::vector<int>& qs)
      : meshgraph_(meshgraph), ps_(ps), qs_(qs) {
    prebuild();
  }

  std::vector<int> points_in_tsi() { return tsi_ans_; }
  std::vector<std::pair<std::vector<int>, int>> points_in_lsi() {
    return lsi_ans_;
  }

 private:
  // |P||Q|log|P|
  void prebuild() {
    for (auto q : qs_) {
      std::vector<std::pair<double, int>> lowers;
      for (auto p : ps_) {
        lowers.push_back({meshgraph_.euclid_distance(p, q), p});
      }
      std::sort(lowers.begin(), lowers.end());

      // TSI
      if (int frontid = lowers.front().second;
          meshgraph_.network_distance(frontid, q) < lowers[1].first) {
        tsi_ans_.push_back(frontid);
      } else {
        // LSI
        // merge segment
        lsi_ans_.push_back({{}, q});
        const double up = meshgraph_.network_distance(frontid, q);
        for (auto [lower, p] : lowers) {
          if (lower <= up) {
            lsi_ans_.back().first.push_back(p);
          }
        }
      }
    }
  }

  skyline::MeshGraph& meshgraph_;
  std::vector<int> tsi_ans_;
  std::vector<std::pair<std::vector<int>, int>> lsi_ans_;

  const std::vector<int>& ps_;
  const std::vector<int>& qs_;
};

}  // namespace surfaceindex

#endif  // SOURCE_INDEX_SURFACE_INDEX_H_
