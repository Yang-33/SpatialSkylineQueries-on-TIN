#include <gtest/gtest.h>
#include <vector>

// See main.cc
TEST(SolutionTest, NaiveAndFast) {
  std::vector<int> a{1, 2, 3};
  std::vector<int> b{1, 2, 3};
  ASSERT_EQ(a.size(), b.size());
  for (size_t i = 0; i < a.size(); ++i) {
    EXPECT_EQ(a[i], b[i]);
  }
}
