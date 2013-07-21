#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>

namespace config {
	extern std::string startupFile;
	extern std::string logFileName;
	extern std::string errFileName;
	extern std::string chatFileName;
	extern std::string todoFileName;
	extern std::string brainFileName;
	extern std::string journalFileName;

	namespace regex {
		extern std::string toUs;
		extern std::string toUsReplace;
	}
}

#endif // CONFIG_HPP
