#ifndef MODULES_CORE_HPP
#define MODULES_CORE_HPP

#include "function.hpp"

// ignore
class IgnoreFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// help
class HelpFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// shutup
class ShutupFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// unshutup
class UnShutupFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// kick a user
class KickFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

#endif // MODULES_CORE_HPP
