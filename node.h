#ifndef _node_H
#define _node_H

#include <sst/core/component.h>
#include <sst/core/link.h>
#include <sst/core/rng/marsaglia.h>
#include "CommunicationEvents.h"

#define QUEUE_NOT_FULL 0
#define QUEUE_FULL 1



class node : public SST::Component {

public: 
	node( SST::ComponentId_t id, SST::Params& params ); // Constructor
	~node(); // Deconstructor

	void setup(); // setup phase
	void finish(); // finish phase

	bool tick( SST::Cycle_t currentCycle); 

	void messageHandler(SST::Event *ev);
	void creditHandler(SST::Event *ev);
	
	// Register the component for lookup via sst-info
	SST_ELI_REGISTER_COMPONENT(
		node, // class
		"deadlock", // element library
		"node", // component
		SST_ELI_ELEMENT_VERSION( 1, 0, 0 ), // current element version
		"nodes that send and receive data circularly. Used to demonstrate a node.", // description of component.
		COMPONENT_CATEGORY_UNCATEGORIZED // * Not grouped in a category. (No category to filter with via sst-info).
	)
	
	// Parameters, description, default value
	SST_ELI_DOCUMENT_PARAMS(
		{"queueMaxSize", "The size of the node's queue.", "50"},
		{"tickFreq", "The frequency the component is called at.", "10s"},
		{"id", "ID for the node.", "1"},
		{"total_nodes", "Number of nodes in simulation.", "1"}
	)

	// Port name, description, event type
	SST_ELI_DOCUMENT_PORTS(
		{"nextPort", "Port which receives credit probe from the next node.", {"MessageEvent"}},
		{"prevPort", "Port which receives Message info from previous node.", {"CreditEvent"}}
	)

private:
	SST::Output output; // SST Output object for printing to console synchronously (?)
	int queueMaxSize; // Max size of nodes queue.
	int queueCurrSize; // Current size of nodes queue.
	int queueCredits; // Amount of space left in the next linked nodes queue. 
	int64_t randSeed; // Seed for MarsagliaRNG
	SST::RNG::MarsagliaRNG *rng; //

	std::queue<Message> msgqueue;
	int node_id; // Node's id
	int total_nodes; // Total number of nodes in simulation.

	void sendMessage(); // Sends a single message across a link from one node to a connected nodes queue.
	void sendCredits(); // Sends number of credits to previous node in circular list.
	void addMessage(); // Utilizes RNG to add messages to each node to simulate messages added from external sources.
	struct Message constructMsg(int source_id, int dest_id, StatusTypes status, MessageTypes type);
	SST::Link *nextPort; // Pointer to queue port
	SST::Link *prevPort; // Pointer to port that will send # of credits to previous node.

	std::string clock; // Defining a clock which can be described via unit math as a string (?).
};

#endif
