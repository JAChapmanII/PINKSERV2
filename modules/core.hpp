#ifndef MODULES_CORE_HPP
#define MODULES_CORE_HPP

#include <vector>
#include "variable.hpp"
#include "bot.hpp"

// #m: core: core functions

// #f: help: returns a list of builtin functions, or the help text
std::string help(Bot *bot, std::string function);

// #f: list: list user defined functions
std::string list(Bot *bot);

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

// #f: defined: returns true if a variable is bound
bool defined(Bot *bot, std::string name);

// #f: undefined: returns true if a variable is unbound
bool undefined(Bot *bot, std::string name);

// #f: rm: wipe a variable form existence
void rm(Bot *bot, std::string name);

// #f: sleep: stop being awake
void sleep(Bot *bot);

// #f: jsize: returns size of journal
long jsize(Bot *bot);

// #f: rgrep: returns a random line from the journal
std::string rgrep(Bot *bot, std::string regex);

// #f: rline: returns a random line (with nick info) from the journal
std::string rline(Bot *bot, std::string regex);

// #f: debug: prints result of parsing argument to cerr
std::string debug(std::string text);

// #f: todo: add a todo for jac to implement
std::string todo(Bot *bot, std::string text);

// #f: toint: force a result to be an integer
Variable toint(Variable arg);

// #f: bmess: fuck with bots
std::string bmess(std::vector<Variable> arguments);

// #f: pol: pretty one line
std::string pol(std::string body);

// #f: restart: restart bot
void restart(Bot *bot);

#endif // MODULES_CORE_HPP
