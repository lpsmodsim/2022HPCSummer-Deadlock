/**
 * Simple node simulation
 *
 * ~~Very Simple example of a circular wait deadlock that will be expanded on.
 * W.I.P.
 * */

#include <sst/core/sst_config.h> 
#include <sst/core/interfaces/stringEvent.h> // Include stringEvent event type.
#include <sst/core/simulation.h>
#include <sst/core/stopAction.h>
#include "node.h" // Element header file.

using SST::Interfaces::StringEvent; 

// Constructor definition
node::node( SST::ComponentId_t id, SST::Params& params) : SST::Component(id) {
	output.init("deadlocksim-" + getName() + "->", 1, 0, SST::Output::STDOUT); // Formatting output for console.

	// Get parameters
	queueMaxSize = params.find<int64_t>("queueMaxSize", 50);
	clock = params.find<std::string>("tickFreq", "10s");
	randSeed = params.find<int64_t>("randseed", 445566);

	// Initialize Variables
	queueCurrSize = 0;
	queueCredits = 1; // Arbitrary non-zero number since this will be overwritten after the first tick.

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
	nextPort = configureLink("nextPort", new SST::Event::Handler<node>(this, &node::handleEvent));
	// Check if port exist. Error out if not3.9.9
	if ( !nextPort ) {
		output.fatal(CALL_INFO, -1, "Failed to configure port 'nextPort'\n");
	}

	// Configure our port for returning credit information to a node.
	prevPort = configureLink("prevPort", new SST::Event::Handler<node>(this, &node::handleEvent));
	// Check if port exist. Error out if not
	if ( !prevPort ) {
		output.fatal(CALL_INFO, -1, "Failed to configure port 'prevPort'\n");
	}
}

// Deconstructor definition
node::~node() {

}


void node::setup() {
	MessageTypes type = CREDIT;
	StatusTypes status = SENDING;
	int numCredits = queueMaxSize - queueCurrSize;
	struct Message msg = { getName(), type, status, numCredits };
	prevPort->send(new MessageEvent(msg));
}

void node::finish() {
	std::cout << getName() << " final queue size: " << queueCurrSize << ". Max queue size is: " << queueMaxSize << std::endl;
	std::cout << getName() << " final credit size: " << queueCredits << std::endl;
}


// Runs every clock tick
bool node::tick( SST::Cycle_t currentCycle ) {
	// Replace with output
	output.output(CALL_INFO, "Sim-Time: %lu--------------------------\n", getCurrentSimTimeNano());
	output.output(CALL_INFO, "Size of queue: %d\n", queueCurrSize);
	output.output(CALL_INFO, "Amount of credits: %d\n", queueCredits);

	// Checking if no credits are available.
	if ( queueCredits <= 0 ) {
		// If the node has no credits, it is idling. Send out a status message to check for deadlock.
		output.output(CALL_INFO, "Status Check\n");

		// Construct Status message.
		MessageTypes type = STATUS;
		StatusTypes status = WAITING;
		int numCredits = queueMaxSize - queueCurrSize;
		struct Message msg = { getName(), type, status, numCredits };
		nextPort->send(new MessageEvent(msg));
		
		//primaryComponentOKToEndSim();
		//return(true);
	}
	

	// Rng and collect messages and add to queue size.
	if (queueCurrSize < queueMaxSize) {
		output.output(CALL_INFO, "Adding a message");
		addMessage();
		std::cout << " queue curr size is now " << queueCurrSize << std::endl;
		sendCredits();
	}

	// Send a message out every tick if the next nodes queue is not full,
	// AND if the node has messages in its queue to send.
	if (queueCredits > 0 && queueCurrSize > 0) {
		output.output(CALL_INFO, "Sending a message. Queue size is now %d\n", queueCurrSize - 1);
		sendMessage();
		sendCredits();
	}

	// Send credits back to previous node.
	return(false);
}

void node::handleEvent(SST::Event *ev) {
	MessageEvent *me = dynamic_cast<MessageEvent*>(ev);
	if ( me != NULL ) {
		switch (me->msg.type)
		{
			case MESSAGE:
				//std::cout << getName() << " is receiving a message from " << me->msg.source_node << std::endl;
				queueCurrSize++;
				sendCredits();
				break;
			case CREDIT:
				//std::cout << getName() << " is receiving # of credits from " << me->msg.source_node << std::endl;
				queueCredits = me->msg.credits;
				break;
			case STATUS:
				// Check which node the message originated from:

				// If the message originated from the same node, the status message has looped through
				// the ring of nodes back to its original sender.
				if (me->msg.source_node == getName()) {
					// All nodes in the ring have status WAITING, a deadlock has occured.
					if (me->msg.status == WAITING) {
						std::cout << getName() << " detected a deadlock. Ending simulation." << std::endl;
						SST::StopAction exit;
						exit.execute();
					}
				} else { 
					//std::cout << getName() << " is pinged with a status from " << me->msg.source_node << std::endl;
					// The node receives a status message from another node.
					// Two situations can happen here:

					// 1. The node receives the status SENDING. A node in the ring can still send messages.
					// 	  In this case the current node sends along the status SENDING.

					//this is not needed. refactor
						if (me->msg.status == SENDING) {
							MessageTypes type = STATUS;
							StatusTypes status = SENDING;
							int numCredits = me->msg.credits;
							struct Message msg = { me->msg.source_node, type, status, numCredits };
							nextPort->send(new MessageEvent(msg));
						}

					// 2. The node receives the status WAITING. In this case the previous node(s) is waiting.
					//	  The current node determines if it can send or if its waiting as well and updates the status before passing the message along.
						else if (me->msg.status == WAITING) {
							if (queueCredits <= 0) {
								// The node cannot send out any messages so it passes the WAITING status forward.
								MessageTypes type = STATUS;
								StatusTypes status = WAITING;
								int numCredits = me->msg.credits;
								struct Message msg = { me->msg.source_node, type, status, numCredits };
								nextPort->send(new MessageEvent(msg));
							} else {
								// The node can still send messages so it discards the STATUS message from the ring.

								//this is not needed. refactor
								MessageTypes type = STATUS;
								StatusTypes status = SENDING;
								int numCredits = me->msg.credits;
								struct Message msg = { me->msg.source_node, type, status, numCredits };
								nextPort->send(new MessageEvent(msg));
							}
						}

					// If the current node can still send messages, update the message with status 'SENDING' and pass the message along.
					// Else the current node can not send, update the message with status 'WAITING' and pass the message along.
				}
				break;
		}
	}
	delete ev;
}

// Simulate sending a single message out to linked component in composition.
void node::sendMessage() {
	queueCurrSize--; 

	// Construct message to send.
	MessageTypes type = MESSAGE;
	StatusTypes status = SENDING;
	int numCredits = queueMaxSize - queueCurrSize;
	struct Message msg = { getName(), type, status, numCredits };

	nextPort->send(new MessageEvent(msg));
}

// Send number of credits left to the previous node.
void node::sendCredits() {
	// Construct credit message to send.
	output.output(CALL_INFO, "Sending credits\n");
	MessageTypes type = CREDIT;
	StatusTypes status = SENDING;
	int numCredits = queueMaxSize - queueCurrSize;
	struct Message msg = { getName(), type, status, numCredits };
	prevPort->send(new MessageEvent(msg));
}

// Simulation purposes, add messages randomly to a nodes queue.
void node::addMessage() {
	int rndNumber;
	rndNumber = (int)(rng->generateNextInt32()); // Generate a random 32-bit integer
	//rndNumber = (rndNumber & 0x0000FFFF) ^ ((rndNumber & 0xFFFF0000) >> 16); // XOR the upper 16 bits with the lower 16 bits.
	rndNumber = abs((int)(rndNumber % 2)); // Generate a integer 0-1.
	std::cout << " of size " << rndNumber;;
	queueCurrSize += rndNumber; // Add messages to queue.
}

struct Message node::constructMsg(std::string source_node, MessageTypes type, StatusTypes status, int numCredits) {
	struct Message msg = { source_node, type, status, numCredits };
	return msg;
}


