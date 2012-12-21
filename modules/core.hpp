#ifndef MODULES_CORE_HPP
#define MODULES_CORE_HPP

#include "function.hpp"

// ignore
class IgnoreFunction : public Function {
	public:
		IgnoreFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);

	protected:
		virtual std::ostream &output(std::ostream &out);
		virtual std::istream &input(std::istream &in);
};

// help
class HelpFunction : public Function {
	public:
		HelpFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// shutup
class ShutupFunction : public Function {
	public:
		ShutupFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// unshutup
class UnShutupFunction : public Function {
	public:
		UnShutupFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// kick a user
class KickFunction : public Function {
	public:
		KickFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};


#endif // MODULES_CORE_HPP
