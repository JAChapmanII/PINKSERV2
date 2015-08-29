#include "eventsystem.hpp"
using std::vector;
using std::string;

#include "expression.hpp"
#include "parser.hpp"
#include "db.hpp"
#include "global.hpp"
using zidcu::Database;
using zidcu::Statement;

#include <iostream>
using std::cerr;
using std::endl;

static string table = "events";
static Statement *_createTable{nullptr};
static Statement *_eventTypeCount{nullptr};
static Statement *_getEvent{nullptr};
static Statement *_getEvents{nullptr};
static Statement *_removeEvent{nullptr};
static Statement *_insertStatement{nullptr};
static bool _tableCreated{false};
static Database &_db = global::db;

static void makeTable() {
	if(_tableCreated) return;
	if(!_createTable) {
		_createTable = new Statement{_db,
			"CREATE TABLE IF NOT EXISTS " + table
			+ " (id integer primary key, type integer, body text)"};
	}
	auto result = _createTable->execute();
	if(result.status() != SQLITE_DONE) {
		cerr << "EventSystem::makeTable: sqlite error: " << result.status() << endl;
		throw result.status();
	}
}
static Statement &eventTypeCount() {
	makeTable();
	if(!_eventTypeCount) {
		_eventTypeCount = new Statement{_db,
			"SELECT COUNT(1) FROM " + table + " WHERE type = ?1"};
	}
	return *_eventTypeCount;
}
static Statement &getEvent() {
	makeTable();
	if(!_getEvent) {
		_getEvent = new Statement{_db,
			"SELECT * FROM " + table + " WHERE id = ?1"};
	}
	return *_getEvent;
}
static Statement &getEvents() {
	makeTable();
	if(!_getEvents) {
		_getEvents = new Statement{_db,
			"SELECT * FROM " + table + " ORDER BY body"};
	}
	return *_getEvents;
}
static Statement &removeEvent() {
	makeTable();
	if(!_removeEvent) {
		_removeEvent = new Statement{_db,
			"DELETE FROM " + table + " WHERE id = ?1"};
	}
	return *_removeEvent;
}
static Statement &insertEvent() {
	makeTable();
	if(!_insertStatement) {
		_insertStatement = new Statement{_db,
			"INSERT INTO " + table + "(type, body) VALUES(?1, ?2)"};
	}
	return *_insertStatement;
}


void EventSystem::push(EventType etype, Event e) {
	auto &statement = insertEvent();
	statement.bind(1, (int)etype);
	statement.bind(2, e);
	auto result = statement.execute();
	if(result.status() != SQLITE_DONE) {
		cerr << "EventSystem::push: sqlite error: " << result.status() << endl;
		throw result.status();
	}
}

vector<Variable> EventSystem::process(EventType etype) {
	vector<Variable> output;
	auto &statement = getEvents();
	statement.bind(1, (int)etype);
	auto result = statement.execute();
	while(result.status() == SQLITE_ROW) {
		try {
			string body = result.getString(2);
			cerr << "EventSystem::process: body: \"" << body << "\"" << endl;

			auto expr = Parser::parseCanonical(body);
			Variable res = expr->evaluate("");
			// TODO: hack
			if(res.type == Type::String && !res.value.s.empty())
				output.push_back(res);
		} catch(ParseException e) {
			// TODO: remove
			cerr << "EventSystem::process: " << e.pretty() << endl;
		} catch(StackTrace e) {
			cerr << "EventSystem::process: " << e.toString() << endl;
		} catch(string &s) {
			cerr << "EventSystem::process: " << s << endl; // TODO
		}
		result.step();
	}
	if(result.status() != SQLITE_DONE) {
		cerr << "EventSystem::process(" << (int)etype << "): sqlite error: "
			<< result.status() << endl;
		throw result.status();
	}
	return output;
}

int EventSystem::eventsSize(EventType etype) {
	auto &statement = eventTypeCount();
	statement.bind(1, (int)etype);
	auto result = statement.execute();
	if(result.status() != SQLITE_ROW) {
		cerr << "EventSystem::eventsSize: sqlite error: " << result.status() << endl;
		throw result.status();
	}
	return result.getInteger(0);
}
Event EventSystem::getEvent(int id) {
	auto &statement = ::getEvent();
	statement.bind(1, id);
	auto result = statement.execute();
	if(result.status() == SQLITE_DONE) { return ""; }
	if(result.status() != SQLITE_ROW) {
		cerr << "EventSystem::getEvent: sqlite error: " << result.status() << endl;
		throw result.status();
	}
	return result.getString(2);
}
void EventSystem::deleteEvent(int id) {
	cerr << "EventSystem::deleteEvent(" << id << "): deleting \""
		<< getEvent(id) << "\"" << endl;

	auto &statement = removeEvent();
	statement.bind(1, id);
	auto result = statement.execute();
	if(result.status() != SQLITE_DONE) {
		cerr << "EventSystem::deleteEvent: sqlite error: " << result.status() << endl;
		throw result.status();
	}
	return;
}

