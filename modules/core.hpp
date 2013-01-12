#ifndef MODULES_CORE_HPP
#define MODULES_CORE_HPP

#include <vector>
#include <string>

// #m: core: core functions: none: none

// #f: irc: issues an IRC command
std::string irc(std::vector<std::string> arguments);

// #f: echo: displays it's arguments
std::string echo(std::vector<std::string> arguments);

// #f: core_or = or: returns a random argument
std::string core_or(std::vector<std::string> arguments);

// #f: rand: returns a random integer in a given range
std::string rand(std::vector<std::string> arguments);

// #f: drand: return a random double in a given range
std::string drand(std::vector<std::string> arguments);

#endif // MODULES_CORE_HPP
