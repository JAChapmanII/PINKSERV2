#ifndef UTIL_HPP
#define UTIL_HPP

#include <vector>
#include <string>
#include <map>

namespace util {
	std::vector<std::string> split(std::string str, std::string on = " \t\r\n");
	std::string join(std::vector<std::string> strs, std::string with = ", ");
	std::vector<std::string> subvector(std::vector<std::string> vec,
			size_t s, size_t n);
	std::string trim(std::string str, std::string what = " \t\r\n");

	template<typename T, typename F>
			std::vector<T> filter(std::vector<T> vec, F func);

	template<typename T> T fromString(std::string str);
	template<typename T> std::string asString(T val);

	template<typename T>
			bool contains(std::vector<T> vec, T val);
	template<typename K, typename V>
			bool contains(std::map<K, V> map, K key);
}

#include "util.imp"

#endif // UTIL_HPP
