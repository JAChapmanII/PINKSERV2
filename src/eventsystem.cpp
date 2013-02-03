#include "eventsystem.hpp"
using std::queue;
using std::vector;
using std::string;

EventSystem::EventSystem() : m_queue(), m_events() {
}

void EventSystem::push(TimedEvent e) {
	this->m_queue.push(e);
}
void EventSystem::push(EventType etype, Event e) {
	this->m_events[etype].push_back(e);
}

queue<Variable> EventSystem::process() {
	queue<Variable> output;
	return output;
}
vector<Variable> EventSystem::process(EventType etype, string what) {
	vector<Variable> output;
	return output;
}

