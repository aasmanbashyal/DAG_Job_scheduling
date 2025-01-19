# Job Scheduling System

A  C++ implementation for Workflow Scheduling problem involves optimizing the execution of tasks represented as a Directed Acyclic Graph (DAG) across multiple machines. Each node represents a job, and edges represent dependencies between jobs. The system consists of L homogeneous clusters, each containing K homogeneous machines.


## Project Structure

```
.
├── include/                # Header files
│   ├── Graph.hpp           # Graph class definition
│   ├── Scheduler.hpp       # Scheduler class definition
│   └── Utility.hpp         # Utility functions
├── src/
│   ├── core/
│   │   ├── Graph.cpp      # Graph implementation
│   │   └── Scheduler.cpp  # Scheduler implementation
│   └── main.cpp           # Program entry point
├── extra/
│   ├── dag_diagram.md     # mermaid code for dag
│   └── scheduling-algorithm-flowcharts.md  # mermaid code for flowcart
└── assets/         # flowchart and diagram used in the report
```

## Getting Started

### Prerequisites

- C++ compiler supporting
- Make build system
- 64-bit operating system
- Docker (optional)

### Building Locally

If you prefer to build locally without Docker:

1. Install required packages:
```bash
  apt install g++
```

2. Build and run:
```bash
  # Build the project
  make
  # Run the scheduler
  ./bin/scheduler
```

### Running with Docker

1. Build the Docker image:
```bash
docker build -t job-scheduler .
```

2. Run the container:
```bash
docker run --rm job-scheduler
```

### Usage Example
Modify main.cpp to define your task graph:
```cpp

// Add tasks with execution times
graph.addNode(0, 20.0);  // Task ID: 0, Execution Time: 20ms
graph.addNode(1, 15.0);  // Task ID: 1, Execution Time: 15ms

// Add dependencies with communication costs
graph.addEdge(0, 1, 2.0, 4.0);  // From: 0, To: 1, Same-cluster cost: 2ms, Different-cluster cost: 4ms

```



