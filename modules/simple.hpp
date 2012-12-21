#ifndef MODULES_SIMPLE_HPP
#define MODULES_SIMPLE_HPP

#include "function.hpp"
#include <string>

// A function to wave to people
class WaveFunction : public Function {
	public:
		WaveFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// A function to provide artificial love
class LoveFunction : public Function {
	public:
		LoveFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// A function to output some fish(es)
class FishFunction : public Function {
	public:
		FishFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// Someone wants a train
class TrainFunction : public Function {
	public:
		TrainFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// WUB WUB WUB WUB WUB
class DubstepFunction : public Function {
	public:
		DubstepFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

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

// Say something
class SayFunction : public Function {
	public:
		SayFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// Tell someone something
class TellFunction : public Function {
	public:
		TellFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

#endif // MODULES_SIMPLE_HPP
