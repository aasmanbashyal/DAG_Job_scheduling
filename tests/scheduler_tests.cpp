#include "Graph.hpp"
#include "Scheduler.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

template <typename Exception, typename Action>
void requireThrows(Action action, const std::string& message) {
    try {
        action();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error(message);
}

const Scheduler::Assignment& assignmentFor(
    const std::vector<Scheduler::Assignment>& assignments, int id) {
    const auto found = std::find_if(assignments.begin(), assignments.end(),
        [id](const auto& assignment) { return assignment.nodeId == id; });
    if (found == assignments.end()) throw std::runtime_error("Missing job assignment");
    return *found;
}

void emptyGraph() {
    Graph graph;
    require(graph.getNodes().empty(), "An empty graph must have no nodes");
    require(graph.getTopologicalOrder().empty(), "An empty graph must have no order");
    Scheduler scheduler(graph, 2, 2);
    require(scheduler.schedule().empty(), "An empty graph must schedule no jobs");
    require(scheduler.getTotalExecutionTime() == 0.0, "Empty makespan must be zero");
}

void sparseIds() {
    Graph graph;
    graph.addNode(40, 3.0);
    graph.addNode(10, 2.0);
    graph.addNode(20, 1.0);
    graph.addEdge(10, 40, 1.0, 4.0);
    graph.addEdge(40, 20, 2.0, 5.0);
    require(graph.getNodes().size() == 3, "ID gaps must not create nodes");
    require(graph.getNode(40).execTime == 3.0, "Lookup must use the node ID");
    require(graph.getTopologicalOrder() == std::vector<int>({10, 40, 20}),
            "Topological order must contain only the original IDs");
    requireThrows<std::out_of_range>([&] { graph.getNode(0); },
                                    "An ID gap must not be an existing node");

    Scheduler scheduler(graph, 2, 2);
    const auto assignments = scheduler.schedule();
    require(assignments.size() == 3, "Only actual nodes may be scheduled");
    for (const auto& assignment : assignments) {
        require(assignment.finishTime - assignment.startTime == graph.getNode(assignment.nodeId).execTime,
                "Duration must come from the actual node ID");
        for (int predecessor : graph.getNode(assignment.nodeId).predecessors) {
            const auto& previous = assignmentFor(assignments, predecessor);
            const double delay = graph.getCommTime(predecessor, assignment.nodeId,
                previous.clusterId == assignment.clusterId);
            require(assignment.startTime >= previous.finishTime + delay,
                    "Assignments must respect predecessor completion and communication");
        }
    }
    const auto repeated = scheduler.schedule();
    require(repeated.size() == assignments.size(), "Scheduling again must not accumulate jobs");
    for (std::size_t i = 0; i < repeated.size(); ++i) {
        require(repeated[i].nodeId == assignments[i].nodeId &&
                repeated[i].clusterId == assignments[i].clusterId &&
                repeated[i].machineId == assignments[i].machineId &&
                repeated[i].startTime == assignments[i].startTime,
                "Repeated scheduling must give the same assignments");
    }
}

void missingEdgeEndpoints() {
    Graph graph;
    graph.addNode(10, 1.0);
    graph.addNode(20, 1.0);
    for (const auto endpoints : {std::pair<int, int>{0, 10}, {10, 19}, {-1, 10}, {10, -1}}) {
        requireThrows<std::invalid_argument>(
            [&] { graph.addEdge(endpoints.first, endpoints.second, 0.0, 0.0); },
            "Missing IDs must be rejected even when below the largest ID");
    }
    require(graph.getNode(10).successors.empty(), "Rejected edges must not alter the graph");
}

void updatingExistingNodeAndEdge() {
    Graph graph;
    graph.addNode(10, 1.0);
    graph.addNode(20, 2.0);
    graph.addEdge(10, 20, 3.0, 4.0);
    graph.addNode(10, 5.0);
    graph.addEdge(10, 20, 6.0, 7.0);
    require(graph.getNodes().size() == 2, "Updating an existing ID must not add a node");
    require(graph.getNode(10).execTime == 5.0, "Node updates must retain the new execution time");
    require(graph.getNode(10).successors == std::vector<int>({20}) &&
            graph.getNode(20).predecessors == std::vector<int>({10}),
            "Updating a node or edge must preserve one dependency");
    require(graph.getCommTime(10, 20, true) == 6.0 && graph.getCommTime(10, 20, false) == 7.0,
            "Updating an edge must replace communication costs");
    require(graph.getTopologicalOrder() == std::vector<int>({10, 20}),
            "Updated dependencies must remain sortable");
}

void rejectsCycles(bool disconnected, bool selfLoop) {
    Graph graph;
    graph.addNode(10, 1.0);
    if (selfLoop) {
        graph.addEdge(10, 10, 0.0, 0.0);
    } else {
        graph.addNode(20, 1.0);
        graph.addEdge(10, 20, 0.0, 0.0);
        graph.addEdge(20, 10, 0.0, 0.0);
    }
    if (disconnected) {
        graph.addNode(30, 1.0);
        graph.addNode(40, 1.0);
        graph.addEdge(30, 40, 0.0, 0.0);
    }
    requireThrows<std::runtime_error>([&] { graph.getTopologicalOrder(); },
                                     "A cycle must not return a partial topological order");
    Scheduler scheduler(graph, 2, 1);
    requireThrows<std::runtime_error>([&] { scheduler.schedule(); },
                                     "Scheduling a cyclic graph must fail");
    require(scheduler.getTotalExecutionTime() == 0.0, "Rejected graph must have no partial schedule");
}

void directPredecessorLocality() {
    Graph graph;
    graph.addNode(10, 0.5);
    graph.addNode(20, 0.25);
    graph.addEdge(10, 20, 0.0, 0.0);
    Scheduler scheduler(graph, 2, 1);
    const auto assignments = scheduler.schedule();
    const auto& parent = assignmentFor(assignments, 10);
    const auto& child = assignmentFor(assignments, 20);
    // The 0.5 direct-predecessor bonus outweighs the parent's 0.25 load penalty.
    // Looking at the parent's own predecessors would find none and choose cluster 1.
    require(parent.clusterId == child.clusterId,
            "The direct predecessor, including a root node, must receive the locality bonus");
}

void disconnectedDag() {
    Graph graph;
    for (int id : {10, 20, 30, 40, 50}) graph.addNode(id, 1.0);
    graph.addEdge(10, 30, 0.0, 0.0);
    graph.addEdge(20, 40, 0.0, 0.0);
    const auto order = graph.getTopologicalOrder();
    require(order.size() == 5 && std::set<int>(order.begin(), order.end()).size() == 5,
            "Disconnected DAG nodes must each appear exactly once");
    for (const auto edge : {std::pair<int, int>{10, 30}, {20, 40}}) {
        require(std::find(order.begin(), order.end(), edge.first) < std::find(order.begin(), order.end(), edge.second),
                "Each component must remain in dependency order");
    }
}

class SilenceScheduler {
    std::ostringstream output;
    std::streambuf* previous = std::cout.rdbuf(output.rdbuf());
public:
    ~SilenceScheduler() { std::cout.rdbuf(previous); }
};
} // namespace

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"empty graph", emptyGraph},
        {"sparse IDs and repeat scheduling", sparseIds},
        {"missing edge endpoints", missingEdgeEndpoints},
        {"existing node and edge updates", updatingExistingNodeAndEdge},
        {"two-node cycle", [] { rejectsCycles(false, false); }},
        {"disconnected cycle", [] { rejectsCycles(true, false); }},
        {"self loop", [] { rejectsCycles(false, true); }},
        {"direct predecessor locality", directPredecessorLocality},
        {"disconnected DAG", disconnectedDag},
    };
    int failures = 0;
    for (const auto& [name, test] : tests) {
        try {
            { SilenceScheduler silence; test(); }
            std::cout << "PASS: " << name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "FAIL: " << name << ": " << error.what() << '\n';
        }
    }
    std::cout << tests.size() - failures << '/' << tests.size() << " tests passed\n";
    return failures == 0 ? 0 : 1;
}
