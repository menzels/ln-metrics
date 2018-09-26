#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>

#include "apGraph.h"

using namespace std;
using json = nlohmann::json;

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
    size_t edgeCount = 2;
    for(auto n : bc) {
      edgeCount = min(edgeCount, g.nodeVect[n]->channels.size());
    }
    if(edgeCount == 1) {
      continue;
    }
    cout << "(" << bc.size() << "){ ";
    if(bc.size() > 10) {
      cout << "... ";
    } else {
      for(auto gn : bc) {
        auto n = g.nodeVect[gn];
        cout << n->name << "(" << n->channels.size() << "), ";
      }
    }
    cout << "}" << endl;
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

  return 0;
}
