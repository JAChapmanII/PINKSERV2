#include "eventsystem.hpp"
using std::queue;
using std::vector;
using std::string;

#include "expressiontree.hpp"

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

