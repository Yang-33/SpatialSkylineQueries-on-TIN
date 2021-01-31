#include <iostream>

#include "source/skyline/meshgraph.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "ARGC MUST BE 2." << std::endl;
    return 1;
  }

  skyline::MeshGraph meshgraph(argv[1], false);
  int n = meshgraph.tin_point_size();
  int from = rand() % n;
  int to = rand() % n;
  while (to == from) {
    to = rand() % n;
  }
  auto res = meshgraph.surface_distance(from, to, &n);
}
