import sst
import random

NUM_NODES = 100
QUEUE_MIN_SIZE = 50
QUEUE_MAX_SIZE = 51
TICK_MIN_FREQ = 5
TICK_MAX_FREQ = 6
random.seed

nodes = dict()
for x in range(NUM_NODES):
    nodes[f"node_{x}"] = sst.Component(f"Node {x}", "deadlock.node")
    nodes[f"node_{x}"].addParams(
        {
            "queueMaxSize": f"{random.randint(QUEUE_MIN_SIZE, QUEUE_MAX_SIZE)}",
            "tickFreq": f"{random.randint(TICK_MIN_FREQ, TICK_MAX_FREQ)}ms",
            "id": f"{x}",
            "total_nodes": f"{NUM_NODES}",
        }
    )

# "queueMaxSize": f"{random.randint(QUEUE_MIN_SIZE, QUEUE_MAX_SIZE)}",
#            "tickFreq": f"{random.randint(TICK_MIN_FREQ, TICK_MAX_FREQ)}s",

for x in range(NUM_NODES - 1):
    sst.Link(f"Link_{x}").connect(
        (nodes[f"node_{x}"], "nextPort", "1ms"),
        (nodes[f"node_{x + 1}"], "prevPort", "1ms"),
    )

sst.Link(f"Link_{NUM_NODES - 1}").connect(
    (nodes[f"node_{NUM_NODES - 1}"], "nextPort", "1ms"),
    (nodes["node_0"], "prevPort", "1ms"),
)
