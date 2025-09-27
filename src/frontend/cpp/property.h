#ifndef PERSE_CPP_PROPERTY
#define PERSE_CPP_PROPERTY

namespace perse {

template<typename T>
class Property {
public:
	Property() = default;
	Property(const T& val) : value(val), is_set(true) {}

	// fixes const char* to string conversion
	template<typename U = T>
    Property(const char* val, typename std::enable_if<std::is_same<U,
		std::string>::value>::type* = nullptr) : value(val), is_set(true) {}
	
	// fixes lambdas in initializer lists
	template<typename U>
	Property(U&& val, typename std::enable_if<std::is_convertible<U, T>::value && !std::is_same<typename std::decay<U>::type, Property>::value>::type* = nullptr)
    : value(std::forward<U>(val)), is_set(true) {}
	
	Property& operator=(const T& val) {
		value = val;
		is_set = true;
		return *this;
	}

	// fixes the lambas in initializer lists fix
	template<typename U>
	typename std::enable_if<std::is_convertible<U, T>::value && !std::is_same<typename std::decay<U>::type, Property>::value, Property&>::type
	operator=(U&& val) {
		value = std::forward<U>(val);
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