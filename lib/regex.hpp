#ifndef REGEX_HPP
#define REGEX_HPP

#include <string>
#include <boost/regex.hpp>

enum class RegexType { Match, Replace, Invalid };

struct Regex {
	Regex(std::string str);

	RegexType type() const;
	std::string match() const;
	std::string flags() const;

	bool matches(std::string text) const;
	bool execute(std::string text, std::string &result) const;

	private:
		void parse(std::string str);

	private:
		RegexType _type{RegexType::Invalid};
		std::string _match{};
		std::string _replace{};
		std::string _flags{};
		char _delimiter{};
		boost::regex _regex{};
};

#endif // REGEX_HPP
