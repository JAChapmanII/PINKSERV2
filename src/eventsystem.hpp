#ifndef EVENTSYSTEM_HPP
#define EVENTSYSTEM_HPP

#include <string>
#include <vector>
#include "variable.hpp"

enum class EventType { Text, Join, Leave, Nick };
using Event = std::string;

// TODO: timed events

namespace EventSystem {
	void push(EventType etype, Event e);

	std::vector<Variable> process(EventType etype);

	int eventsSize(EventType type);
	Event getEvent(int id);
	void deleteEvent(int id);
}

#endif // EVENTSYSTEM_HPP
