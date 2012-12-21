#ifndef UTIL_HPP
#define UTIL_HPP

#include <vector>
#include <string>
#include <map>
#include <utility>

namespace util {
	template<typename T> T split(std::string str, std::string on = " \t\r\n");
	std::vector<std::string> split(std::string str, std::string on = " \t\r\n");
	template<typename T> std::string join(T strs, std::string with = ", ");
	std::string join(std::vector<std::string> strs, std::string with = ", ");

	template<typename T> std::vector<T> subvector(std::vector<T> vec,
			size_t s, size_t n);
	template<typename T> std::vector<T> last(std::vector<T> vec, size_t n);

	std::string trim(std::string str, std::string what = " \t\r\n");
	std::string trimWhitespace(std::string str);

	bool startsWith(std::string str, std::string prefix);

	template<typename T> T fromString(std::string str);
	template<typename T> std::string asString(T val);

	template<typename T>
			bool contains(std::vector<T> vec, T val);
	template<typename K, typename V>
			bool contains(std::map<K, V> map, K key);
	template<typename K, typename V>
			bool contains(std::vector<std::pair<K, V>> list, K key);

	namespace file {
		bool exists(std::string filename);
		bool executable(std::string filename);
		//time_t mtime(std::string filename);
	}
}

#include "util.imp"

#endif // UTIL_HPP
