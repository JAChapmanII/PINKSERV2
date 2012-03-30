#ifndef MODULES_REGEX_HPP
#define MODULES_REGEX_HPP

#include "function.hpp"
#include <vector>
#include <boost/regex.hpp>

// regex all the things!
class RegexFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
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

		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
	protected:
		std::ostream &output(std::ostream &out);
		std::istream &input(std::istream &in);

		std::string m_name;
		std::vector<std::string> m_first;
		std::vector<std::string> m_second;
		std::vector<boost::regex> m_replaces;
};

// add predefined regex
class PushFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
	protected:
		std::ostream &output(std::ostream &out);
		std::istream &input(std::istream &in);
};

// invoke a predefined regex
class InvokeFunction : public Function {
	public:
		virtual std::string secondary(global::ChatLine line);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// list the predefined regexes
class ListRegexesFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};


#endif // MODULES_REGEX_HPP
