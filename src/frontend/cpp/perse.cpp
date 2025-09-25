#include "perse.h"

extern "C" {
#include "../../library/backend.h"
#include "../../library/layout.h"
}

#include <iostream>

namespace perse {

static Widget(*root_func)() = nullptr;
static perse_widget* current_root = nullptr;

void Init() {
	perse_LoadBackend();
}

void SetRoot(Widget(*root)()) {
	root_func = root;
}

void Render() {
	std::cout << "getting root widget" << std::endl;
	auto root_widg = root_func();
	perse_widget* new_root = (perse_widget*)root_widg.ptr;
	
	std::cout << "root is widget " << new_root << std::endl;
	
	if (current_root) {
		perse_MergeTree(current_root, new_root);
	} else {
		current_root = new_root;
	}
	
	std::cout << "calculate layout" << std::endl;
	
	perse_CalculateLayout(current_root);
	
	std::cout << "apply changes" << std::endl;
	
	perse_ApplyChanges(current_root);
}

bool Wait() {
	perse_BackendProcessEvents();
	
	return !perse_BackendShouldQuit();
}

}