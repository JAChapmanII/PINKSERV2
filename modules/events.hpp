#ifndef MODULES_EVENTS_HPP
#define MODULES_EVENTS_HPP

#include <vector>
#include "variable.hpp"

// #m: events: event functions

// #f: on: adds an event based trigger
Variable on(std::vector<Variable> arguments);

// #f: in: adds a time based trigger
Variable in(std::vector<Variable> arguments);

// #f: eventCount: return the number of on text events
Variable eventCount(std::vector<Variable> arguments);

// #f: getEvent: return a specific text event
Variable getEvent(std::vector<Variable> arguments);

// #f: eraseEvent: delete a specific text event
Variable eraseEvent(std::vector<Variable> arguments);


#endif // MODULES_EVENTS_HPP
