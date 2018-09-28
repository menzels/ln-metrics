#include "digraph.h"

namespace digraph {

Graph::Graph(size_t nNodes) :
  V(nNodes)
, dist(V, std::vector<int64_t>(V, cInfinity))
, next(V, std::vector<size_t>(V, 0))
{
}

void Graph::addEdge(size_t u, size_t v, int64_t w) {
  dist[u][v] = w;
  next[u][v] = v;
}

void Graph::floydWarshall() {
  // https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm
  // with path reconstruction
  if(!readCache()) {
    for(size_t k = 0; k < V; k++) {
      for(size_t i = 0; i < V; i++) {
        for(size_t j = 0; j < V; j++) {
          if(dist[i][k] == cInfinity
             || dist[k][j] == cInfinity) {
            continue;
          } else if(dist[i][j] > dist[i][k] + dist[k][j]) {
            dist[i][j] = dist[i][k] + dist[k][j];
            next[i][j] = next[i][k];
          }
        }
      }
    }
    writeCache();
  }
}

std::vector<size_t> Graph::path(size_t u, size_t v) const {
  std::vector<size_t> res;
  if(next[u][v] == 0) {
    return res;
  }
  res.push_back(u);
  while(u != v) {
    u = next[u][v];
    res.push_back(u);
  }
  return res;
}

std::pair<int, std::vector<int>> Graph::centrality() const {
  std::vector<int> res(V, 0);
  int count = 0;
  for(size_t i = 0; i < V; i++) {
    for(size_t j = 0; j < V; j++) {
      auto p = path(i, j);
      for(auto pn : p) {
        if(pn != i && pn != j) {
          res[pn]++;
        }
      }
      if(p.size() != 0) {
        count++;
      }
    }
  }
  return {count, res};
}

int64_t Graph::cost(size_t u, size_t v) const {
  return dist[u][v];
}

int64_t Graph::maxCost() const {
  int64_t res = 0;
  for(auto& d : dist) {
    for(auto c : d) {
      if(c != cInfinity) {
        res = std::max(c, res);
      }
    }
  }
  return res;
}

std::size_t Graph::hash(const std::vector<std::vector<int64_t> >& vec) {
  std::size_t seed = vec.size();
  for(auto& i : vec) {
    for(auto& v : i) {
      seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
  }
  return seed;
}

bool Graph::readCache() {
  auto h = hash(dist);

  std::string path = "C:\\Users\\smenzel\\Documents\\lngraph\\cache\\";
  std::ifstream t(path + std::to_string(h), std::ios::in | std::ifstream::binary);

  if(t) {
    for(auto& d : dist) {
      t.read((char*)d.data(), V * sizeof(int64_t));
    }
    for(auto& n : next) {
      t.read((char*)n.data(), V * sizeof(size_t));
    }
    return true;
  }
  return false;
}

void Graph::writeCache() {
  auto h = hash(dist);

  std::string path = "C:\\Users\\smenzel\\Documents\\lngraph\\cache\\";
  std::ofstream t(path + std::to_string(h), std::ios::out | std::ofstream::binary);

  if(t) {
    for(auto& d : dist) {
      t.write((char*)d.data(), V * sizeof(int64_t));
    }
    for(auto& n : next) {
      t.write((char*)n.data(), V * sizeof(size_t));
    }
  }
}
}
