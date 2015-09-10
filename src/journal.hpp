#ifndef JOURNAL_HPP
#define JOURNAL_HPP

#include <string>
#include <vector>
#include <functional>
#include <sqlite3.h>
#include "db.hpp"

enum class EntryType { Text, Join, Quit, Part, Invalid };
enum class ExecuteType { None, Hook, Function, Sent, Unknown };
enum class SentType { Recieved, Sent, Log, Invalid };

struct Entry {
	sqlite_int64 id{-1};
	sqlite_int64 timestamp{0}; // timestamp line occurred
	SentType sent{SentType::Recieved}; // whether we sent or recieved it
	ExecuteType etype{ExecuteType::Unknown}; // reason we executed it
	std::string network{};
	std::string contents{}; // full IRC-raw line

	// inferred properties after parsing
	std::string who{}, where{}, command{}, arguments{};
	EntryType type{EntryType::Invalid};

	Entry(sqlite_int64 itimestamp, std::string inetwork, std::string icontents);
	Entry(sqlite_int64 iid, sqlite_int64 itimestamp, SentType isent,
			ExecuteType ietype, std::string inetwork, std::string icontents);

	// parse contents to determine type, w^3, arguments
	void parse();

	// determines if this is a message addressed to us
	//bool toUs();
	// formats this object as a printable string
	//std::string toString() const;

	// returns the nick portion of who
	std::string nick() const;
};


// predicate types for searching through journal
using EntryPredicate = std::function<bool(Entry &)>;

bool NoopPredicate(Entry &e);

struct RegexPredicate {
	RegexPredicate(std::string regex);

	bool operator()(Entry &e);

	private:
		std::string _regex;
};

struct AndPredicate {
	AndPredicate(EntryPredicate p1, EntryPredicate p2);

	bool operator()(Entry &e);

	private:
		EntryPredicate _p1;
		EntryPredicate _p2;
};

struct Journal {
	Journal(zidcu::Database &db, std::string table = "journal");

	sqlite_int64 upsert(Entry &entry);
	void log(sqlite_int64 ts, std::string msg);

	std::vector<Entry> fetch(EntryPredicate predicate = NoopPredicate,
			int limit = -1);

	sqlite_int64 size();

	private:
		void createTable();

	private:
		zidcu::Database &_db;
		std::string _table;
		bool _tableCreated{false};
};

#endif // JOURNAL_HPP
