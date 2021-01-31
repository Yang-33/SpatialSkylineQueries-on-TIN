#ifndef SOURCE_SKYLINE_UNIONFIND_H_
#define SOURCE_SKYLINE_UNIONFIND_H_

#include <vector>

namespace skyline {

class UnionFind {
 public:
  UnionFind(int n) : component(n) { data.assign(n, -1); }
  bool unionSet(int x, int y) {
    x = root(x);
    y = root(y);
    if (x != y) {
      if (data[y] < data[x])
        std::swap(x, y);
      data[x] += data[y];
      data[y] = x;
      component--;
    }
    return x != y;
  }
  bool same(int x, int y) { return root(x) == root(y); }
  int root(int x) { return data[x] < 0 ? x : data[x] = root(data[x]); }
  int size(int x) { return -data[root(x)]; }
  int component_size() { return component; }

  // 返り値はroot(i) == iのときのみ要素が入っていることに注意
  std::vector<std::vector<int>> groupbyroot() {
    std::vector<std::vector<int>> res(data.size());
    for (int i = 0; i < (int)data.size(); i++) {
      res[root(i)].push_back(i);
    }
    return res;
  }

 private:
  std::vector<int> data;
  int component;
};

}  // namespace skyline

#endif  // SOURCE_SKYLINE_UNIONFIND_H_
