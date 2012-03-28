#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace config {
	extern std::string nick;
	extern std::string owner;
	extern std::string reload;
	extern std::string die;
	extern std::string logFileName;
	extern std::string errFileName;
	extern std::string chatFileName;
	extern std::string todoFileName;
	extern std::string brainFileName;
	extern unsigned int maxLineLength;
	extern double markovResponseChance;

	namespace regex {
		extern std::string user;
		extern std::string hmask;
		extern std::string target;

		extern std::string privmsg;
		extern std::string join;
		extern std::string toUs;
		extern std::string toUsReplace;
	}
}

#endif // CONFIG_HPP
