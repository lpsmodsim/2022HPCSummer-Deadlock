#include <sst/core/event.h>

// Message Types
enum MessageTypes {
	MESSAGE,
	STATUS,
};

// Status types
enum StatusTypes {
	SENDING,
	WAITING,
};

// Struct for a Message
struct Message {
	int source_id;
	int dest_id;
	StatusTypes status;
	MessageTypes type;
};

// Struct for a Credit Probe
struct CreditProbe {
	int credits;
};

class MessageEvent : public SST::Event {

public:
	
	void serialize_order(SST::Core::Serialization::serializer &ser) override {
		Event::serialize_order(ser);
		ser & msg.source_id;
		ser & msg.dest_id;
		ser & msg.status;
		ser & msg.type;
	}

	
	MessageEvent(Message msg) :
		Event(),
		msg(msg)
	{}

	
	MessageEvent() {} // For Serialization only

	Message msg; 

	ImplementSerializable(MessageEvent); //
};

class CreditEvent : public SST::Event {

public:

	void serialize_order(SST::Core::Serialization::serializer &ser) override {
		Event::serialize_order(ser);
		ser & probe.credits;
	}

	CreditEvent(CreditProbe probe) :
		Event(),
		probe(probe)
	{}

	CreditEvent() {}

	CreditProbe probe;

	ImplementSerializable(CreditEvent);

};