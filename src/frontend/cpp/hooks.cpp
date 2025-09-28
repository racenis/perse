#include "hooks.h"

#include "perse.h"

#include <map>
#include <functional>
#include <cstring>

extern "C" {
#include "../../library/perse.h"
}

namespace perse {

enum StateType {
	INT,
	BOOL,
	STRING,
	POINTER
};

struct State {
	union {
		int integer;
		bool boolean;
		char* string; // not using std::string to keep struct as PDO
		void* pointer;
	};
	
	void (*destr)(void*);
	
	StateType type;
};

struct Context {
	std::vector<State> states;
	int current_state = 0;
};

static std::map<std::string, Context*> contexts;
Context* context = nullptr;
	
void SetContext(std::string identifier) {
	context = contexts[identifier];
	if (!context) {
		context = new Context;
		contexts[identifier] = context;
	}
	context->current_state = 0;
}

std::pair<int, std::function<void(int)>> UseState(int initial) {
	if (context->current_state >= context->states.size()) {
		context->states.push_back({.integer = initial, .type = INT});
	}
	
	Context* context_as_of_now = context;
	int context_index = context->current_state;
	context->current_state++;
	
	return {context->states[context_index].integer, [context_as_of_now, context_index](int value) -> void{
		context_as_of_now->states[context_index].integer = value;
		Render();
	}};
}

std::pair<bool, std::function<void(bool)>> UseState(bool initial) {
	if (context->current_state >= context->states.size()) {
		context->states.push_back({.boolean = initial, .type = BOOL});
	}
	
	Context* context_as_of_now = context;
	int context_index = context->current_state;
	context->current_state++;
	
	return {context->states[context_index].boolean, [context_as_of_now, context_index](bool value) -> void{
		context_as_of_now->states[context_index].boolean = value;
		Render();
	}};
}

std::pair<std::string, std::function<void(std::string)>> UseState(std::string initial) {
	if (context->current_state >= context->states.size()) {
		State state;
		state.type = STRING;
		state.string = new char[initial.size() + 1];
		strcpy(state.string, initial.c_str());
		
		context->states.push_back(state);
	}
	
	Context* context_as_of_now = context;
	int context_index = context->current_state;
	context->current_state++;
	
	return {context->states[context_index].string, [context_as_of_now, context_index](std::string value) -> void {
		delete context_as_of_now->states[context_index].string;
		char* cpy = new char[value.size() + 1];
		strcpy(cpy, value.c_str());
		
		context_as_of_now->states[context_index].string = cpy;
		Render();
	}};
}

std::pair<void*, std::function<void(void*)>> UseStateDeletablePtr(void* initial, void (*destr)(void*)) {
	if (context->current_state >= context->states.size()) {
		State state;
		state.type = POINTER;
		state.pointer = initial;
		state.destr = destr;
		context->states.push_back(state);
	}
	
	Context* context_as_of_now = context;
	int context_index = context->current_state;
	context->current_state++;
	
	return {context->states[context_index].pointer, [context_as_of_now, context_index](void* value) -> void {
		auto& context = context_as_of_now->states[context_index];
		context.destr(context.pointer);
		context.pointer = value;
		Render();
	}};
}


}