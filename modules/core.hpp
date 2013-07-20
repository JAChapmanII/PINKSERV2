#ifndef MODULES_CORE_HPP
#define MODULES_CORE_HPP

#include <iostream>
#include <vector>
#include "variable.hpp"

// #m: core: core functions: coreLoad: coreSave
void coreLoad(std::istream &in);
void coreSave(std::ostream &out);

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

// #f: die: reload bot code without saving brain
Variable die(std::vector<Variable> arguments);

// #f: sleep: stop being awake
Variable sleep(std::vector<Variable> arguments);

#endif // MODULES_CORE_HPP
