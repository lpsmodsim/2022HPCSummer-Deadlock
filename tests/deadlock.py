import sst

TOTAL_NODES = 3

# Component node from element deadlock (deadlock.node), named "node_one"
node_zero = sst.Component("Node 0", "deadlock.node")
node_zero.addParams(
    {
        "queueMaxSize": "3",  # max message queue size.
        "tickFreq": "5ms",  # simulated time node runs at.
        "id": "0",  # id of node
        "total_nodes": f"{TOTAL_NODES}",  # total nodes in simulation
    }
)

node_one = sst.Component("Node 1", "deadlock.node")
node_one.addParams(
    {
        "queueMaxSize": "3",
        "tickFreq": "5ms",
        "id": "1",
        "total_nodes": f"{TOTAL_NODES}",
    }
)

node_two = sst.Component("Node 2", "deadlock.node")
node_two.addParams(
    {
        "queueMaxSize": "3",
        "tickFreq": "5ms",
        "id": "2",
        "total_nodes": f"{TOTAL_NODES}",
    }
)

# Connect the nodes by their ports.
sst.Link("Message_Link_Zero").connect(
    (node_zero, "nextPort", "1ms"), (node_one, "prevPort", "1ms")
)
sst.Link("Message_Link_One").connect(
    (node_one, "nextPort", "1ms"), (node_two, "prevPort", "1ms")
)
sst.Link("Message_Link_Two").connect(
    (node_two, "nextPort", "1ms"), (node_zero, "prevPort", "1ms")
)
