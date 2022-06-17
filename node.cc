/**
 * Simulation of deadlock occuring in a ring topology network of nodes.
 *
 * ~~Very Simple example of a circular wait deadlock that will be expanded on.
 * W.I.P.
 * */

#include <sst/core/sst_config.h> 
#include <sst/core/simulation.h>
#include <sst/core/stopAction.h>
#include <queue>
#include "node.h" // Element header file.

//TODO
/*
	Make inititator send out status based on duration based int vs queueCredits == 0 (Less spam in sim).
	Fix message dropped (data race occuring).
	doxygen comments

	Future if time: Three ports allowing more than just a ring topology network.

*/

// Constructor definition
node::node( SST::ComponentId_t id, SST::Params& params) : SST::Component(id) {
	output.init("deadlocksim-" + getName() + "->", 1, 0, SST::Output::STDOUT); // Formatting output for console.

	// Get parameters
	queueMaxSize = params.find<int64_t>("queueMaxSize", 50);
	clock = params.find<std::string>("tickFreq", "10s");
	randSeed = params.find<int64_t>("randseed", 442233);
	node_id = params.find<int64_t>("id", 1);
	total_nodes = params.find<int64_t>("total_nodes", 5);

	// Initialize Variables
	queueCurrSize = 0;
	queueCredits = 0;
	generated = 0;

	// Initialize Random
	rng = new SST::RNG::MarsagliaRNG(10, randSeed); // Create a Marsaglia RNG with a default value and a random seed.

	// Register the node as a primary component.
	// Then declare that the simulation cannot end until this 
	// primary component declares primaryComponentOKToEndSim();
	registerAsPrimaryComponent(); 
	primaryComponentDoNotEndSim();

	// Set Main Clock
	// Handler object is created with a reference to this object and a pointer to
	// a function that is called on every clock tick event (?).
	registerClock(clock, new SST::Clock::Handler<node>(this, &node::tick));
	
	// Configure the port for receiving a message from a node.
	nextPort = configureLink("nextPort", new SST::Event::Handler<node>(this, &node::creditHandler));
	// Check if port exist. Error out if not.
	if ( !nextPort ) {
		output.fatal(CALL_INFO, -1, "Failed to configure port 'nextPort'\n");
	}

	// Configure our port for returning credit information to a node.
	prevPort = configureLink("prevPort", new SST::Event::Handler<node>(this, &node::messageHandler));
	// Check if port exist. Error out if not.
	if ( !prevPort ) {
		output.fatal(CALL_INFO, -1, "Failed to configure port 'prevPort'\n");
	}
}

// Deconstructor definition
node::~node() {

}


void node::setup() {
	output.output(CALL_INFO, "id %d initialized\n", node_id);
	struct CreditProbe creds = { queueMaxSize - (int)msgqueue.size() };
	prevPort->send(new CreditEvent(creds));
}

void node::finish() {
	output.output(CALL_INFO, "Final queue size is %ld | Max queue size is %d | Final credit size is %d\n", msgqueue.size(), queueMaxSize, queueCredits);
	struct Message top = msgqueue.front();
	output.output(CALL_INFO, "Top of queue: Dest_ID-%d\n", top.dest_id);
}

// Runs every clock tick
bool node::tick( SST::Cycle_t currentCycle ) {
	// Replace with output
	output.output(CALL_INFO, "--------------------------Sim-Time: %lu--------------------------\n", getCurrentSimTimeMilli());
	output.output(CALL_INFO, "Size of queue: %ld\n", msgqueue.size());
	output.output(CALL_INFO, "Amount of credits: %d\n", queueCredits);


	// TODO Make this check based off a int that is filled up every tick (duration based?)
	// Checking if no credits are available.
	if ( queueCredits <= 0 && node_id == 0) {
		// If the node has no credits, it is idling. Send out a status message to check for deadlock.
		output.output(CALL_INFO, "Status Check\n");

		// Construct Status message.
		struct Message statusMsg = { 0, 0, WAITING, STATUS };
		nextPort->send(new MessageEvent(statusMsg));

	}

	// Rng and generate message to send out.
	if (queueCredits > 0) {
		addMessage();
	}

	// Send a message out every tick if the next nodes queue is not full,
	// AND if the node           "is_initiator": "0" has messages in its queue to send.
	if ((generated != 1 && (queueCredits > 0 && msgqueue.size() > 0))) {
		sendMessage();
		sendCredits();
	} else if (generated != 1 && !msgqueue.empty()) {
		// Peek at the top message to see if it needs to be delivered to the next node.
		struct Message top = msgqueue.front();
		if (top.dest_id == (node_id + 1) % total_nodes) {
			sendMessage();
			sendCredits();
		}
	}

	generated = 0;
	// Send credits back to previous node.
	return(false);
}

void node::messageHandler(SST::Event *ev) {
	MessageEvent *me = dynamic_cast<MessageEvent*>(ev);
	if ( me != NULL ) {
		switch (me->msg.type)
		{
			case MESSAGE:
				output.output(CALL_INFO, "is receiving a message from node %d.\n", me->msg.source_id);
				output.output(CALL_INFO, "Message Details: SourceID %d | DestID %d\n", me->msg.source_id, me->msg.dest_id);
				//msgqueue.push(me->msg); // Push new message onto queue.
				sendCredits(); // Update previous node with credits available.

				// Check if the message is meant for the node and that the node has correct space.
				// If no space available the message is dropped. (Prevents data races)
				if (me->msg.dest_id != node_id && msgqueue.size() < queueMaxSize) {
					//output.output(CALL_INFO, "Sending a message. Queue size is now %ld\n", msgqueue.size());
					output.output(CALL_INFO, "Message was added to the queue\n");
					msgqueue.push(me->msg);
				} else if (me->msg.dest_id == node_id) {
					output.output(CALL_INFO, "Consumed a message\n");
				} else if (msgqueue.size() >= queueMaxSize) {
					output.output(CALL_INFO, "Message was dropped\n");
				}
				break;
			case STATUS:
				// Check which node the message originated from:
				output.output(CALL_INFO, "Received a STATUS from id %d\n", me->msg.source_id);
				// If the message originated from the same node, the status message has looped through
				// the ring of nodes back to its original sender.
				if (me->msg.source_id == node_id) {
					// All nodes in the ring have status WAITING, and the initiator node is still in a waiting state. A deadlock has occured.
					if (me->msg.status == WAITING && queueCredits <= 0) {
						std::cout << getName() << " detected a deadlock. Ending simulation." << std::endl;
						SST::StopAction exit;
						exit.execute();
					}
				} else { 
					// 2. The node receives the status WAITING. In this case the previous node(s) is waiting.
					//	  The current node determines if it can send or if its waiting as well and updates the status before passing the message along.
						if (me->msg.status == WAITING) {
							struct Message top = msgqueue.front();
							if (queueCredits <= 0 && top.dest_id != ((node_id + 1) % total_nodes)) {
								// The node cannot send out any messages so it passes the WAITING status forward.
								struct Message statusMsg = { me->msg.source_id, me->msg.dest_id, WAITING, STATUS };
								nextPort->send(new MessageEvent(statusMsg));
							} 
						} else {
							output.output(CALL_INFO, "Dropped the status. Can still send.");
						}
				}
				break;
		}
	}
	delete ev; // Clean up event to prevent memory leaks.
}

void node::creditHandler(SST::Event *ev) {
	CreditEvent *ce = dynamic_cast<CreditEvent*>(ev);
	if ( ce != NULL ) {
		queueCredits = ce->probe.credits;
	}
}

// Simulate sending a single message out to linked component in composition.
void node::sendMessage() {
	struct Message msg = msgqueue.front();
	msgqueue.pop();
	// Before sending message, determine if the message was meant for the current node.
	// If it was, consume the message. If not, send the message out.
	std::cout << getName() << " detected a deadlock. Ending simulation." << std::endl;
						SST::StopAction exit;
						exit.execute();
	nextPort->send(new MessageEvent(msg));
	//if (msg.dest_id != node_id) {
	//	output.output(CALL_INFO, "Sending a message. Queue size is now %ld\n", msgqueue.size());
	//} else {
	//	output.output(CALL_INFO, "Consumed a message\n");
	//}
}

// Send number of credits left to the previous node.
void node::sendCredits() {
	// Construct credit message to send.
	output.output(CALL_INFO, "Sending credits\n");
	struct CreditProbe creds = { queueMaxSize - (int)msgqueue.size() };
	prevPort->send(new CreditEvent(creds));
}

// Simulation purposes, generate messages randomly and send to next node.
void node::addMessage() {
	int rndNumber;
	rndNumber = (int)(rng->generateNextInt32()); // Generate a random 32-bit integer
	rndNumber = abs((int)(rndNumber % 2)); // Generate a integer 0-1.
	
	if (rndNumber) {
		// Construct and send a message
		generated = 1;

		// Generate a random destination node that exist in the simulation.
		int rndNode = (int)(rng->generateNextInt32());
		rndNode = abs((int)(rndNode % total_nodes)); // Generate a integer 0-(Total Nodes - 1)
		output.output(CALL_INFO, "Generating a message.\n");
		struct Message newMsg = { node_id, rndNode, SENDING, MESSAGE};
		nextPort->send(new MessageEvent(newMsg));
	}
}


