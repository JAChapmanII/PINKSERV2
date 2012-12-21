#ifndef MODULES_SIMPLE_HPP
#define MODULES_SIMPLE_HPP

#include "function.hpp"
#include <string>

// Return one thing or the other
class OrFunction : public Function {
	public:
		OrFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// say yes
class YesFunction : public Function {
	public:
		YesFunction(std::string nick);
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// Tell someone something
class TellFunction : public Function {
	public:
		TellFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

#endif // MODULES_SIMPLE_HPP
