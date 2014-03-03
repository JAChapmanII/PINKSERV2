#include "eventsystem.hpp"
using std::queue;
using std::vector;
using std::string;
using std::istream;
using std::ostream;

#include "expression.hpp"
#include "parser.hpp"
#include "brain.hpp"

EventSystem::EventSystem() : m_queue(), m_events() {
}

void EventSystem::push(TimedEvent e) {
	this->m_queue.push(e);
}
void EventSystem::push(EventType etype, Event e) {
	this->m_events[etype].push_back(e);
}

#include <iostream>
using std::cerr;
using std::endl;
vector<Variable> EventSystem::process() {
	vector<Variable> output;
	return output;
}
vector<Variable> EventSystem::process(EventType etype) {
	vector<Variable> output;
	for(auto e : this->m_events[etype]) {
		try {
			auto expr = Parser::parseCanonical(e.body);
			Variable result = expr->evaluate("");
			// TODO: hack
			if(result.type == Type::String && !result.value.s.empty())
				output.push_back(result);
		} catch(ParseException e) {
			// TODO: remove
			cerr << "EventSystem::process: " << e.pretty() << endl;
		} catch(StackTrace e) {
			cerr << "EventSystem::process: " << e.toString() << endl;
		} catch(string &s) {
			cerr << "EventSystem::process: " << s << endl; // TODO
		}
	}
	return output;
}

istream &EventSystem::read(istream &in) {
	size_t size = 0;
	brain::read(in, size);
	for(unsigned i = 0; i < size; ++i) {
		int key = 0;
		brain::read(in, key);
		vector<Event> value;
		brain::read(in, value);

		this->m_events[(EventType)key] = value;
	}

	return in;
}
ostream &EventSystem::write(ostream &out) {
	brain::write(out, (size_t)this->m_events.size());
	for(auto it : this->m_events) {
		brain::write(out, (int)it.first);
		brain::write(out, it.second);
	}
	return out;
}

int EventSystem::eventsSize(EventType type) {
	return this->m_events[type].size();
}
Event EventSystem::getEvent(EventType type, int idx) {
	return this->m_events[type][idx];
}
#include <iostream>
void EventSystem::deleteEvent(EventType type, int idx) {
	std::cerr << "EventSystem::deleteEvent: deleting: \"" <<
		this->m_events[type][idx].body << endl;
	this->m_events[type].erase(this->m_events[type].begin() + idx);
}

