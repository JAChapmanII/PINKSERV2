#ifndef EVENTSYSTEM_HPP
#define EVENTSYSTEM_HPP

#include <string>
#include <vector>
#include <queue>
#include <map>
#include "variable.hpp"


enum class EventType { Text, Join, Leave, Nick };

struct Event {
	std::string body;
	Event(std::string ibody) : body(ibody) { }
};

struct TimedEvent {
	uint64_t time;
	std::string body;
	TimedEvent(uint64_t itime, std::string ibody) : time(itime), body(ibody) { }
	bool operator<(const TimedEvent &rhs) const {
		return time < rhs.time;
	}
};

class EventSystem {
	public:
		EventSystem();

		void push(TimedEvent e);
		void push(EventType etype, Event e);

		std::vector<Variable> process();
		std::vector<Variable> process(EventType etype, std::string what);

	protected:
		std::priority_queue<TimedEvent> m_queue;
		std::map<EventType, std::vector<Event>> m_events;
};

#endif // EVENTSYSTEM_HPP
