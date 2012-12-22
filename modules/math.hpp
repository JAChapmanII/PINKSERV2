#ifndef MODULES_MATH_HPP
#define MODULES_MATH_HPP

#include "function.hpp"

// set a variable to something
class SetFunction : public Function {
	public:
		SetFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};
// erase a variable
class EraseFunction : public Function {
	public:
		EraseFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};
// list all variables
class ListFunction : public Function {
	public:
		ListFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};
// get the value of a variable
class ValueFunction : public Function {
	public:
		ValueFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// increment a variable
class IncrementFunction : public Function {
	public:
		IncrementFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};
// decrement a variable
class DecrementFunction : public Function {
	public:
		DecrementFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

#endif // MODULES_MATH_HPP
