#ifndef BOT_HPP
#define BOT_HPP

#include <random>
#include <string>
#include <vector>
#include "db.hpp"
#include "dictionary.hpp"
#include "journal.hpp"
#include "pvm.hpp"
#include "eventsystem.hpp"

struct Options {
	bool debugSQL{false};
	bool debugEventSystem{false};
	bool debugFunctionBodies{false};

	unsigned int seed{0};
};

struct Clock {
	sqlite_int64 now();
};

struct Bot {
	Bot(zidcu::Database &db, Options opts, Clock clock);

	bool done() const;
	void done(bool ndone);

	bool secondaryInit(std::string startupFile);

	void send(std::string network, std::string target, std::string line,
			bool send = true);

	bool isOwner(std::string nick);
	bool isAdmin(std::string nick);

	std::string set(std::string name, std::string val);
	std::string get(std::string name);
	bool defined(std::string name);
	void erase(std::string name);

	sqlite_int64 upsert(Entry &entry);
	std::vector<Entry> fetch(EntryPredicate predicate = NoopPredicate,
			int limit = -1);

	std::vector<Variable> process(EventType etype);

	Pvm &vm();

	private:
		bool _done{false};
		zidcu::Database &_db;
		Options _opts;
		Clock _clock;

		Journal _journal;
		EventSystem _events;
		Dictionary _dictionary;
		VarStore _vars;
		Pvm _vm;

	public:
		std::mt19937_64 rengine;
};

#endif // BOT_HPP
