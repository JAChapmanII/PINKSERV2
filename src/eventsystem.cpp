#include "eventsystem.hpp"
using std::queue;
using std::vector;
using std::string;
using std::istream;
using std::ostream;

#include "expressiontree.hpp"
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
vector<Variable> EventSystem::process(EventType etype, string what) {
	vector<Variable> output;
	for(auto e : this->m_events[etype]) {
		Variable result;
		try {
			ExpressionTree *etree = ExpressionTree::parse(e.body);
			try {
				result = etree->evaluate("");
				// TODO: hack
				if(result.type == Type::String)
					output.push_back(result);
			} catch(string &s) {
				cerr << "EventSystem::process: " << s << endl; // TODO
			}
			delete etree;
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

