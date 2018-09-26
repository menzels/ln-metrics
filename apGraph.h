// algorithm taken from https://www.hackerearth.com/practice/algorithms/graphs/biconnected-components/tutorial/

#pragma once

#include <iostream>
#include <list>
#include <algorithm>
#include <vector>
#include <stack>
#include <unordered_set>

#define NIL static_cast<size_t>(-1)

namespace AP {

struct Result {
  std::unordered_set<size_t> articulationPoints;
  std::vector<std::unordered_set<size_t>> biconnectedComponents;

  long long countComponentsForVertex(size_t node) const {
    return count_if(biconnectedComponents.begin(),
                    biconnectedComponents.end(),
                    [node](std::unordered_set<size_t> set){
      return set.count(node) > 0;
    });
  }
};

struct Edge {
  size_t u;
  size_t v;
};

// A class that represents an undirected graph
class Graph
{
public:
  Graph(size_t V);

  void addEdge(size_t v, size_t w);   // function to add an edge to graph

  Result getResult();
  std::vector<size_t> getDistances();

private:
  size_t V;    // No. of vertices
  std::vector<std::vector<size_t>> adj; // adjacency matrix
  void recurseDFS(size_t v,
                  int d,
                  std::vector<bool>& visited,
                  std::vector<int>& depth,
                  std::vector<int>& low,
                  std::vector<size_t>& parent,
                  std::stack<Edge>& stack,
                  Result& result);
  std::unordered_set<size_t> handleEdges(std::stack<Edge>& stack,
                                         size_t u,
                                         size_t v);
  size_t bfs(size_t start);
};

Graph::Graph(size_t V)
{
  this->V = V;
  adj.resize(V);
}

void Graph::addEdge(size_t u, size_t v)
{
  adj[u].push_back(v);
  adj[v].push_back(u);  // Note: the graph is undirected
}

// handles stack of edges in case an articulation point is found,
// so we can keep track of the biconnected components.
// return value is the set of nodes that are in one biconnected component.
std::unordered_set<size_t> Graph::handleEdges(std::stack<Edge>& stack, size_t u, size_t v) {
  std::unordered_set<size_t> nodes;
  while(true) {
    auto edge = stack.top();
    nodes.insert(edge.u);
    nodes.insert(edge.v);
    stack.pop();
    if((edge.u == u && edge.v == v) || (edge.v == u && edge.u == v)){
      return nodes;
    }
  }
}

size_t Graph::bfs(size_t start)
{
  std::deque<size_t> queue;     //Double-ended queue
  std::vector<size_t> distance(V, NIL);
  std::vector<bool> visited(V, false);

  queue.push_back(start);
  distance[start] = 0;
  while(!queue.empty ())
  {
    auto u = queue.front();
    visited[u] = true;
    queue.pop_front();
    for(auto i = adj[u].begin(); i != adj[u].end(); ++i)
    {
      size_t v = *i;  // v is current adjacent of u

      /* if distance of neighbour of v from start node is greater than sum of distance of v from start node and edge weight between v and its neighbour (distance between v and its neighbour of v) ,then change it */

      if(distance[u] + 1 < distance[v])
      {
        distance[v] = distance[u] + 1;

        /*if edge weight between v and its neighbour is 0 then push it to front of
        double ended queue else push it to back*/

        queue.push_back(v);
      }
    }
  }

  size_t maxDistance = 0;
  for(size_t i = 0; i < V; i ++){
    if(visited[i]) {
      maxDistance = std::max(maxDistance, distance[i]);
    }
  }

  return maxDistance;
}

// A recursive function that find articulation points using DFS traversal
// u --> The vertex to be visited next
// visited --> keeps track of visited vertices
// depth --> Stores depth of visited vertices in DFS tree
// parent --> Stores parent vertices in DFS tree
// ap --> Store articulation points
// stack --> The stack of edges to keep track of biconnected components
// d --> current depth
void Graph::recurseDFS(size_t u,
                       int d,
                       std::vector<bool>& visited,
                       std::vector<int>& depth,
                       std::vector<int>& low,
                       std::vector<size_t>& parent,
                       std::stack<Edge>& stack,
                       Result& result)
{
  bool isArticulation = false;
  // Count of children in DFS Tree
  int children = 0;

  // Mark the current node as visited
  visited[u] = true;

  // Initialize depth and low value
  depth[u] = low[u] = d;

  // Go through all vertices aadjacent to this
  for (auto i = adj[u].begin(); i != adj[u].end(); ++i)
  {
    size_t v = *i;  // v is current adjacent of u

    // If v is not visited yet, then make it a child of u
    // in DFS tree and recurse for it
    if (!visited[v]) {
      children++;
      parent[v] = u;
      stack.push({u, v});
      recurseDFS(v, d + 1, visited, depth, low, parent, stack, result);

      // Check if the subtree rooted with v has a connection to
      // one of the ancestors of u
      low[u] = std::min(low[u], low[v]);

      // node is articulation point when one of the following is true
      if(parent[u] == NIL && children > 1) {
        // (1) u is root of DFS tree and has two or more chilren.
        isArticulation = true;

        // add biconnected component to result
        auto nodes = handleEdges(stack, u, v);
        result.biconnectedComponents.push_back(nodes);
      } else if(parent[u] != NIL && low[v] >= depth[u]){
        // (2) If u is not root and low value of one of its children is more
        // than depth value of u.
        isArticulation = true;

        // add biconnected component to result
        auto nodes = handleEdges(stack, u, v);
        result.biconnectedComponents.push_back(nodes);
      }
    } else if (v != parent[u]) {
      // Update low value of u for parent function calls.
      low[u] = std::min(low[u], depth[v]);
//      stack.push({u, v}); this somehow fucks up the result. without everything is fine
    }
  }
  if (isArticulation) {
    // add node to result
    result.articulationPoints.insert(u);
  }
}

Result Graph::getResult()
{
  // Mark all the vertices as not visited
  std::vector<bool> visited(V);
  std::vector<int> depth(V);
  std::vector<int> low(V);
  std::vector<size_t> parent(V);
  std::stack<Edge> stack;
  Result result;

  // Initialize parent and visited, and ap(articulation point) arrays
  for (size_t i = 0; i < V; i++)
  {
    parent[i] = NIL;
    visited[i] = false;
  }

  // Call the recursive helper function to find articulation points
  // in DFS tree rooted with vertex 'i'
  for (size_t i = 0; i < V; i++) {
    if (visited[i] == false) {
      recurseDFS(i, 0, visited, depth, low, parent, stack, result);

      // add remaining edges from the stack to the result
      // this is our last remaining biconnected component.
      std::unordered_set<size_t> nodes;
      while(stack.size() > 0){
        nodes.insert(stack.top().u);
        nodes.insert(stack.top().v);
        stack.pop();
      }
      if(nodes.size() > 0) {
        result.biconnectedComponents.push_back(nodes);
      }
    }
  }
  return result;
}

std::vector<size_t> Graph::getDistances()
{
  std::vector<size_t> res;
  for(size_t i = 0; i < V; i ++){
    auto distance = bfs(i);
    res.push_back(distance);
  }
  return res;
}
}
