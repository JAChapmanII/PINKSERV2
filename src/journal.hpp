#ifndef JOURNAL_HPP
#define JOURNAL_HPP

#include <string>
#include <vector>

namespace journal {
	enum class EntryType { Text, Join, Quit, Part, Invalid };

	// TODO: log of logitem
	struct Entry {
		unsigned long timestamp;
		std::string contents;

		EntryType type() const;
		std::string who() const;
		std::string where() const;
		std::string message() const;

		// parse a line of IRC to cerate a LogItem
		static Entry parse(std::string line);

		// determines if this is a message addressed to us
		bool toUs();
		// formats this object as a printable string
		std::string toString();
	};

	extern std::vector<Entry> entries;

	bool init();
	bool deinit();

	std::vector<Entry> search(std::string regex);
}

#endif // JOURNAL_HPP