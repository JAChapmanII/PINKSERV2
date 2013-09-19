#ifndef JOURNAL_HPP
#define JOURNAL_HPP

#include <string>
#include <vector>
#include "global.hpp"

namespace journal {
	enum class EntryType { Text, Join, Quit, Part, Invalid };

	struct Entry {
		long long timestamp{global::now()};
		std::string contents{};
		std::string who{}, where{}, command{}, arguments{};
		EntryType type{EntryType::Invalid};

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
	};

	bool init();
	bool deinit();

	void push(Entry nentry);
	std::vector<Entry> search(std::string regex);
	unsigned size();
}

#endif // JOURNAL_HPP
