#ifndef EVENTSYSTEM_HPP
#define EVENTSYSTEM_HPP

#include <string>
#include <vector>
#include <queue>
#include <map>
#include "variable.hpp"


enum class EventType { Text, Join, Leave, Nick };

struct Event {
	std::string context;
	Variable event;
};

struct TimedEvent {
	uint64_t time;
	Variable event;
};

class EventSystem {
	public:
		EventSystem();

		void push(TimedEvent e);
		void push(EventType etype, Event e);

		std::queue<Variable> process();
		std::vector<Variable> process(EventType etype, std::string what);

	protected:
		std::queue<TimedEvent> m_queue;
		std::map<EventType, std::vector<Event>> m_events;
};

#endif // EVENTSYSTEM_HPP
