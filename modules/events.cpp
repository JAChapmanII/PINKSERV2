#include "events.hpp"
using std::vector;
using std::string;

#include "permission.hpp"
#include "eventsystem.hpp"

string on(Bot *bot, modules::Word type, string body) {
	if((string)type == (string)"text") {
		bot->events.push(EventType::Text, Event{body});
	} else if((string)type == (string)"join") {
		bot->events.push(EventType::Join, Event{body});
	} else if((string)type == (string)"leave") {
		bot->events.push(EventType::Leave, Event{body});
	} else if((string)type == (string)"nick") {
		bot->events.push(EventType::Nick, Event{body});
	} else if((string)type == (string)"startup") {
		bot->events.push(EventType::BotStartup, Event{body});
	} else if((string)type == (string)"shutdown") {
		bot->events.push(EventType::BotShutdown, Event{body});
	} else
		throw (string)"invalid trigger type";
	return "success";
}

Variable in(Bot *, vector<Variable>) {
	throw (string)"error: in not implemented"; // TODO
}

long eventCount(Bot *bot) { return bot->events.eventsSize(EventType::Text); }
string getEvent(Bot *bot, long id) { return bot->events.getEvent(id); }

long eraseEvent(Bot *bot, long id) {
	bot->events.deleteEvent(id);
	return eventCount(bot);
}

