#ifndef PERSE_CPP_HOOKS
#define PERSE_CPP_HOOKS

#include <utility>

#include "widget.h"

namespace perse {

void SetContext(std::string);

std::pair<int, std::function<void(int)>> UseState(int);
std::pair<bool, std::function<void(bool)>> UseState(bool);
std::pair<std::string, std::function<void(std::string)>> UseState(std::string);
std::pair<void*, std::function<void(void*)>> UseStateDeletablePtr(void*, void (*destr)(void*));

template <typename T, typename... Args>
std::pair<const T&, std::function<void(Args&&...)>> UseState(Args&&... args) {
	void* ptr = new T(std::forward<Args>(args)...);
	auto [value, setter] = UseStateDeletablePtr(ptr, [](void* p ){ delete (T*)p; });
	return {*(T*)value, [setter](Args&&... args) -> void {
		void* ptr = new T(std::forward<Args>(args)...);
		setter(ptr);
	}};
}

}

#endif // PERSE_CPP_HOOKS