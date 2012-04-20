#ifndef MODULES_SIMPLE_HPP
#define MODULES_SIMPLE_HPP

#include "function.hpp"
#include <string>

// A function to wave to people
class WaveFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// A function to provide artificial love
class LoveFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// A function to output some fish(es)
class FishFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// Someone wants a train
class TrainFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// WUB WUB WUB WUB WUB
class DubstepFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// Return one thing or the other
class OrFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// say yes
class YesFunction : public Function {
	public:
		YesFunction(std::string nick);

		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;

	protected:
		std::string m_nick;
};

// Say something
class SayFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// Tell someone something
class TellFunction : public Function {
	public:
		virtual std::string run(global::ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

#endif // MODULES_SIMPLE_HPP
