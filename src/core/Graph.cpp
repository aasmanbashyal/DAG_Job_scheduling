/**
 * @file Graph.cpp
 * @brief Implementation of the Graph class
 */

#include "Graph.hpp"
#include <algorithm>
#include <queue>
#include <sstream>
#include <stdexcept>

void Graph::addNode(int id, double execTime) {
    // Validate inputs
    if (id < 0) {
        throw std::invalid_argument("Node ID cannot be negative");
    }
    if (execTime < 0) {
        throw std::invalid_argument("Execution time cannot be negative");
    }

    const auto existing = nodeIndices.find(id);
    if (existing != nodeIndices.end()) {
        nodes[existing->second].execTime = execTime;
        return;
    }
    nodeIndices.emplace(id, nodes.size());
    nodes.push_back(Node{id, execTime, {}, {}});
}

const Graph::Node& Graph::getNode(int id) const {
    return nodes.at(nodeIndices.at(id));
}

void Graph::addEdge(int from, int to, double t1, double t2) {
    // Validate node existence and communication times
    if (nodeIndices.find(from) == nodeIndices.end() || nodeIndices.find(to) == nodeIndices.end()) {
        throw std::invalid_argument("Node does not exist");
    }
    if (t1 < 0 || t2 < 0) {
        throw std::invalid_argument("Communication times cannot be negative");
    }

    // Create unique edge identifier and store edge
    std::string key = makeEdgeKey(from, to);
    // Replacing an edge's communication costs must not duplicate dependencies.
    if (edges.find(key) == edges.end()) {
        nodes[nodeIndices.at(from)].successors.push_back(to);
        nodes[nodeIndices.at(to)].predecessors.push_back(from);
    }
    edges[key] = Edge{from, to, t1, t2};
}

std::string Graph::makeEdgeKey(int from, int to) const {
    std::stringstream ss;
    ss << from << "->" << to;
    return ss.str();
}

double Graph::getCommTime(int from, int to, bool sameCluster) const {
    std::string key = makeEdgeKey(from, to);
    auto it = edges.find(key);
    if (it == edges.end()) return 0.0;

    // Use t1 or t2 based on whether nodes are in same cluster
    return sameCluster ? it->second.weight : it->second.t2;
}

std::vector<int> Graph::getTopologicalOrder() const {
    // Track in-degree (number of incoming edges/dependencies) for each node
    std::vector<int> inDegree(nodes.size(), 0);
    std::vector<int> result;
    std::queue<int> q;  // Queue of nodes with no remaining dependencies

    // Calculate initial in-degrees by counting incoming edges
    for (const auto& node : nodes) {
        for (int succ : node.successors) {
            inDegree[nodeIndices.at(succ)]++;
        }
    }

    // Initialize queue with all nodes that have no dependencies (in-degree = 0)
    for (size_t i = 0; i < nodes.size(); i++) {
        if (inDegree[i] == 0) {
            q.push(nodes[i].id);
        }
    }

    // Process nodes in topological order:
    // 1. Take a node with no dependencies
    // 2. Add it to the result
    // 3. Reduce in-degree of all its dependents
    // 4. Add newly independent nodes to the queue
    while (!q.empty()) {
        int curr = q.front();
        q.pop();
        result.push_back(curr);

        // Reduce in-degree of dependent nodes
        // Add them to queue when all prerequisites are processed
        for (int succ : getNode(curr).successors) {
            const auto index = nodeIndices.at(succ);
            inDegree[index]--;
            if (inDegree[index] == 0) {
                q.push(succ);
            }
        }
    }

    if (result.size() != nodes.size()) {
        throw std::runtime_error("Graph contains a cycle");
    }
    return result;
}
