#ifndef PERSE_CPP_HOOKS
#define PERSE_CPP_HOOKS

#include <utility>
#include <type_traits>

#include "widget.h"

namespace perse {

void SetContext(std::string);

std::pair<int, std::function<void(int)>> UseState(int);
std::pair<bool, std::function<void(bool)>> UseState(bool);
std::pair<std::string, std::function<void(std::string)>> UseState(std::string);
std::pair<void*, std::function<void(void*)>> UseStateDeletablePtr(void*, void (*destr)(void*));

template <typename T>
std::enable_if_t<
    !std::is_same_v<std::decay_t<T>, int> &&
    !std::is_same_v<std::decay_t<T>, bool> &&
    !std::is_same_v<std::decay_t<T>, std::string>,
    std::pair</*const*/ T/*&*/, std::function<void(T)>>
>
UseState(T initial_value) {
    void* ptr = new T(std::move(initial_value));
    auto [value, setter] = UseStateDeletablePtr(ptr, [](void* p){ delete (T*)p; });
    return {*(T*)value, [setter](T new_value) -> void {
        void* ptr = new T(std::move(new_value));
        setter(ptr);
    }};
}

}

#endif // PERSE_CPP_HOOKS