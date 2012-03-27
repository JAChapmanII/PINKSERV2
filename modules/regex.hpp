#ifndef MODULES_REGEX_HPP
#define MODULES_REGEX_HPP

#include "function.hpp"
#include <vector>
#include <boost/regex.hpp>

// replace all the things!
class ReplaceFunction : public Function {
	public:
		virtual std::string run(FunctionArguments fargs);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};
// regex all the things!
class RegexFunction : public Function {
	public:
		virtual std::string run(FunctionArguments fargs);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

class PredefinedRegexFunction : public Function {
	public:
		PredefinedRegexFunction(std::string iname);
		PredefinedRegexFunction(std::string iname,
				std::string first, std::string second);
		PredefinedRegexFunction(std::string iname,
				std::vector<std::string> first, std::vector<std::string> second);

		std::string push(std::string first, std::string second);

		virtual std::string run(FunctionArguments fargs);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
	protected:
		std::string m_name;
		std::vector<std::string> m_first;
		std::vector<std::string> m_second;
		std::vector<boost::regex> m_replaces;
};

// add predefined regex
class PushFunction : public Function {
	public:
		virtual std::string run(FunctionArguments fargs);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
	protected:
		std::vector<std::string> m_functions;
};
// add predefined regex
class PushXMLFunction : public PushFunction {
	public:
		virtual std::string regex() const;
};

#endif // MODULES_REGEX_HPP
