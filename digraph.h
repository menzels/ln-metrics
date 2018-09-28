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

  void addEdge(size_t u, size_t v, int64_t w);

  void floydWarshall();

  std::vector<size_t> path(size_t u, size_t v) const;
  std::pair<int, std::vector<int> > centrality() const;

  int64_t cost(size_t u, size_t v) const;
  int64_t maxCost() const;

private:
  size_t V; // no of vertices
  std::vector<std::vector<int64_t>> dist;
  std::vector<std::vector<size_t>> next;

  static std::size_t hash(std::vector<std::vector<int64_t>> const& vec);

  bool readCache();
  void writeCache();
};
}
