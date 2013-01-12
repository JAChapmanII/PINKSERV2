#ifndef LOGITEM_HPP
#define LOGITEM_HPP

#include <string>

enum ItemType { Text, Join, Quit, Part };

// TODO: log of logitem
struct LogItem {
	unsigned long timestamp;
	ItemType type;
	std::string who;
	std::string where;
	std::string message;

	// parse a line of IRC to cerate a LogItem
	static LogItem parse(std::string line);

	// determines if this is a message addressed to us
	bool toUs();
	// formats this object as a printable string
	std::string toString();
}

#endif // LOGITEM_HPP
