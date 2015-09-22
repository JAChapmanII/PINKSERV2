#ifndef BOT_HPP
#define BOT_HPP

#include <functional>
#include <random>
#include <string>
#include <vector>
#include "db.hpp"
#include "dictionary.hpp"
#include "journal.hpp"
#include "pvm.hpp"
#include "eventsystem.hpp"
#include "ngram.hpp"
#include "clock.hpp"

struct Options {
	bool debugSQL{false};
	bool debugEventSystem{false};
	bool debugFunctionBodies{false};
	bool import{false};
	bool importLog{false};

	unsigned int seed{0};
};

struct Bot;
using ExtraSetup = std::function<void(Bot *)>;

struct Bot {
	Bot(zidcu::Database &db, Options opts, Clock clock, ExtraSetup setup);
	~Bot();

	void send(std::string network, std::string target, std::string line,
			bool send = true);

	bool isOwner(std::string nick);
	bool isAdmin(std::string nick);

	std::string evaluate(std::string script, std::string nick);
	void process(std::string network, std::string script, std::string nick, std::string target);

	public:
		bool done{false};
		zidcu::Database &db;
		Options opts;
		Clock clock;

		Journal journal;
		EventSystem events;
		Dictionary dictionary;
		VarStore vars;
		Pvm vm;
		ngramStore ngStore;

		std::mt19937_64 rengine;
};

#endif // BOT_HPP
