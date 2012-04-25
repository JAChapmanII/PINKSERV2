#ifndef MODULES_MATH_HPP
#define MODULES_MATH_HPP

#include "function.hpp"

// lg
class BinaryLogFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};
// set a variable to something
class SetFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};
// erase a variable
class EraseFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};
// list all variables
class ListFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};
// get the value of a variable
class ValueFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// increment a variable
class IncrementFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};
// decrement a variable
class DecrementFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// arbitrary expression function
class MathFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

#endif // MODULES_MATH_HPP
