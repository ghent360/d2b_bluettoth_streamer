/*
 * StringWithHash.h
 *
 *  Created on: Nov 21, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef STRINGWITHHASH_H_
#define STRINGWITHHASH_H_

#include <string>

namespace dbus {

/*
 * This class contains an immutable string together with the computed hash value of that string.
 * The hash value facilitates fast comparison for == or !=.
 *
 * It is safe to use the default copy constructor logic, since the underlying STL string
 * handles the copy and assign operations and the hash value for equal strings is the same.
 *
 * This class is not thread safe.
 */
class StringWithHash {
public:
	StringWithHash() : hash_value_(0) {
	};

	StringWithHash(const char* str) : str_(str) {
		calculateHash();
	}

	StringWithHash(const std::string& str) : str_(str) {
		calculateHash();
	}

	virtual ~StringWithHash() {};

	const char* str() const {
		return str_.c_str();
	}

	size_t length() const {
		return str_.length();
	}

	uint32_t hashValue() const {
		return hash_value_;
	}

	bool equals(const StringWithHash& other) const {
		if (other.hash_value_ != hash_value_) return false;
		return other.str_.compare(str_) == 0;
	}

	void assign(const std::string& str) {
		str_ = str;
		calculateHash();
	}

	void assign(const char* str) {
		str_ = str;
		calculateHash();
	}

	bool operator == (const StringWithHash& other) const {
		return equals(other);
	}

	bool operator != (const StringWithHash& other) const {
		return !equals(other);
	}

	StringWithHash& operator = (const std::string& str) {
		assign(str);
		return *this;
	}

	StringWithHash& operator = (const char* str) {
		assign(str);
		return *this;
	}

	operator const std::string&() const {
		return str_;
	}

	operator const char*() const {
		return str_.c_str();
	}

private:
	void calculateHash() {
		uint32_t hash = 0;

		for (char c : str_) {
		    hash = c + (hash << 6) + (hash << 16) - hash;
		}
		hash_value_ = hash;
	}

	std::string str_;
	uint32_t hash_value_;
};

} /* namespace dbus */

#endif /* STRINGWITHHASH_H_ */
