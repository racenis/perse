#ifndef PERSE_CPP_PROPERTY
#define PERSE_CPP_PROPERTY

namespace perse {

template<typename T>
class Property {
public:
	Property() = default;
	Property(const T& val) : value(val), is_set(true) {}

	template<typename U = T>
    Property(const char* val, typename std::enable_if<std::is_same<U,
		std::string>::value>::type* = nullptr) : value(val), is_set(true) {}
	
	Property& operator=(const T& val) {
		value = val;
		is_set = true;
		return *this;
	}

	operator T() const {
		return value;
	}

	bool set() const {
		return is_set;
	}

	const T& get() const {
		return value;
	}
private:
	T value{};
	bool is_set = false;
};

}

#endif // PERSE_CPP_PROPERTY