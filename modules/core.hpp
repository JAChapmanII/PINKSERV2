#ifndef MODULES_CORE_HPP
#define MODULES_CORE_HPP

#include <vector>
#include "variable.hpp"
#include "bot.hpp"

// #m: core: core functions

// #f: help: returns a list of builtin functions, or the help text
std::string help(Bot *bot, std::string function);

// #f: irc: issues an IRC command
void irc(std::string command);

// #f: echo: displays it's arguments
std::string echo(std::string args);

// #f: core_or = or: returns a random argument
Variable core_or(Bot *bot, std::vector<Variable> arguments);

// #f: rand: returns a random integer in a given range
long rand(Bot *bot, long low, long high);

// #f: drand: return a random double in a given range
double drand(Bot *bot, double low, double high);

// #f: type: return a string representation of the variable(s) type(s)
std::string type(std::vector<Variable> arguments);

// #f: undefined: returns true if a variable is unbound
bool undefined(Bot *bot, std::string name);

// #f: rm: wipe a variable form existence
void rm(Bot *bot, std::string name);

// #z: sleep: stop being awake
Variable sleep(std::vector<Variable> arguments);

// #z: jsize: returns size of journal
Variable jsize(std::vector<Variable> arguments);

// #z: rgrep: returns a random line from the journal
Variable rgrep(std::vector<Variable> arguments);

// #z: rline: returns a random line (with nick info) from the journal
Variable rline(std::vector<Variable> arguments);

// #z: debug: prints result of parsing argument to cerr
Variable debug(std::vector<Variable> arguments);

// #z: todo: add a todo for jac to implement
Variable todo(std::vector<Variable> arguments);

// #z: toint: force a result to be an integer
Variable toint(std::vector<Variable> arguments);

// #z: bmess: fuck with bots
Variable bmess(std::vector<Variable> arguments);

// #z: pol: pretty one line
Variable pol(std::vector<Variable> arguments);

#endif // MODULES_CORE_HPP
