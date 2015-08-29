#include "events.hpp"
using std::vector;

#include <string>
using std::string;

// TODO: C++ time
#include <cstdlib>

#include "global.hpp"
using global::now;
#include "permission.hpp"
#include "eventsystem.hpp"

Variable on(vector<Variable> arguments) {
	if(arguments.size() != 2)
		throw (string)"on takes two arguments; see help";
	string type = arguments.front().toString();
	if(type == (string)"text") {
		EventSystem::push(EventType::Text, Event(arguments.back().toString()));
	} else if(type == (string)"join") {
		EventSystem::push(EventType::Join, Event(arguments.back().toString()));
	} else if(type == (string)"leave") {
		EventSystem::push(EventType::Leave, Event(arguments.back().toString()));
	} else if(type == (string)"nick") {
		EventSystem::push(EventType::Nick, Event(arguments.back().toString()));
	} else
		throw (string)"invalid trigger type";
	return Variable("successfully pushed", Permissions());
}

Variable in(vector<Variable>) {
	throw (string)"error: in not implemented"; // TODO
}

Variable eventCount(vector<Variable> arguments) {
	if(arguments.size() != 0)
		return Variable("error: eventCount takes no arguments", Permissions());
	return Variable((long)EventSystem::eventsSize(EventType::Text), Permissions());
}

Variable getEvent(vector<Variable> arguments) {
	if(arguments.size() != 1)
		return Variable("error: getEvent takes one argument", Permissions());
	return Variable(EventSystem::getEvent(
				arguments.front().asInteger().value.l), Permissions());
}

Variable eraseEvent(vector<Variable> arguments) {
	if(arguments.size() != 1)
		return Variable("error: eraseEvent takes one argument", Permissions());
	EventSystem::deleteEvent(arguments.front().asInteger().value.l);
	return Variable((long)EventSystem::eventsSize(EventType::Text), Permissions());
}

