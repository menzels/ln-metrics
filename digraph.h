#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <functional>
#include <string>
#include <algorithm>
#include <list>

namespace digraph {

constexpr int64_t cInfinity = std::numeric_limits<int64_t>::max();

class Graph
{
public:
  Graph(size_t nNodes);

  void addEdge(size_t u, size_t v, int64_t w, int64_t balance);

  void floydWarshall(int64_t amount);

  bool isPath(size_t u, size_t v);
  bool isInPath(size_t u, size_t v, size_t x);
  std::vector<size_t> path(size_t u, size_t v) const;
  std::pair<int, std::vector<int> > centrality() const;

  int64_t cost(size_t u, size_t v) const;
  int64_t maxCost() const;

private:
  size_t V_; // no of vertices
  size_t hash_;
  std::vector<std::vector<int64_t>> dist_;
  std::vector<std::vector<size_t>> next_;
  std::vector<std::vector<int64_t>> cap_;

  static std::size_t hash(std::vector<std::vector<int64_t>> const& vec,
                          int64_t seed);

  bool readCache();
  void writeCache();
};
}
