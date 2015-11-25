#include "events.hpp"
using std::vector;
using std::string;

#include "eventsystem.hpp"

string on(Bot *bot, modules::Word type, string body) {
	auto eventType = fromHumanReadable((string)type);
	if(eventType == EventType::Invalid)
		throw (string)"invalid trigger type";

	bot->events.push(eventType, Event{body});
	return "success";
}

Variable in(Bot *, vector<Variable>) {
	throw (string)"error: in not implemented"; // TODO
}

long eventCount(Bot *bot) { return bot->events.eventsSize(EventType::Text); }
string getEvent(Bot *bot, long id) {
	return toHumanReadable(bot->events.getEventType(id))
		+ ": " + bot->events.getEvent(id);
}

long eraseEvent(Bot *bot, long id) {
	bot->events.deleteEvent(id);
	return eventCount(bot);
}

