#ifndef SOURCE_SKYLINE_MINIMUM_BOUNDING_BOX_H_
#define SOURCE_SKYLINE_MINIMUM_BOUNDING_BOX_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <vector>

#include "source/skyline/meshgraph.h"

namespace skyline {

using POINT3D = CGAL::Epick::Point_3;

POINT3D max(const POINT3D& a, const POINT3D& b) {
  double x = std::max(a.x(), b.x());
  double y = std::max(a.y(), b.y());
  double z = std::max(a.z(), b.z());
  return POINT3D(x, y, z);
}

POINT3D min(const POINT3D& a, const POINT3D& b) {
  double x = std::min(a.x(), b.x());
  double y = std::min(a.y(), b.y());
  double z = std::min(a.z(), b.z());
  return POINT3D(x, y, z);
}

class Box {
 public:
  Box() : is_initialized_(false) {}
  Box(const POINT3D& mn, const POINT3D& mx)
      : is_initialized_(true), disable_(false), mn_(mn), mx_(mx) {}

  bool is_point_in_minimum_bounding_box(MeshGraph& meshgraph, const int pid) {
    if(disable_){
      return true;
    }
    if (!is_valid_data()) {
      // not initialized yet.
      return true;
    }
    bool ok = true;
    POINT3D p = meshgraph.id_to_point(pid);
    ok &= mn_.x() <= p.x() && p.x() <= mx_.x();
    ok &= mn_.y() <= p.y() && p.y() <= mx_.y();
    ok &= mn_.z() <= p.z() && p.z() <= mx_.z();
    return ok;
  }
  void disable() { disable_ = true; }
  bool is_valid_data() { return is_initialized_; }

  void update_minimum_bounding_box(MeshGraph& meshgraph,
                                   const int new_p,
                                   const std::vector<int>& qs) {
    if (disable_) {
      return;
    }
    Box mbr_of_new_p =
        create_minimum_bounding_box_from_p_and_qs(meshgraph, new_p, qs);
    update_intersect_box(mbr_of_new_p);
  }

  // It can be called when tsi is updated.
  void update_minimum_bounding_box(MeshGraph& meshgraph,
                                   const std::vector<int>& skylines,
                                   const std::vector<int>& qs) {
    for (auto skyline : skylines) {
      update_minimum_bounding_box(meshgraph, skyline, qs);
    }
  }

 private:
  class Sphare {
   public:
    Sphare(const double r, const POINT3D& p) : r_(r), p_(p) {}
    Box boxize() {
      POINT3D mn(p_.x() - r_, p_.y() - r_, p_.z() - r_);
      POINT3D mx(p_.x() + r_, p_.y() + r_, p_.z() + r_);
      return Box(mn, mx);
    }

   private:
    double r_;
    POINT3D p_;
  };

  void update_union_box(const Box& other) {
    initialized_check_and_update(other);
    mn_ = min(mn_, other.mn_);
    mx_ = max(mx_, other.mx_);
  }

  void update_intersect_box(const Box& other) {
    initialized_check_and_update(other);
    mn_ = max(mn_, other.mn_);
    mx_ = min(mx_, other.mx_);
  }

  Box create_minimum_bounding_box_from_p_and_qs(MeshGraph& meshgraph,
                                                const int p,
                                                const std::vector<int>& qs) {
    std::vector<Sphare> sphares;

    for (auto q : qs) {
      double dn = meshgraph.network_distance(p, q);
      sphares.push_back(Sphare(dn, meshgraph.id_to_point(q)));
    }
    Box mbr(sphares.front().boxize());
    for (auto sphare : sphares) {
      mbr.update_union_box(sphare.boxize());
    }
    return mbr;
  }

  void initialized_check_and_update(const Box& other) {
    if (!is_initialized_) {
      is_initialized_ = true;
      mn_ = other.mn_;
      mx_ = other.mx_;
    }
  }

  bool is_initialized_;
  bool disable_;
  POINT3D mn_, mx_;
};

}  // namespace skyline

#endif  // SOURCE_SKYLINE_MINIMUM_BOUNDING_BOX_H_
