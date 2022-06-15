import sst

TOTAL_NODES = 3

# Component node from element deadlock (deadlock.node), named "node_one"
node_zero = sst.Component("Node 0", "deadlock.node")
node_zero.addParams(
    {
        "queueMaxSize": "200",  # max message queue size.
        "tickFreq": "5ns",  # simulated time node runs at.
        "id": "0",  # id of node
        "total_nodes": f"{TOTAL_NODES}",  # total nodes in simulation
    }
)

node_one = sst.Component("Node 1", "deadlock.node")
node_one.addParams(
    {
        "queueMaxSize": "200",
        "tickFreq": "4ns",
        "id": "1",
        "total_nodes": f"{TOTAL_NODES}",
    }
)

node_two = sst.Component("Node 2", "deadlock.node")
node_two.addParams(
    {
        "queueMaxSize": "200",
        "tickFreq": "4ns",
        "id": "2",
        "total_nodes": f"{TOTAL_NODES}",
    }
)

# Connect the nodes by their ports.
sst.Link("Message_Link_Zero").connect(
    (node_zero, "nextPort", "1ps"), (node_one, "prevPort", "1ps")
)
sst.Link("Message_Link_One").connect(
    (node_one, "nextPort", "1ps"), (node_two, "prevPort", "1ps")
)
sst.Link("Message_Link_Two").connect(
    (node_two, "nextPort", "1ps"), (node_zero, "prevPort", "1ps")
)
