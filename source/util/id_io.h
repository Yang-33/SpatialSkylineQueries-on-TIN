#ifndef SOURCE_UTIL_ID_IO_H_
#define SOURCE_UTIL_ID_IO_H_

#include <fstream>
#include <iostream>
#include <numeric>
#include <vector>

namespace util {

const int kRandShuffleTimes = 100000;

std::vector<int> read_id(const std::string& filepath) {
  std::ifstream in(filepath);
  int N;
  in >> N;

  std::vector<int> res;
  for (int i = 0; i < N; ++i) {
    int e;
    in >> e;
    res.push_back(e);
  }
  return res;
}

void write_id(const std::string& filepath, const std::vector<int>& ids) {
  std::ofstream out(filepath);
  out << ids.size() << std::endl;
  for (auto e : ids) {
    out << e << std::endl;
  }
}

std::vector<int> shuffle_seq(const std::vector<int>& seq) {
  std::vector<int> ids = seq;
  int seq_size = (int)seq.size();
  for (int i = 0; i < kRandShuffleTimes; ++i) {
    int a = rand() % seq_size;
    int b = rand() % seq_size;
    std::swap(ids[a], ids[b]);
  }
  return ids;
}

std::vector<int> rand_permutation(int seq_size) {
  std::vector<int> ids(seq_size, 0);
  std::iota(ids.begin(), ids.end(), 0);
  return shuffle_seq(ids);
}

}  // namespace util

#endif  // SOURCE_UTIL_ID_IO_H_
