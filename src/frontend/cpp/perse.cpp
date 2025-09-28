#include "perse.h"

extern "C" {
#include "../../library/backend.h"
#include "../../library/layout.h"
}

#include <iostream>

namespace perse {

static Widget(*root_func)() = nullptr;
static perse_widget* current_root = nullptr;

void temp_resize_callback(perse_widget* widget) {
	std::cout << "resize callback called" << std::endl;
	if (widget == current_root) Reflow();
}

void Init() {
	perse_LoadBackend();
}

void SetRoot(Widget(*root)()) {
	root_func = root;
}

static bool need_render = false;
static bool need_reflow = false;

void Render() {
	need_render = true;
}

void Reflow() {
	need_reflow = true;
}

bool Wait() {
	if (!current_root) {
		auto root_widg = root_func();
		current_root = (perse_widget*)root_widg.ptr;
		
		perse_CalculateLayout(current_root);
		perse_ApplyChanges(current_root);
	}
	
	perse_BackendProcessEvents();
	
	if (perse_BackendShouldQuit()) {
		return false;
	}
	
	if (need_render) {
		auto root_widg = root_func();
		perse_widget* new_root = (perse_widget*)root_widg.ptr;

		perse_MergeTree(current_root, new_root);
	}
	
	if (need_render || need_reflow) {
		perse_CalculateLayout(current_root);
		perse_ApplyChanges(current_root);
	}
	
	need_render = false;
	need_reflow = false;
	
	return true;
}

}