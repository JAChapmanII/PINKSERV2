#ifndef MODULES_FUNCTION_HPP
#define MODULES_FUNCTION_HPP

#include <string>
#include <iostream>
#include <boost/regex.hpp>
#include "chatline.hpp"

// Base class for all other functions
class Function {
	public:
		// Function constructor. Make sure you call the second version in
		// write-able functions or your class will not be written.
		Function(std::string iname, std::string ihelp, std::string iregex);
		Function(std::string iname, std::string ihelp, std::string iregex, bool write);

		// Function deconstructor. Override if you have things to cleanup
		virtual ~Function();

		// This is run when the regex matches.
		virtual std::string run(ChatLine line, boost::smatch matches);

		// This is run when no Function has already matched.
		virtual std::string secondary(ChatLine line);

		// Run on all Functions even if the line has already been handled
		virtual std::string passive(ChatLine line, bool handled);

		// return a human readable name for this function
		std::string name() const;
		// return human readable help for this function
		std::string help() const;
		// return the regex this function uses
		std::string regex() const;

		// used to write this Function. Override output, not this
		friend std::ostream &operator<<(std::ostream &out, Function &function);
		// used to read this Function. Override input, not this
		friend std::istream &operator>>(std::istream &in, Function &function);

	protected:
		// User overridable function to write this function out
		virtual std::ostream &output(std::ostream &out);
		// User overridable function to read this function in
		virtual std::istream &input(std::istream &in);

		// set to true if we should try to write this
		bool m_write;

		// stores important information that all functions must have
		std::string m_name;
		std::string m_help;
		std::string m_regex;
};

#endif // MODULES_FUNCTION_HPP
