#ifndef _LIB_SUPPORT_BASICS_h
#define _LIB_SUPPORT_BASICS_h

// Convenience definitions

template<typename T>
inline std::vector<T> &operator<<(std::vector<T> &a, T val) {
	a.push_back(val);	
	return a;
}


template<typename T>
inline std::vector<T> &operator<<(std::vector<T> &a, std::vector<T> const &b) {
	a.insert(a.end(), b.begin(), b.end());
	return a;
}


inline std::vector<std::string> &operator<<(std::vector<std::string> &a, char const *str) {
	a.push_back(str);	
	return a;
}

#endif  // _LIB_SUPPORT_BASICS_h
