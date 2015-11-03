#ifndef EVENTSYSTEM_HPP
#define EVENTSYSTEM_HPP

#include <string>
#include <vector>
#include "sekisa/db.hpp"
#include "pbrane/variable.hpp"
#include "pbrane/pvm.hpp"

enum class EventType { Text, Join, Leave, Nick, BotStartup, BotShutdown };
using Event = std::string;

// TODO: timed events

struct EventSystem {
	EventSystem(zidcu::Database &db, bool debug, std::string table = "events");

	void push(EventType etype, Event e);

	std::vector<Variable> process(EventType etype, Pvm &vm);

	int eventsSize(EventType type);
	Event getEvent(int id);
	void deleteEvent(int id);

	private:
		void createTables();
		std::vector<std::string> getBodies(EventType etype);

	private:
		zidcu::Database &_db;
		std::string _table;
		bool _debug{false};
		bool _tablesCreated{false};
};

#endif // EVENTSYSTEM_HPP
