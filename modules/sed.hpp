#ifndef MODULES_SED_HPP
#define MODULES_SED_HPP

#include <string>
#include "bot.hpp"
#include "modules.hpp"

// #m: sed: regex related functionality

// #f: s: adds an event based trigger
std::string s(Bot *bot, std::string regex);

// #f: push: addds a predefined regex
std::string push(Bot *bot, modules::Word name, std::string regex);


#endif // MODULES_SED_HPP
