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
		std::vector<boost::regex> m_regex;
		std::vector<global::ChatLine> m_lines;
};

#endif // MODULES_SCRIPT_HPP
