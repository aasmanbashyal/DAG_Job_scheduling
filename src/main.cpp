/**
 * @file main.cpp
 * @brief Main file for job scheduling simulation using a Directed Acyclic Graph (DAG)
 *
 * This program demonstrates the scheduling of interdependent jobs across multiple
 * computing clusters while considering execution times and communication costs.
 */

#include "Graph.hpp"
#include "Scheduler.hpp"
#include "Utility.hpp"
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <string>

/**
 * @brief Creates and returns the workflow DAG
 * @return Graph representing the workflow
 */
Graph createWorkflowGraph()
{
    Graph graph;

    try
    {
        // Add nodes (id, execution_time)
        graph.addNode(0, 20.0);
        graph.addNode(1, 15.0);
        graph.addNode(2, 18.0);
        graph.addNode(3, 12.0);
        graph.addNode(4, 14.0);
        graph.addNode(5, 16.0);
        graph.addNode(6, 10.0);
        graph.addNode(7, 8.0);
        graph.addNode(8, 13.0);
        graph.addNode(9, 11.0);
        graph.addNode(10, 9.0);
        graph.addNode(11, 17.0);
        graph.addNode(12, 15.0);
        // Add edges with varied communication costs (from, to, t1, t2)
        graph.addEdge(0, 1, 2.0, 4.0);
        graph.addEdge(0, 2, 3.0, 6.0);
        graph.addEdge(0, 3, 2.5, 5.0);
        graph.addEdge(1, 4, 2.0, 4.0);
        graph.addEdge(1, 5, 3.0, 6.0);
        graph.addEdge(2, 6, 1.5, 3.0);
        graph.addEdge(2, 7, 2.0, 4.0);
        graph.addEdge(3, 8, 2.5, 5.0);
        graph.addEdge(3, 9, 1.0, 2.0);
        graph.addEdge(4, 10, 2.0, 4.0);
        graph.addEdge(5, 10, 1.5, 3.0);
        graph.addEdge(6, 11, 2.0, 4.0);
        graph.addEdge(7, 11, 1.0, 2.0);
        graph.addEdge(8, 11, 2.5, 5.0);
        graph.addEdge(9, 11, 2.0, 4.0);
        graph.addEdge(10, 12, 1.5, 3.0);
        graph.addEdge(11, 12, 2.0, 4.0);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error creating workflow graph: " << e.what() << std::endl;
        throw;
    }
    return graph;
}

int main()
{
    try
    {
        print_step("Creating Workflow", "Setting up parallel task graph");

        // Configuration
        const int NUM_CLUSTERS = 3;
        const int MACHINES_PER_CLUSTER = 2;

        // Create workflow graph
        Graph graph = createWorkflowGraph();

        // Get number of nodes from the graph's nodes collection
        size_t num_nodes = graph.getNodes().size();

        std::cout << "Created workflow with " << num_nodes
                  << " tasks across " << NUM_CLUSTERS
                  << " clusters with " << MACHINES_PER_CLUSTER
                  << " machines each\n";

        // Create and run scheduler
        Scheduler scheduler(graph, NUM_CLUSTERS, MACHINES_PER_CLUSTER);

        auto assignments = scheduler.schedule();

        // Print detailed assignments
        for (const auto &assign : assignments)
        {
            std::cout << "Job " << assign.nodeId
                      << " -> Cluster " << assign.clusterId
                      << ", Machine " << assign.machineId
                      << " (Start: " << std::fixed << std::setprecision(1)
                      << assign.startTime << ", Finish: "
                      << assign.finishTime << ")\n";
        }

        // Print summary statistics
        std::cout << "\nSummary Statistics:\n";
        std::cout << "==================\n";
        std::cout << "Total tasks scheduled: " << assignments.size() << "\n";
        std::cout << "Total makespan: " << assignments.back().finishTime << " time units\n";
        std::cout << "Average task duration: "
                  << (assignments.back().finishTime / assignments.size())
                  << " time units\n\n";

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
