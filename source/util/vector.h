#ifndef SOURCE_UTIL_VECTOR_H_
#define SOURCE_UTIL_VECTOR_H_

#include <string>
#include <vector>

#include "source/util/enumurateref.h"

namespace util {

std::string vec_to_string(const std::vector<int>& vec) {
  std::string ans_string;
  ans_string = "[ ";
  for (auto [i, a] : enumerateref(vec)) {
    ans_string += std::to_string(a);
    if (i + 1 != vec.size())
      ans_string += ", ";
  }
  ans_string += " ]";
  return ans_string;
}

}  // namespace util

#endif  // SOURCE_UTIL_VECTOR_H_
