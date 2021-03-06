# Demonstration of Deadlock in SST 

[Structural Simulation Toolkit](https://sst-simulator.org/) (SST) is a Discrete Event Simulation (DES) framework written in C++ and Python. Although SST has historically focused on simulation of High Performance Computing, the DES framework can be used in other scenarios. 

In this repo, [deadlock](https://en.wikipedia.org/wiki/Deadlock) is modeled using SST. 


This demonstration of deadlock relies only on [SST-Core](https://github.com/sstsimulator/sst-core)


# Objectives
- [ ] literature search for instances of this problem
- [ ] identify underlying mathematical or logical conditions that trigger this problem in distributed systems
- [ ] identify minimum features for C++ components needed to create this problem
- [ ] identify minimum size of graph (links, components) to observe the issue
- [ ] identify metrics needed to identify the existence of the problem
- [ ] measure the metrics and demonstrate the detection of the problem
- [ ] use the metrics to identify the problem at various scales (e.g., 10 nodes, 100 nodes, 1000 nodes)

# Context

The intended audience for this documentation has basic familiar with use of SST, C++, Python, Makefiles.

Documentation should explain how the concept of deadlock is mapped to the SST model.
