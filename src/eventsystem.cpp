#include "eventsystem.hpp"
using std::vector;
using std::string;
using std::to_string;
using zidcu::Database;

#include "pbrane/expression.hpp"
#include "pbrane/parser.hpp"
#include "sekisa/db.hpp"
#include "sekisa/err.hpp"

#include <iostream>
using std::cerr;
using std::endl;


string toHumanReadable(EventType type) {
	switch(type) {
		case EventType::Text: return "text";
		case EventType::Join: return "join";
		case EventType::Leave: return "leave";
		case EventType::Nick: return "nick";
		case EventType::BotStartup: return "startup";
		case EventType::BotShutdown: return "shutdown";
	}
	return "invalid";
}
EventType fromHumanReadable(string str) {
	if(str == (string)"text")
		return EventType::Text;
	if(str == (string)"join")
		return EventType::Join;
	if(str == (string)"leave")
		return EventType::Leave;
	if(str == (string)"nick")
		return EventType::Nick;
	if(str == (string)"startup")
		return EventType::BotStartup;
	if(str == (string)"shutdown")
		return EventType::BotShutdown;

	return EventType::Invalid;
}


EventSystem::EventSystem(Database &db, bool debug, string table)
	: _db{db}, _table{table}, _debug{debug} { }

void EventSystem::createTables() {
	if(_tablesCreated) return;
	auto tran = _db.transaction();
	_db.executeVoid("CREATE TABLE IF NOT EXISTS " + _table
			+ " (id integer primary key, type integer, body text)");
	_tablesCreated = true;
}

void EventSystem::push(EventType etype, Event e) {
	this->createTables();
	auto tran = _db.transaction();
	_db.executeVoid("INSERT INTO " + _table + "(type, body) VALUES(?1, ?2)",
			(int)etype, e);
}

vector<string> EventSystem::getBodies(EventType etype) {
	this->createTables();
	vector<string> bodies;
	auto result = _db.execute("SELECT * FROM " + _table + " WHERE type = ?1",
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

vector<Variable> EventSystem::process(EventType etype, Pvm &vm) {
	vector<Variable> output;
	auto bodies = this->getBodies(etype);
	for(auto &body : bodies) {
		try {
			if(_debug)
				cerr << "EventSystem::process: body: \"" << body << "\"" << endl;

			auto expr = Parser::parseCanonical(body);
			Variable res = expr->evaluate(vm, "");
			// TODO: hack
			if(res.type == Type::String && !res.value.empty())
				output.push_back(res);
		} catch(ParseException e) {
			// TODO: remove
			cerr << "EventSystem::process: " << e.pretty() << endl;
		} catch(StackTrace &e) {
			cerr << "EventSystem::process: " << e.toString() << endl;
		} catch(string &s) {
			cerr << "EventSystem::process: " << s << endl; // TODO
		}
	}
	return output;
}

int EventSystem::eventsSize(EventType etype) {
	this->createTables();
	return _db.executeScalar<int>(
			"SELECT COUNT(1) FROM " + _table + " WHERE type = ?1", (int)etype)
		.value_or(0);
}
Event EventSystem::getEvent(int id) {
	this->createTables();
	return _db.executeScalar<string>(
			"SELECT body FROM " + _table + " WHERE id = ?1", id)
		.value_or("");
}
void EventSystem::deleteEvent(int id) {
	this->createTables();
	cerr << "EventSystem::deleteEvent(" << id << "): deleting \""
		<< getEvent(id) << "\"" << endl;

	auto tran = _db.transaction();
	_db.executeVoid("DELETE FROM " + _table + " WHERE id = ?1", id);
}

