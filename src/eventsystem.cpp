#include "eventsystem.hpp"
using std::vector;
using std::string;
using std::to_string;

#include "expression.hpp"
#include "parser.hpp"
#include "db.hpp"
#include "global.hpp"
#include "err.hpp"
using zidcu::Database;
using zidcu::Statement;

#include <iostream>
using std::cerr;
using std::endl;

static string table = "events";
static bool _tableCreated{false};
static Database &_db = global::db;

static void makeTable() {
	if(_tableCreated) return;
	auto tran = _db.transaction();
	_db.executeVoid("CREATE TABLE IF NOT EXISTS " + table
			+ " (id integer primary key, type integer, body text)");
	_tableCreated = true;
}

void EventSystem::push(EventType etype, Event e) {
	makeTable();
	auto tran = _db.transaction();
	_db.executeVoid("INSERT INTO " + table + "(type, body) VALUES(?1, ?2)",
			(int)etype, e);
}

vector<string> getEventBodies(EventType etype);
vector<string> getEventBodies(EventType etype) {
	makeTable();
	vector<string> bodies;
	auto tran = _db.transaction();
	auto result = _db.execute("SELECT * FROM " + table + " WHERE type = ?1",
			(int)etype);
	while(result.status() == SQLITE_ROW) {
		string body = result.getString(2);
		bodies.push_back(body);
		result.step();
	}
	if(result.status() != SQLITE_DONE) {
		throw make_except("expected result set of " + to_string((int)etype)
				+ ", got: " + to_string(result.status()));
	}
	return bodies;
}

vector<Variable> EventSystem::process(EventType etype) {
	vector<Variable> output;
	auto bodies = getEventBodies(etype);
	for(auto &body : bodies) {
		try {
			if(global::debugEventSystem)
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
	}
	return output;
}

int EventSystem::eventsSize(EventType etype) {
	makeTable();
	auto count = _db.executeScalar<int>(
			"SELECT COUNT(1) FROM " + table + " WHERE type = ?1",
			(int)etype);
	return (count ? *count : 0);
}
Event EventSystem::getEvent(int id) {
	makeTable();
	auto body = _db.executeScalar<string>(
			"SELECT body FROM " + table + " WHERE id = ?1",
			id);
	return (body ? *body : "");
}
void EventSystem::deleteEvent(int id) {
	makeTable();
	cerr << "EventSystem::deleteEvent(" << id << "): deleting \""
		<< getEvent(id) << "\"" << endl;

	auto tran = _db.transaction();
	_db.executeVoid("DELETE FROM " + table + " WHERE id = ?1", id);
}

