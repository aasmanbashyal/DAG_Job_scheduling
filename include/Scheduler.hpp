/**
 * @file Scheduler.hpp
 * @brief Defines the job scheduler for distributed computing clusters
 */

#pragma once
#include "Graph.hpp"
#include <vector>

/**
 * @class Scheduler
 * @brief Schedules jobs across multiple computing clusters
 *
 * This class implements a scheduling algorithm that assigns jobs to machines
 * across multiple clusters while considering execution times, communication
 * costs, and load balancing.
 */
class Scheduler {
public:
    /**
     * @struct Assignment
     * @brief Represents the assignment of a job to a specific machine
     */
    struct Assignment {
        int nodeId;      ///< Job/node ID
        int clusterId;   ///< Assigned cluster ID
        int machineId;   ///< Assigned machine ID
        double startTime; ///< Job start time
        double finishTime; ///< Job finish time
    };

    /**
     * @brief Constructor
     * @param graph Reference to the workflow graph
     * @param numClusters Number of available clusters
     * @param machinesPerCluster Number of machines per cluster
     * @throw std::invalid_argument if cluster or machine count is invalid
     */
    Scheduler(const Graph& graph, int numClusters, int machinesPerCluster);

    /**
     * @brief Computes the schedule for all jobs
     * @return Vector of job assignments
     * @throw std::runtime_error if scheduling fails
     */
    std::vector<Assignment> schedule();

    /**
     * @brief Gets the total execution time of the schedule
     * @return Total execution time including communication costs
     */
    double getTotalExecutionTime() const;

private:
    const Graph& graph;
    int L;  // number of clusters
    int K;  // machines per cluster
    std::vector<Assignment> assignments;

    /**
     * @brief Calculates earliest start time for a job
     * @param nodeId Job/node ID
     * @param clusterId Target cluster
     * @param machineId Target machine
     * @param scheduled Vector indicating which jobs are scheduled
     * @return Earliest possible start time
     */
    double calculateEST(int nodeId, int clusterId, int machineId,
                       const std::vector<bool>& scheduled);

    /**
     * @brief Finds the best machine for a job
     * @param nodeId Job/node ID
     * @param scheduled Vector indicating which jobs are scheduled
     * @return Pair of cluster ID and machine ID
     */
    std::pair<int, int> findEarliestMachine(int nodeId,
                                          const std::vector<bool>& scheduled);

    double calculateClusterLoad(int clusterId) const;
    bool isPredecessorInCluster(int nodeId, int clusterId) const;
};