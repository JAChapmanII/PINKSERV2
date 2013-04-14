#ifndef MODULES_EVENTS_HPP
#define MODULES_EVENTS_HPP

#include <vector>
#include "variable.hpp"

// #m: events: core functions: none: none

// #f: on: adds an event based trigger
Variable on(std::vector<Variable> arguments);

// #f: in: adds a time based trigger
Variable in(std::vector<Variable> arguments);

#endif // MODULES_EVENTS_HPP
