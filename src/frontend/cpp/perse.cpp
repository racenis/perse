#include "perse.h"

extern "C" {
#include "../../library/backend.h"
#include "../../library/layout.h"
}

#include <iostream>

namespace perse {

static Widget(*root_func)() = nullptr;
static perse_widget* current_root = nullptr;

void temp_resize_callback(perse_widget* widget, perse_property*) {
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

// leaving this debug code here until we implement a better tree debug printing
// function in the C part of the library
void recurse(perse_widget_t* widg) {
	if (widg->type == PERSE_WIDGET_LIST_BOX) {
		std::cout <<"---\n";
		for (perse_widget_t* c = widg->child; c; c = c->next){
			for (perse_property_t* p = c->property; p; p = p->next) {
				if (p->name != PERSE_NAME_TITLE) continue;
				std::cout << "pp: " << p->string << std::endl;
			}
			
		}
		std::cout <<"===\n";
		}
	
	
	for (perse_widget_t* c = widg->child; c; c = c->next){
		recurse(c);
	}
}

bool Wait() {
	if (!current_root) {
		auto root_widg = root_func();
		current_root = (perse_widget*)root_widg.ptr;
		
		//recurse(current_root);
		
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

		//std::cout << "\nprev:" << std::endl;
		//recurse(current_root);
		//std::cout << "\nnew:" << std::endl;
		//recurse(new_root);
		perse_MergeTree(current_root, new_root);
		//recurse(current_root);
		//std::cout << "\nmerged:" << std::endl;
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