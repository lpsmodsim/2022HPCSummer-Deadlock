#ifndef _node_H
#define _node_H

#include <sst/core/component.h>
#include <sst/core/link.h>
#include <sst/core/rng/marsaglia.h>
#include "CommunicationEvents.h"

class node : public SST::Component {

public: 
	node( SST::ComponentId_t id, SST::Params& params ); // Constructor for a SST component.
	~node(); // Deconstructor

	void setup(); // Setup phase. This function runs after every component has been constructed in the composition.
	void finish(); // Finish phase. This function runs before all the components are cleaned up in the composition.

	bool tick( SST::Cycle_t currentCycle); 

	void messageHandler(SST::Event *ev); // Handles incoming messages and status messages. Determines if a message should be
										 // consumed or added to the node's queue.
	void creditHandler(SST::Event *ev);	// Handles credit information from connected nodes.

	int generated; // Lock so that if a node generates a message it will not also send out a message from its queue as well in one tick.

	
	// Register the component for lookup via sst-info
	SST_ELI_REGISTER_COMPONENT(
		node, // class
		"deadlock", // element library
		"node", // component
		SST_ELI_ELEMENT_VERSION( 1, 0, 0 ), // current element version
		"nodes that send and receive data circularly. Used to demonstrate a communication deadlock.", // description of component.
		COMPONENT_CATEGORY_UNCATEGORIZED // * Not grouped in a category. (No category to filter with via sst-info).
	)
	
	// Parameters, description, default value
	SST_ELI_DOCUMENT_PARAMS(
		{"queueMaxSize", "The size of the node's queue.", "50"},
		{"tickFreq", "The frequency the component is called at.", "10s"},
		{"id", "ID for the node.", "1"},
		{"total_nodes", "Number of nodes in simulation.", "1"},
	)

	// Port name, description, event type
	SST_ELI_DOCUMENT_PORTS(
		{"nextPort", "Port which receives credit probe from the next node.", {"MessageEvent"}},
		{"prevPort", "Port which receives Message info from previous node.", {"CreditEvent"}}
	)

private:
	SST::Output output; // SST Output object for printing to console synchronously (?)

	std::queue<Message> msgqueue; // Queue data type that stores Message structures.
	int queueMaxSize; // Max size of node's queue.
	int queueCurrSize; // Current size of node's queue.
	int queueCredits; // Amount of space left in the next linked node's queue. 

	int64_t randSeed; // Seed for MarsagliaRNG
	SST::RNG::MarsagliaRNG *rng; 

	int node_id; // Node's id
	int total_nodes; // Total number of nodes in simulation.

	void sendMessage(); // Sends a single message across a link from one node to a connected node's queue.
	void sendCredits(); // Sends number of credits to previous node in circular list.
	void addMessage(); // Utilizes RNG to create a message and send it out from a node.

	SST::Link *nextPort; // Pointer to queue port
	SST::Link *prevPort; // Pointer to port that will send # of credits to previous node.

	std::string clock; // Defining a clock which can be described via unit math as a string (?).
};

#endif
