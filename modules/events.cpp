#include "events.hpp"
using std::istream;
using std::ostream;
using std::vector;

#include <string>
using std::string;

// TODO: C++ time
#include <cstdlib>

#include "global.hpp"
using global::eventSystem;
using global::now;
#include "permission.hpp"
#include "eventsystem.hpp"

void eventsLoad(istream &in) {
	eventSystem.read(in);
}
void eventsSave(ostream &out) {
	eventSystem.write(out);
}

Variable on(vector<Variable> arguments) {
	if(arguments.size() != 2)
		throw (string)"on takes two arguments; see help";
	string type = arguments.front().toString();
	if(type == (string)"text") {
		eventSystem.push(EventType::Text, Event(arguments.back().toString()));
	} else if(type == (string)"join") {
		eventSystem.push(EventType::Join, Event(arguments.back().toString()));
	} else if(type == (string)"leave") {
		eventSystem.push(EventType::Leave, Event(arguments.back().toString()));
	} else if(type == (string)"nick") {
		eventSystem.push(EventType::Nick, Event(arguments.back().toString()));
	} else
		throw (string)"invalid trigger type";
	return Variable("successfully pushed", Permissions());
}

Variable in(vector<Variable> arguments) {
	if(arguments.size() != 2)
		throw (string)"in takes two arguments; see help";
	uint64_t when = time(NULL) + arguments.front().asInteger().value.l;
	eventSystem.push(TimedEvent(when, arguments.back().toString()));
	return Variable("sucessfully pushed", Permissions());
}

