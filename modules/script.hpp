#ifndef MODULES_SCRIPT_HPP
#define MODULES_SCRIPT_HPP

#include "function.hpp"
#include <boost/regex.hpp>
#include "global.hpp"

// ignore
class OnRegexFunction : public Function {
	public:
		OnRegexFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
		virtual std::string secondary(ChatLine line);

	protected:
		std::ostream &output(std::ostream &out);
		std::istream &input(std::istream &in);

		std::vector<std::string> m_triggers;
		std::vector<std::string> m_scopes;
		std::vector<boost::regex> m_regex;
		std::vector<ChatLine> m_lines;
};

// explain
class ExplainFunction : public Function {
	public:
		ExplainFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// read text
class TextFunction : public Function {
	public:
		TextFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};


#endif // MODULES_SCRIPT_HPP
