#ifndef SOURCE_SKYLINE_DOMINANCE_DIST_HELPER_H_
#define SOURCE_SKYLINE_DOMINANCE_DIST_HELPER_H_

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "source/skyline/meshgraph.h"
#include "source/util/compression.h"

namespace skyline {

namespace fast {

class DominanceDistHelper {
 public:
  DominanceDistHelper(MeshGraph& meshgraph,
                      const std::vector<int>& ps,
                      const std::vector<int>& qs)
      : p_compressed_id_(util::compression_seq(ps)),
        q_compressed_id_(util::compression_seq(qs)),
        meshgraph_(meshgraph),
        surface_distance_memo_(ps.size(),
                               std::vector<std::optional<double>>(qs.size())),
        dominance_memo_(ps.size(), std::vector<int>(ps.size(), 0)),
        fast_the_number_of_calculating_ds_(0),
        disable_newLB_(false),disalbe_ineq_(false) {
    init_dominance_memo(ps, qs);
  }
  void disable_newLB() { disable_newLB_ = true; }
  void disable_ineq() { disalbe_ineq_ = true; }
  // true : |pnew| is dominated by one of |current_skylines|
  bool is_dominated_with_inequality(int pnew,
                                    const std::set<int>& current_skylines,
                                    const std::vector<int>& qs) {
    if (disalbe_ineq_){
      return false;
    }
      for (auto skyline : current_skylines) {
        if (is_dominated_with_inequality(pnew, skyline, qs)) {
          return true;
        }
      }
    return false;
  }

  // true : |pnew| is skyline since all of |current_skylnes| don't dominate it.
  bool is_not_dominated_with_inequality(int pnew,
                                        const std::set<int>& current_skylines,
                                        const std::vector<int>& qs) {
    if (disalbe_ineq_) {
      return false;
    }
    // all : if not Ds(pnew) > Ds(skyline), pnew is not dominated.
    for (auto skyline : current_skylines) {
      if (!is_not_dominated_with_inequality(pnew, skyline, qs)) {
        return false;
      }
    }
    return true;
  }

  // pnew is dominated
  bool is_not_dominated_with_surface_distance(const int pnew,
                                              const std::set<int> skylines,
                                              const std::vector<int>& qs) {
    // one of -> false
    for (auto skyline : skylines) {
      if (!is_not_dominated_with_surface_distance(pnew, skyline, qs)) {
        return false;
      }
    }

    return true;
  }

  std::vector<int> dominated_skylines_by_p(
      int pnew,
      const std::set<int>& current_skylines,
      const std::vector<int>& qs) {
    std::vector<int> ans;
    for (auto skyline : current_skylines) {
      if (is_dominated_with_inequality(skyline, pnew, qs) ||
          is_dominated_with_surface_distance(skyline, pnew, qs)) {
        ans.push_back(skyline);
      }
    }
    return ans;
  }

  int ds_counter() const { return fast_the_number_of_calculating_ds_; }

  double euclid_distance(int p, int q) {
    return meshgraph_.euclid_distance(p, q);
  }
  double network_distance(int p, int q) {
    return meshgraph_.network_distance(p, q);
  }
  // q, ps
  int nearest_search(int to, const std::vector<int>& sources) {
    return meshgraph_.nearest_search(to, sources,
                                     &fast_the_number_of_calculating_ds_);
  }
  double surface_distance(int p, int q) {
    int pid = p_compressed_id_[p];
    int qid = q_compressed_id_[q];
    if (surface_distance_memo_[pid][qid]) {
      return surface_distance_memo_[pid][qid].value();
    }
    double dist =
        meshgraph_.surface_distance(p, q, &fast_the_number_of_calculating_ds_)
            .first;
    surface_distance_memo_[pid][qid] = dist;
    return dist;
  }

 private:
  // one of q, !(Ds(p1, q) > Ds(p2, q))
  // Dn(p1, q) <= De(p2, q) -> Ds(p1, q) <= Ds(p2, q)
  bool is_not_dominated_with_inequality(int p1,
                                        int p2,
                                        const std::vector<int>&) {
    return check_no_dominance(p1, p2);
  }

  // true : |p1| is dominated by |p2|
  // dn(p2, q) < de(p1, q) => ds(p2, q) < ds(p1, q)
  bool is_dominated_with_inequality(const int p1,
                                    const int p2,
                                    const std::vector<int>& qs) {
    for (auto q : qs) {
      if (!(DN(p2, q) < DE(p1, q))) {
        return false;
      }
    }
    return true;
  }

  // forall q, Ds(p1,q) > Ds(p2,q) -> p1 is dominated.
  bool is_dominated_with_surface_distance(const int p1,
                                          const int p2,
                                          const std::vector<int>& qs) {
    return !is_not_dominated_with_surface_distance(p1, p2, qs);
  }

  bool is_not_dominated_with_surface_distance(const int p1,
                                              const int p2,
                                              const std::vector<int>& qs) {
    if (check_no_dominance(p1, p2) && !disalbe_ineq_) {
      return true;
    }
    for (auto q : qs) {
      if (DE(p1, q) > DN(p2, q) && !disalbe_ineq_) {  // DS(p1) > DS(p2)
        continue;
      }
      if (DS(p1, q) <= DS(p2, q)) {
        // same pair won't calc again.
        return true;
      }
    }
    return false;
  }

  double DE(int p, int q) {
    if (!disable_newLB_) {  // gm..
      double theta_min = meshgraph_.calc_theta_min();
      // LOG(INFO) << "theta min := " << theta_min;
      double lambda = std::min(
          {std::sin(theta_min) / 2, std::sin(theta_min), std::cos(theta_min)});
      double lower = network_distance(p, q) * lambda;
      // LOG(INFO) << "lower check ! De := " << euclid_distance(p, q)
      //           << "lambda*Dn := " << lower;
      return std::max(lower, euclid_distance(p,q));
    }

    return euclid_distance(p, q);
  }

  double DN(int p, int q) { return network_distance(p, q); }

  double DS(int p, int q) { return surface_distance(p, q); }

  void init_dominance_memo(const std::vector<int>& ps,
                           const std::vector<int>& qs) {
    for (auto p1 : ps) {
      for (auto p2 : ps) {
        if (p1 == p2) {
          continue;
        }
        int p1id = p_compressed_id_[p1];
        int p2id = p_compressed_id_[p2];

        int& result = dominance_memo_[p1id][p2id];

        // p1 isn't dominated by p2 because of one of Q => set true
        for (auto q : qs) {
          if (DN(p1, q) <= DE(p2, q)) {
            result = true;
            break;
          }
        }
      }
    }
  }

  bool check_no_dominance(const int p1, const int p2) {
    int p1id = p_compressed_id_[p1];
    int p2id = p_compressed_id_[p2];
    int& result = dominance_memo_[p1id][p2id];
    return result;
  }

  std::map<int, int> p_compressed_id_;
  std::map<int, int> q_compressed_id_;
  MeshGraph& meshgraph_;
  std::vector<std::vector<std::optional<double>>> surface_distance_memo_;
  std::vector<std::vector<int>> dominance_memo_;
  int fast_the_number_of_calculating_ds_;
  bool disable_newLB_;
  bool disalbe_ineq_;
};

}  // namespace fast

}  // namespace skyline

#endif  // SOURCE_SKYLINE_DOMINANCE_DIST_HELPER_H_
