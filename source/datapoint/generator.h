#ifndef SOURCE_DATAPOINT_GENERATOR_H_
#define SOURCE_DATAPOINT_GENERATOR_H_

#include <glog/logging.h>
#include <fstream>
#include <vector>

#include "source/util/id_io.h"

namespace datapoint {

std::vector<int> create_id_from_rand_permutation(int select, int seq_size) {
  std::vector<int> ids = util::rand_permutation(seq_size);
  std::vector<int> ans;
  for (int i = 0; i < select; ++i) {
    ans.push_back(ids[i]);
  }
  return ans;
}

std::vector<int> create_or_read_datapoint(const std::string& filepath,
                                          int size,
                                          int tin_size,
                                          bool forceupdate) {
  if (std::ifstream ifs(filepath); !ifs.is_open() || forceupdate) {
    std::vector<int> ids = create_id_from_rand_permutation(size, tin_size);
    util::write_id(filepath, ids);
    LOG(INFO) << filepath << " has been updated.";
  } else {
    LOG(INFO) << filepath << " is used because it exists. ";
  }
  return util::read_id(filepath);
}

}  // namespace datapoint

#endif  // SOURCE_DATAPOINT_GENERATOR_H_
