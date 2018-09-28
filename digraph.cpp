#include "digraph.h"

namespace digraph {

Graph::Graph(size_t nNodes) :
  V_(nNodes)
, dist_(V_, std::vector<int64_t>(V_, cInfinity))
, next_(V_, std::vector<size_t>(V_, cInfinity))
, cap_(V_, std::vector<int64_t>(V_, 0))
{
}

void Graph::addEdge(size_t u, size_t v, int64_t fee, int64_t capacity) {
  dist_[u][v] = fee;
  next_[u][v] = v;
  cap_[u][v] = capacity;
}

void Graph::floydWarshall(int64_t amount) {
  // https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm
  // with path reconstruction
  hash_ = hash(dist_, amount);
  if(!readCache()) {
    for(size_t k = 0; k < V_; k++) {
      for(size_t i = 0; i < V_; i++) {
        for(size_t j = 0; j < V_; j++) {
          if(i == j || i == k || k == j
             || dist_[i][k] == cInfinity
             || dist_[k][j] == cInfinity
             || cap_[i][k] < amount
             || cap_[k][j] < amount) {
            continue;
          } else if(dist_[i][j] > dist_[i][k] + dist_[k][j]) {
            dist_[i][j] = dist_[i][k] + dist_[k][j];
            next_[i][j] = next_[i][k];
            cap_[i][j] = std::min(cap_[i][k], cap_[k][j]);
          }
        }
      }
    }
    writeCache();
  }
}

bool Graph::isPath(size_t u, size_t v) {
  return next_[u][v] != cInfinity;
}

bool Graph::isInPath(size_t u, size_t v, size_t x) {
  if(next_[u][v] == cInfinity) {
    return false;
  }
  while(u != v) {
    u = next_[u][v];
    if(u == x) {
      return true;
    }
  }
  return false;
}

std::vector<size_t> Graph::path(size_t u, size_t v) const {
  std::vector<size_t> res;
  if(next_[u][v] == cInfinity) {
    return res;
  }
  res.push_back(u);
  while(u != v) {
    u = next_[u][v];
    res.push_back(u);
  }
  return res;
}

std::pair<int, std::vector<int>> Graph::centrality() const {
  std::vector<int> res(V_, 0);
  int count = 0;
  for(size_t i = 0; i < V_; i++) {
    for(size_t j = 0; j < V_; j++) {
      auto p = path(i, j);
      if(p.size() != 0) {
        count++;
        for(auto pn : p) {
          if(pn != i && pn != j) {
            res[pn]++;
          }
        }
      }
    }
  }
  return {count, res};
}

int64_t Graph::cost(size_t u, size_t v) const {
  return dist_[u][v];
}

int64_t Graph::maxCost() const {
  int64_t res = 0;
  for(auto& d : dist_) {
    for(auto c : d) {
      if(c != cInfinity) {
        res = std::max(c, res);
      }
    }
  }
  return res;
}

std::size_t Graph::hash(const std::vector<std::vector<int64_t>>& vec,
                        int64_t seed) {
  for(auto& i : vec) {
    for(auto v : i) {
      seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
  }
  return seed;
}

bool Graph::readCache() {
  std::string path = "C:\\Users\\smenzel\\Documents\\lngraph\\cache\\";
  std::ifstream t(path + std::to_string(hash_), std::ios::in | std::ifstream::binary);

  if(t) {
    for(auto& d : dist_) {
      t.read((char*)d.data(), V_ * sizeof(int64_t));
    }
    for(auto& n : next_) {
      t.read((char*)n.data(), V_ * sizeof(size_t));
    }
    return true;
  }
  return false;
}

void Graph::writeCache() {
  std::string path = "C:\\Users\\smenzel\\Documents\\lngraph\\cache\\";
  std::ofstream t(path + std::to_string(hash_), std::ios::out | std::ofstream::binary);

  if(t) {
    for(auto& d : dist_) {
      t.write((char*)d.data(), V_ * sizeof(int64_t));
    }
    for(auto& n : next_) {
      t.write((char*)n.data(), V_ * sizeof(size_t));
    }
  }
}
}
