#ifndef JOURNAL_HPP
#define JOURNAL_HPP

#include <string>
#include <vector>
#include <functional>
#include "global.hpp"

namespace journal {
	enum class EntryType { Text, Join, Quit, Part, Invalid };
	enum class ExecuteType { None, Hook, Function, Unknown };

	struct Entry {
		long long timestamp{global::now()};
		std::string contents{};
		std::string who{}, where{}, command{}, arguments{};
		EntryType type{EntryType::Invalid};
		ExecuteType etype{ExecuteType::Unknown};

		Entry() = default;
		Entry(std::string icontents);
		Entry(long long itimestamp, std::string icontents);

		static Entry fromLog(std::string line);

		// parse contents to determine type, w^3, arguments
		void parse();

		// determines if this is a message addressed to us
		bool toUs();
		// formats this object as a printable string
		std::string toString() const;
		// formats this object for writing to a journal file
		std::string format() const;

		// returns the nick portion of who
		std::string nick() const;
	};

	using SecondaryPredicate = std::function<bool(Entry &)>;
	bool NoopPredicate(Entry &e);

	bool init();
	bool deinit();

	void push(Entry nentry);
	std::vector<Entry> search(std::string regex,
			SecondaryPredicate predicate = NoopPredicate, int limit = -1);
	unsigned size();

	std::vector<Entry>::iterator jbegin();
	std::vector<Entry>::iterator jend();
}

#endif // JOURNAL_HPP
