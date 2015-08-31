#ifndef MODULES_CORE_HPP
#define MODULES_CORE_HPP

#include <iostream>
#include <vector>
#include "variable.hpp"

// #m: core: core functions

// #f: help: returns a list of builtin functions, or the help text
Variable help(std::vector<Variable> arguments);

// #f: irc: issues an IRC command
Variable irc(std::vector<Variable> arguments);

// #f: echo: displays it's arguments
Variable echo(std::vector<Variable> arguments);

// #f: core_or = or: returns a random argument
Variable core_or(std::vector<Variable> arguments);

// #f: rand: returns a random integer in a given range
Variable rand(std::vector<Variable> arguments);

// #f: drand: return a random double in a given range
Variable drand(std::vector<Variable> arguments);

// #f: type: return a string representation of the variable(s) type(s)
Variable type(std::vector<Variable> arguments);

// #f: undefined: returns true if a variable is unbound
Variable undefined(std::vector<Variable> arguments);

// #f: rm: wipe a variable form existence
Variable rm(std::vector<Variable> arguments);

// #f: sleep: stop being awake
Variable sleep(std::vector<Variable> arguments);

// #f: jsize: returns size of journal
Variable jsize(std::vector<Variable> arguments);

// #f: rgrep: returns a random line from the journal
Variable rgrep(std::vector<Variable> arguments);

// #f: rline: returns a random line (with nick info) from the journal
Variable rline(std::vector<Variable> arguments);

// #f: debug: prints result of parsing argument to cerr
Variable debug(std::vector<Variable> arguments);

// #f: todo: add a todo for jac to implement
Variable todo(std::vector<Variable> arguments);

// #f: toint: force a result to be an integer
Variable toint(std::vector<Variable> arguments);

// #f: bmess: fuck with bots
Variable bmess(std::vector<Variable> arguments);

// #f: pol: pretty one line
Variable pol(std::vector<Variable> arguments);

#endif // MODULES_CORE_HPP
