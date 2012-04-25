#ifndef DICTIONARY_HPP
#define DICTIONARY_HPP

#include <map>
#include <iostream>

template<typename K, typename V> class Dictionary {
	public:
		Dictionary();
		V fetch(K key);
		K fetch(V value);

		std::istream &read(std::istream &in);
		std::ostream &write(std::ostream &out);

		unsigned size() const;

	protected:
		std::map<K, V> m_fmap;
		std::map<V, K> m_rmap;
};

#include "dictionary.imp"

#endif // DICTIONARY_HPP
