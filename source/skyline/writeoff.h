#ifndef SOURCE_SKYLINE_WRITEOFF_H_
#define SOURCE_SKYLINE_WRITEOFF_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "source/util/enumurateref.h"

namespace skyline {

class RGBA {
 public:
  RGBA(int r, int g, int b, double a = 1) : r_(r), g_(g), b_(b), a_(a) {}
  int R() const { return r_; }
  int G() const { return g_; }
  int B() const { return b_; }
  int A() const { return a_; }

 private:
  int r_, g_, b_;
  double a_;
};
const RGBA red(255, 0, 0);
const RGBA green(0, 255, 0);
const RGBA blue(0, 0, 255);
const RGBA yellow(255, 255, 0);
const RGBA magenta(255, 0, 255);
const RGBA cyan(0, 255, 255);
const RGBA white(255, 255, 255);
const RGBA orange(255, 128, 0);

const RGBA default_color = green;

const int kInnerPointSplitN = 10;
const double kBlockRatios = 0.1;
const int kBlockSize = 2;

class WriteOFF {
 public:
  WriteOFF(const std::string& filepath, bool vertex_color = false)
      : filepath_(filepath), vertex_color_(vertex_color) {}

  using POINT3D = CGAL::Epick::Point_3;

  void add_point(double x, double y, double z, RGBA color = default_color) {
    points_.push_back(POINT3D(x, y, z));
    if (vertex_color_) {
      point_colors_.push_back(color);
    }
  }
  void add_point(const POINT3D& p, RGBA color = default_color) {
    points_.push_back(p);
    if (vertex_color_) {
      point_colors_.push_back(color);
    }
  }
  void add_points(const std::vector<POINT3D>& points,
                  RGBA color = default_color) {
    for (auto point : points) {
      add_point(point, color);
    }
  }
  void add_points_as_block(const std::vector<POINT3D>& points,
                           const RGBA& color,
                           const bool shaped_mode) {
    std::vector<CGAL::Epick::Point_3> block;
    for (auto p : points) {
      for (int dx = -kBlockSize; dx <= kBlockSize; ++dx) {
        for (int dy = -kBlockSize; dy <= kBlockSize; ++dy) {
          for (int dz = -kBlockSize; dz <= kBlockSize; ++dz) {
            double x =
                p.x() + dx * kBlockRatios *
                            (shaped_mode ? 1 : ((double)rand() / RAND_MAX));
            double y =
                p.y() + dy * kBlockRatios *
                            (shaped_mode ? 1 : ((double)rand() / RAND_MAX));
            double z =
                p.z() + dz * kBlockRatios *
                            (shaped_mode ? 1 : ((double)rand() / RAND_MAX));
            CGAL::Epick::Point_3 point(x, y, z);
            block.push_back(point);
          }
        }
      }
    }
    for (auto point : block) {
      add_point(point, color);
    }
  }
  void add_points_as_path(const std::vector<POINT3D>& points,
                          RGBA color = default_color) {
    std::vector<CGAL::Epick::Point_3> paths;
    paths.push_back(points.front());
    for (std::size_t i = 0; i + 1 < points.size(); ++i) {
      double dx = points[i + 1].x() - points[i].x();
      double dy = points[i + 1].y() - points[i].y();
      double dz = points[i + 1].z() - points[i].z();
      for (int split = 0; split < kInnerPointSplitN; ++split) {
        double x = points[i].x() + dx / kInnerPointSplitN * (split + 1);
        double y = points[i].y() + dy / kInnerPointSplitN * (split + 1);
        double z = points[i].z() + dz / kInnerPointSplitN * (split + 1);
        CGAL::Epick::Point_3 point(x, y, z);
        paths.push_back(point);
      }
    }
    for (auto point : paths) {
      add_point(point, color);
    }
  }
  void add_face(const std::vector<int>& ids) { faces_.push_back(ids); }
  bool finish(bool append_mode = false) {
    std::ofstream output(
        filepath_, (append_mode ? std::ios_base::app : std::ios_base::out));
    if (vertex_color_) {
      output << "COFF" << std::endl;
    } else {
      output << "OFF" << std::endl;
    }
    output << points_.size() << " " << faces_.size() << " " << 0 << std::endl;

    for (auto [i, point] : util::enumerateref(points_)) {
      if (vertex_color_) {
        auto color = point_colors_[i];
        output << point.x() << " " << point.y() << " " << point.z() << " ";
        output << color.R() << " " << color.G() << " " << color.B() << " "
               << color.A() << std::endl;
      } else {
        output << point.x() << " " << point.y() << " " << point.z()
               << std::endl;
      }
    }

    bool valid = true;
    std::vector<int> invalid_reasons;
    for (auto ids : faces_) {
      output << ids.size() << " ";
      for (auto id : ids) {
        output << id << " ";
        if (id >= (int)points_.size()) {
          valid = false;
          invalid_reasons.push_back(id);
        }
      }
      output << std::endl;
    }
    if (!valid) {
      std::cerr << "[ERROR] in " << filepath_
                << " . This is because the number of points is "
                << points_.size() << ", but point ids ( ";
      for (auto invalid_id : invalid_reasons) {
        std::cerr << invalid_id << " ";
      }
      std::cerr << ") are bigger than it." << std::endl;
    }

    return valid;
  }

 private:
  const std::string filepath_;
  const bool vertex_color_;
  std::vector<POINT3D> points_;
  std::vector<RGBA> point_colors_;
  std::vector<std::vector<int>> faces_;
};

}  // namespace skyline

#endif  // SOURCE_SKYLINE_WRITEOFF_H_
