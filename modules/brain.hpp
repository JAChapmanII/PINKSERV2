#ifndef BRAIN_HPP
#define BRAIN_HPP

#include <fstream>
#include <string>
#include <map>

namespace brain {
	std::ostream &write(std::ostream &out, unsigned &variable);
	std::istream &read(std::istream &in, unsigned &variable);

	std::ostream &write(std::ostream &out, const std::string &variable);
	std::istream &read(std::istream &in, std::string &variable);

	template<typename K, typename V> std::ostream &write(
			std::ostream &out, std::map<K, V> &variable);
	template<typename K, typename V> std::istream &read(
			std::istream &in, std::map<K, V> &variable);

	/*
	template<typename T> std::ostream &write(std::ostream &out, T &variable);
	template<typename T> std::istream &read(std::istream &in, T &variable);
	*/
}

#include "brain.imp"

#endif // BRAIN_HPP
