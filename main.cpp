#include <iostream>
#include <fstream>
#include <iomanip>

#include <nlohmann/json.hpp>

#include "apGraph.h"
#include "digraph.h"

using namespace std;
using json = nlohmann::json;

constexpr int64_t cTtransferAmount = 10000000; // milli satoshi (10ksat)

template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin (reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

template <typename T>
auto end (reversion_wrapper<T> w) { return std::rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse (T&& iterable) { return { iterable }; }

struct Channel;

struct Node {
  size_t number;
  string id;
  string name;
  std::vector<Channel*> channels;

  //dijkstra
  std::map<Node*, int> distance;
};

struct Channel {
  string id;
  Node* nodeA;
  Node* nodeB;
  int64_t capacity; // satoshi
  int64_t feeA;     // fee routing from node A, millisatoshi
  int64_t feerateA; // feerate routing from node A, one millionth
  int64_t feeB;     // fee routing from node B, millisatoshi
  int64_t feerateB; // feerate routing from node B, one millionth
};

struct Graph {
  std::map<string, Node> nodes;
  std::map<string, Channel> channels;
  std::vector<Node*> nodeVect;
  int64_t capacity;
};

Graph graphFromJson(string file) {
  std::ifstream t(file);
  std::string str((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());

  auto raw = json::parse(str);

  Graph g;
  int64_t capacity{};

  if(raw.find("nodes") != raw.end()) {
    auto nodes = raw["nodes"];
    size_t number = 0;
    for(auto& node : nodes) {
      Node n;
      n.number = number++;
      n.id = node["pub_key"].get<std::string>();
      n.name = node["alias"].get<std::string>();
      auto nres = g.nodes.insert({n.id, n});
      g.nodeVect.push_back(&nres.first->second);
    }
  }

  if(raw.find("edges") != raw.end()) {
    auto edges = raw["edges"];
    for(auto& edge : edges) {
      Channel n;
      auto n1Policy = edge["node1_policy"];
      auto n2Policy = edge["node2_policy"];
      if(n1Policy.is_null() && n2Policy.is_null()) {
        continue;
      }
      if(!n1Policy.is_null()) {
        if(n1Policy["disabled"].get<bool>()) {
          continue;
        }

        n.feeA = stoi(n1Policy["fee_base_msat"].get<string>());
        n.feerateA = stoi(n1Policy["fee_rate_milli_msat"].get<string>());
      }

      if(!n2Policy.is_null()) {
        if(n2Policy["disabled"].get<bool>()) {
          continue;
        }

        n.feeB = stoi(n2Policy["fee_base_msat"].get<string>());
        n.feerateB = stoi(n2Policy["fee_rate_milli_msat"].get<string>());
      }

      n.id = edge["channel_id"].get<std::string>();
      auto nodeAId = edge["node1_pub"].get<std::string>();
      auto nodeBId = edge["node2_pub"].get<std::string>();
      n.nodeA = &g.nodes[nodeAId];
      n.nodeB = &g.nodes[nodeBId];
      n.capacity = stoi(edge["capacity"].get<string>());
      capacity += n.capacity;
      auto inserted = g.channels.insert({n.id, n});
      n.nodeA->channels.emplace_back(&inserted.first->second);
      n.nodeB->channels.emplace_back(&inserted.first->second);
    }
  }

  g.capacity = capacity;
  return g;
}

void printAPBC(const Graph& g, AP::Graph& apg)
{
  auto aps = apg.getResult();

  cout << "articulation points: " << aps.articulationPoints.size() << endl;
  cout << "node alias (count of biconnected components it is part of)" << endl;
  for(auto ap : aps.articulationPoints) {
    auto node = g.nodeVect[ap];
    cout << node->name << " (" << aps.countComponentsForVertex(ap) << ")" << endl;
  }
  cout << endl;

  cout << "biconnected components: " << aps.biconnectedComponents.size() << endl;
  cout << "(number of nodes in component){ names of nodes(count of channels) }" << endl;
  for(auto bc : aps.biconnectedComponents) {
    // filter components that are only connected to one component
//    size_t edgeCount = 2;
//    for(auto n : bc) {
//      edgeCount = min(edgeCount, g.nodeVect[n]->channels.size());
//    }
//    if(edgeCount == 1) {
//      continue;
//    }
    auto print = false;
    for(auto gn : bc) {
      if(gn == 539) {
        print = true;
      }
    }
    if(!print) {
      continue;
    }
    cout << "(" << bc.size() << "){ ";
    if(bc.size() > 10) {
      cout << "... ";
    } else {
      for(auto gn : bc) {
        auto n = g.nodeVect[gn];
        cout << n->name << /*"(" << gn << ")*/"(" << n->channels.size() << "), ";
      }
    }
    cout << "}" << endl;
  }
  cout << endl;
}

void printPathCost(const Graph& g, digraph::Graph& dig,
                   size_t from, size_t to)
{
  auto path = dig.path(from, to);
  auto cost = dig.cost(from, to);
  auto costPercentage = cost / static_cast<double>(cTtransferAmount) * 100;

  cout << "cheapest path from " << g.nodeVect[from]->name
       << " to " << g.nodeVect[to]->name
       << " which costs " << cost / 1000. << " sat"
       << " that is " << costPercentage << "%" << endl;
  for(auto p : path) {
    cout << g.nodeVect[p]->name << endl;
  }
  cout << endl;
}

void printDistances(const Graph& g, AP::Graph& apg)
{
  auto distances = apg.getDistances();
  map<size_t, size_t> dist;

  for(auto& n : g.nodes) {
    auto& node = n.second;
    auto d = distances[node.number];
    dist[d]++;
  }

  cout << "highest shortest distances between any two nodes" << endl;
  cout << "(count x distance)" << endl;
  for(auto d : dist) {
    cout << d.second << " x " << d.first << endl;
  }

  cout << endl;
}

void printCentrality(const Graph& g, const digraph::Graph& dig) {
  auto [numPaths, centrality] = dig.centrality();
  std::cout << "count of shortest paths in the network: "
            << numPaths << std::endl;
  std::map<double, size_t> c;
  for(size_t i = 0; i < g.nodes.size(); i++) {
    c.insert({centrality[i] * 100. / numPaths, i});
  }
  cout << "centrality of top 20 nodes" << endl;
  cout << "(centrality in %) node name (channels)" << endl;
  int count = 20;
  for(auto& ct : reverse(c)) {
    auto node = g.nodeVect[ct.second];
    cout << "(" << std::fixed << std::setprecision(2) << ct.first << ") " << node->name
         << "(" << node->channels.size() << ")" << endl;
    if(count-- == 0) {
      break;
    }
  }
  cout << endl;
}

int main()
{
  auto g = graphFromJson("C:\\Users\\smenzel\\Documents\\lngraph\\graph.json");

  cout << "nodes:" << g.nodes.size() << endl;
  cout << "channels:" << g.channels.size() << endl;
  cout << "capacity:" << g.capacity / 100000000. << endl;
  cout << endl;

  AP::Graph apg(g.nodes.size());

  for(auto& chan : g.channels) {
    apg.addEdge(chan.second.nodeA->number, chan.second.nodeB->number);
  }

  printAPBC(g, apg);

  printDistances(g, apg);

  digraph::Graph dig(g.nodes.size());

  for(auto& chan : g.channels) {
    dig.addEdge(chan.second.nodeA->number,
                chan.second.nodeB->number,
                chan.second.feeA
                + chan.second.feerateA * cTtransferAmount / 1000000,
                chan.second.capacity * 1000);
    dig.addEdge(chan.second.nodeB->number,
                chan.second.nodeA->number,
                chan.second.feeB
                + chan.second.feerateB * cTtransferAmount / 1000000,
                chan.second.capacity * 1000);
  }

  dig.floydWarshall(cTtransferAmount);

  cout << "max cost: " << dig.maxCost() << endl;
  cout << endl;

  printPathCost(g, dig, 30, 7);
  printPathCost(g, dig, 7, 30);

  printCentrality(g, dig);

  return 0;
}
