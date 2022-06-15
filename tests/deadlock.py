import sst

TOTAL_NODES = 3

# Component node from element deadlock (deadlock.node), named "node_one"
node_one = sst.Component("Node 0", "deadlock.node")
node_one.addParams(
    {
        "queueMaxSize": "6",  # max message queue size.
        "tickFreq": "2ns",  # simulated time node runs at.
        "id": "0",
        "total_nodes": f"{TOTAL_NODES}",
    }
)

node_two = sst.Component("Node 1", "deadlock.node")
node_two.addParams(
    {
        "queueMaxSize": "3",
        "tickFreq": "8ns",
        "id": "1",
        "total_nodes": f"{TOTAL_NODES}",
    }
)

node_three = sst.Component("Node 2", "deadlock.node")
node_three.addParams(
    {
        "queueMaxSize": "3",
        "tickFreq": "4ns",
        "id": "2",
        "total_nodes": f"{TOTAL_NODES}",
    }
)

# Connect the nodes by their ports.
sst.Link("Message_Link_One").connect(
    (node_one, "nextPort", "1ps"), (node_two, "prevPort", "1ps")
)
sst.Link("Message_Link_Two").connect(
    (node_two, "nextPort", "1ps"), (node_three, "prevPort", "1ps")
)
sst.Link("Message_Link_Three").connect(
    (node_three, "nextPort", "1ps"), (node_one, "prevPort", "1ps")
)
