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

		Entry() : timestamp(0), contents() {}
		Entry(std::string icontents);
		// parse a line of IRC to cerate a LogItem
		static Entry parse(std::string line);

		EntryType type() const;
		std::string who() const;
		std::string where() const;
		std::string message() const;

		// determines if this is a message addressed to us
		bool toUs();
		// formats this object as a printable string
		std::string toString() const;
		// formats this object for writing to a file
		std::string format() const;
	};

	bool init();
	bool deinit();

	void push(Entry nentry);
	std::vector<Entry> search(std::string regex);
}

#endif // JOURNAL_HPP