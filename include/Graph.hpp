/**
 * @file Graph.hpp
 * @brief Defines a directed acyclic graph (DAG) for representing job dependencies
 */

#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <stdexcept>

/**
 * @class Graph
 * @brief Represents a directed acyclic graph for modeling job dependencies
 *
 * This class implements a DAG where nodes represent jobs with execution times
 * and edges represent dependencies between jobs with associated communication costs.
 */
class Graph {
public:
    /**
     * @struct Edge
     * @brief Represents a directed edge between two nodes with communication costs
     */
    struct Edge {
        int from;              ///< Source node ID
        int to;               ///< Destination node ID
        double weight;        ///< Communication time within same cluster (t1) - faster
        double t2;            ///< Communication time between different clusters - slower due to network overhead
    };

    /**
     * @struct Node
     * @brief Represents a job node in the graph
     */
    struct Node {
        int id;                        ///< Unique identifier for the node
        double execTime;               ///< Execution time of the job
        std::vector<int> predecessors; ///< IDs of predecessor nodes
        std::vector<int> successors;   ///< IDs of successor nodes
    };

    /**
     * @brief Default constructor
     */
    Graph() = default;

    /**
     * @brief Adds a new node to the graph
     * @param id Unique identifier for the node
     * @param execTime Execution time for the job
     * @throw std::invalid_argument if id is negative
     */
    void addNode(int id, double execTime);

    /**
     * @brief Adds a directed edge between two nodes
     * @param from Source node ID
     * @param to Destination node ID
     * @param t1 Communication time within same cluster
     * @param t2 Communication time between different clusters
     * @throw std::invalid_argument if nodes don't exist or times are negative
     */
    void addEdge(int from, int to, double t1, double t2);

    /**
     * @brief Gets all nodes in the graph
     * @return Constant reference to the vector of nodes
     */
    const std::vector<Node>& getNodes() const { return nodes; }

    /**
     * @brief Gets communication time between two nodes
     * @param from Source node ID
     * @param to Destination node ID
     * @param sameCluster Whether nodes are in the same cluster
     * @return Communication time between the nodes
     * @throw std::out_of_range if edge doesn't exist
     */
    double getCommTime(int from, int to, bool sameCluster) const;

    /**
     * @brief Computes topological ordering of nodes
     * @return Vector of node IDs in topological order
     * @throw std::runtime_error if graph contains cycles
     */
    std::vector<int> getTopologicalOrder() const;

private:
    std::vector<Node> nodes;
    std::unordered_map<std::string, Edge> edges;

    /**
     * @brief Creates a unique key for an edge
     * @param from Source node ID
     * @param to Destination node ID
     * @return String key representing the edge
     */
    std::string makeEdgeKey(int from, int to) const;
};