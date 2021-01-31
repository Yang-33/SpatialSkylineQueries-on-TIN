#ifndef SOURCE_UTIL_COMPRESSION_H_
#define SOURCE_UTIL_COMPRESSION_H_

#include <map>
#include <string>
#include <vector>

#include "source/util/enumurateref.h"

namespace util {

std::map<int, int> compression_seq(const std::vector<int>& raw_seq) {
  std::map<int, int> res;
  for (auto [i, e] : enumerateref(raw_seq)) {
    res[e] = i;
  }
  return res;
}

}  // namespace util

#endif  // SOURCE_UTIL_COMPRESSION_H_
