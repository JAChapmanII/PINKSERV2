#ifndef MODULES_SCRIPT_HPP
#define MODULES_SCRIPT_HPP

#include "function.hpp"
#include <boost/regex.hpp>
#include "global.hpp"

// ignore
class OnRegexFunction : public Function {
	public:
		OnRegexFunction();
		virtual std::string run(FunctionArguments fargs);
		virtual std::string secondary(FunctionArguments fargs);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
	protected:
		std::ostream &output(std::ostream &out);
		std::istream &input(std::istream &in);

		std::vector<std::string> m_triggers;
		std::vector<std::string> m_scopes;
		std::vector<boost::regex> m_regex;
		std::vector<global::ChatLine> m_lines;
};

// explain
class ExplainFunction : public Function {
	public:
		virtual std::string run(FunctionArguments fargs);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

#endif // MODULES_SCRIPT_HPP
