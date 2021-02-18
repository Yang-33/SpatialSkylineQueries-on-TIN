#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iomanip>
#include <iostream>
#include <vector>

#include "source/datapoint/generator.h"
#include "source/querypoint/generator.h"
#include "source/skyline/meshgraph.h"
#include "source/skyline/skyline_on_tin.h"
#include "source/skyline/vizualize.h"
#include "source/util/measure_time.h"
#include "source/util/stringformat.h"

const int kRandSeed = 0;
DEFINE_int32(randseed, kRandSeed, "seed value for rand()");

const std::string kTinPath = "./data/q3-0001000.off";
DEFINE_string(tinpath, kTinPath, "TIN file path. the extension must be off");

const bool kTestMemoDs = true;
DEFINE_bool(testmemods,
            kTestMemoDs,
            "experimental : calc ds exactly once to test faster.");

const int kPSize = 100;
DEFINE_int32(Psize, kPSize, "the number of |P|");

const int kQSize = 10;
DEFINE_int32(Qsize, kQSize, "the number of |Q|");

const double kQMBRPercentage = 2;
DEFINE_double(QMBRPercentage,
              kQMBRPercentage,
              "the percentage of the minimum bounding box of |Q|.");

const bool kVisualizeShaped = false;
DEFINE_bool(vshape,
            kVisualizeShaped,
            "shaped visualizer mode. This cannot show deplicated points. ");

const bool kForcePointUpdate = false;
DEFINE_bool(forcepointupdate,
            kForcePointUpdate,
            "if true, input sequences are updated forcely.");

const bool kQPositionSquare = true;
DEFINE_bool(qpossquare, kQPositionSquare, "select position of Q");

const int kReorder = 0;
DEFINE_int32(reorder, kReorder, "reorder P by sorting with q");

const bool kOnlyFast = false;
DEFINE_bool(onlyfast, kOnlyFast, "Calc only the fast solution");



const bool kFastLSI = true;
DEFINE_bool(fastlsi, kFastLSI, "Use fast LSI");

DEFINE_bool(useTSILSI, false, "1.");
DEFINE_bool(useSibori, false, "2.");
DEFINE_bool(useBB, false, "3");
DEFINE_bool(useIneq, false, "4.");
DEFINE_bool(useNewLB, false, "5.");
const bool kUseTinFilter = false;
DEFINE_bool(usetinfilter,
            kUseTinFilter,
            "6. Use TIN filter to make calc time faster");
DEFINE_bool(useALL, false, "7. Use all");

int main(int argc, char* argv[]) {
  {  // LOG
    FLAGS_log_dir = "./log/glog";
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;
    google::InitGoogleLogging(argv[0]);
  }

  {  // FLAG
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    LOG(INFO) << "|randseed| := " << FLAGS_randseed << std::endl
              << "|tinpath| := " << FLAGS_tinpath << std::endl
              << "|testmemods| := " << FLAGS_testmemods << std::endl
              << "|Psize| := " << FLAGS_Psize << std::endl
              << "|Qsize| := " << FLAGS_Qsize << std::endl
              << "|QMBRPercentage| := " << FLAGS_QMBRPercentage << std::endl
              << "|vshape| := " << FLAGS_vshape << std::endl
              << "|forcepointupdate| := " << FLAGS_forcepointupdate << std::endl
              << "|qpossquare| := " << FLAGS_qpossquare << std::endl
              << "|reorder| := " << FLAGS_reorder << std::endl
              << "|onlyfast| := " << FLAGS_onlyfast << std::endl
              << "|usetinfilter| := " << FLAGS_usetinfilter << std::endl
              << "|fastlsi| := " << FLAGS_fastlsi << std::endl;

    srand(FLAGS_randseed);

    // diff count
    int diff_flag_count = 0;
    diff_flag_count += !!(FLAGS_tinpath != kTinPath);
    diff_flag_count += !!(FLAGS_Psize != kPSize);
    diff_flag_count += !!(FLAGS_Qsize != kQSize);
    diff_flag_count += !!(FLAGS_QMBRPercentage != kQMBRPercentage);
    if (diff_flag_count > 1) {
      LOG(INFO) << "not default value is over two. ";
    }
  }

  // Main
  LOG(INFO) << "start" << std::endl;

  // read tin
  skyline::MeshGraph meshgraph(FLAGS_tinpath, FLAGS_testmemods);

  // read p or create p
  const int psize = std::min(FLAGS_Psize, meshgraph.tin_point_size());
  const std::vector<int> ps = datapoint::create_or_read_datapoint(
      util::format("./P-size-%04d.seq", psize), psize,
      meshgraph.tin_point_size(), FLAGS_forcepointupdate);

  // ========== INPUT ==========

  // read q or create q
  const int qsize = FLAGS_Qsize;
  const double mbr_percentage = FLAGS_QMBRPercentage;
  const std::vector<int> qs = querypoint::create_or_read_querypoint(
      util::format("./Q-size-%04d-MBR-%3.4f.seq", qsize, mbr_percentage), qsize,
      meshgraph.tin_point_size(), mbr_percentage, meshgraph,
      FLAGS_forcepointupdate, FLAGS_qpossquare);

  meshgraph.prebuild(ps, qs);

  // ========== SKYLINE ==========

  std::chrono::milliseconds time1;
  std::vector<int> skyline_points;
  if (!FLAGS_onlyfast) {
    skyline_points = MEASURE_MILLISECONDS(
        time1, skyline::naive::solve_skyline_on_tin(meshgraph, ps, qs));
    meshgraph.clear_cache();
  }

  if (FLAGS_usetinfilter || FLAGS_useALL) {
    meshgraph.use_tin_filter();
    LOG(INFO) << "use tin filter";
  }

  std::chrono::milliseconds time2;
  const std::vector<int> skyline_points2 = MEASURE_MILLISECONDS(
      time2,
      skyline::fast::solve_skyline_on_tin(
          meshgraph, ps, qs, FLAGS_reorder, FLAGS_fastlsi,
          FLAGS_useTSILSI || FLAGS_useALL, FLAGS_useSibori || FLAGS_useALL,
          FLAGS_useBB || FLAGS_useALL, FLAGS_useIneq || FLAGS_useALL,
          FLAGS_useNewLB || FLAGS_useALL, FLAGS_useALL));

  meshgraph.clear_cache();
  if (!FLAGS_onlyfast) {
    LOG(INFO) << "performance := x " << (double)time1.count() / time2.count()
              << " faster";

    // CHECK if the solution is correct
    CHECK_EQ(skyline_points.size(), skyline_points2.size())
        << "skyline size is not correct.";
    for (size_t i = 0; i < skyline_points.size(); ++i) {
      CHECK_EQ(skyline_points[i], skyline_points2[i])
          << "skyline elem is not correct; id:" << i;
    }
  }

  // vizualize skyline, p, q, TSI, LSI
  vizualize::vizualize("./viz.off", meshgraph, skyline_points2, skyline::blue,
                       ps, skyline::white, qs, skyline::red, kVisualizeShaped);

  return 0;
}
