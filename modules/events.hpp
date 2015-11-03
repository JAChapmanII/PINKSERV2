#ifndef MODULES_EVENTS_HPP
#define MODULES_EVENTS_HPP

#include <string>
#include <vector>
#include "pbrane/variable.hpp"
#include "pbrane/modules.hpp"
#include "bot.hpp"

// #m: events: event functions

// #f: on: adds an event based trigger
std::string on(Bot *bot, modules::Word type, std::string body);

// #f: in: adds a time based trigger
Variable in(Bot *bot, std::vector<Variable> arguments);

// #f: eventCount: return the number of on text events
long eventCount(Bot *bot);

// #f: getEvent: return a specific text event
std::string getEvent(Bot *bot, long id);

// #f: eraseEvent: delete a specific text event
long eraseEvent(Bot *bot, long id);


#endif // MODULES_EVENTS_HPP
