/**
 * @file Scheduler.cpp
 * @brief Implementation of the Scheduler class
 */

#include "Scheduler.hpp"
#include "Utility.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <iostream>
#include <iomanip>

Scheduler::Scheduler(const Graph& g, int numClusters, int machinesPerCluster)
    : graph(g), L(numClusters), K(machinesPerCluster) {
    if (numClusters <= 0 || machinesPerCluster <= 0) {
        throw std::invalid_argument("Number of clusters and machines must be positive");
    }
}

std::vector<Scheduler::Assignment> Scheduler::schedule() {
    print_step("Initialization", "Setting up the scheduler");
    std::unordered_set<int> scheduled;
    assignments.clear();

    print_step("Topological Sorting");
    // Get nodes in dependency order (prerequisites before dependent tasks)
    std::vector<int> order = graph.getTopologicalOrder();
    std::cout << "Topological order: ";
    for (int node : order) {
        std::cout << node << " ";
    }
    std::cout << "\n\n";

    print_step("Task Scheduling");
    // Schedule each task in topological order to ensure prerequisites are scheduled first
    for (int nodeId : order) {
        std::cout << "\nScheduling Task " << nodeId << ":\n";

        // Find optimal cluster and machine based on multiple factors
        auto [bestCluster, bestMachine] = findEarliestMachine(nodeId, scheduled);
        std::cout << "  Selected Cluster: " << bestCluster
                 << ", Machine: " << bestMachine << "\n";

        // Calculate start and finish times considering dependencies and machine availability
        double startTime = calculateEST(nodeId, bestCluster, bestMachine, scheduled);
        double finishTime = startTime + graph.getNode(nodeId).execTime;
        std::cout << "  Start Time: " << std::fixed << std::setprecision(1)
                 << startTime << ", Finish Time: " << finishTime << "\n";

        // Record the assignment decision
        assignments.push_back(Assignment{
            nodeId,
            bestCluster,
            bestMachine,
            startTime,
            finishTime
        });

        scheduled.insert(nodeId);
    }

    return assignments;
}

double Scheduler::calculateEST(int nodeId, int clusterId, int machineId,
                             const std::unordered_set<int>& scheduled) {
    double est = 0.0;
    const auto& node = graph.getNode(nodeId);

    // Check all prerequisites to determine earliest possible start time
    for (int predId : node.predecessors) {
        if (scheduled.find(predId) == scheduled.end()) {
            throw std::runtime_error("Predecessor has not been scheduled");
        }

        // Find where this prerequisite task was scheduled
        auto predAssign = std::find_if(assignments.begin(), assignments.end(),
            [predId](const Assignment& a) { return a.nodeId == predId; });

        // Add communication delay if prerequisite is on different cluster
        double commTime = graph.getCommTime(predId, nodeId,
                                          predAssign->clusterId == clusterId);
        double predFinishPlusComm = predAssign->finishTime + commTime;
        est = std::max(est, predFinishPlusComm);
    }

    // Ensure we don't schedule before machine becomes available
    double machineAvailable = 0.0;
    for (const auto& assign : assignments) {
        if (assign.clusterId == clusterId && assign.machineId == machineId) {
            machineAvailable = std::max(machineAvailable, assign.finishTime);
        }
    }

    // Return the later of prerequisite completion and machine availability
    return std::max(est, machineAvailable);
}

std::pair<int, int> Scheduler::findEarliestMachine(int nodeId,
                                                  const std::unordered_set<int>& scheduled) {
    int bestCluster = 0;
    int bestMachine = 0;
    double bestScore = std::numeric_limits<double>::max();

    // Calculate current workload for each cluster for load balancing
    std::vector<double> clusterLoads(L);
    for (int cluster = 0; cluster < L; ++cluster) {
        clusterLoads[cluster] = calculateClusterLoad(cluster);
    }

    // Try all possible cluster and machine combinations
    for (int cluster = 0; cluster < L; ++cluster) {
        for (int machine = 0; machine < K; ++machine) {
            // Calculate earliest possible start time on this machine
            double startTime = calculateEST(nodeId, cluster, machine, scheduled);

            // Scoring system combines multiple factors:
            // 1. timeScore: Prioritize earlier start times
            // 2. loadScore: Penalize overloaded clusters (weighted at 50%)
            // 3. predBonus: Reward keeping dependent tasks in same cluster (-0.5 bonus)
            double timeScore = startTime;
            double loadScore = clusterLoads[cluster] * 0.5;

            // Bonus for keeping dependent tasks in same cluster
            double predBonus = 0.0;
            const auto& node = graph.getNode(nodeId);
            for (int predId : node.predecessors) {
                if (isNodeInCluster(predId, cluster)) {
                    predBonus -= 0.5; // Negative bonus improves (reduces) score
                }
            }

            // Combined placement score (lower is better)
            double totalScore = timeScore + loadScore + predBonus;

            if (totalScore < bestScore) {
                bestScore = totalScore;
                bestCluster = cluster;
                bestMachine = machine;
            }
        }
    }

    std::cout << "  Selected: Cluster " << bestCluster
              << ", Machine " << bestMachine
              << " (Score: " << bestScore << ")\n";

    return {bestCluster, bestMachine};
}

double Scheduler::getTotalExecutionTime() const {
    if (assignments.empty()) return 0.0;
    return std::max_element(assignments.begin(), assignments.end(),
        [](const Assignment& a, const Assignment& b) {
            return a.finishTime < b.finishTime;
        })->finishTime;
}

double Scheduler::calculateClusterLoad(int clusterId) const {
    double load = 0.0;
    for (const auto& assign : assignments) {
        if (assign.clusterId == clusterId) {
            load += graph.getNode(assign.nodeId).execTime;
        }
    }
    return load;
}

bool Scheduler::isNodeInCluster(int nodeId, int clusterId) const {
    for (const auto& assign : assignments) {
        if (assign.nodeId == nodeId && assign.clusterId == clusterId) {
            return true;
        }
    }
    return false;
}
