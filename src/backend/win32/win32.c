#include "../../library/widget.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef _WIN32
  #define PERSE_API __declspec(dllexport)
#else
  #define PERSE_API __attribute__((visibility("default")))
#endif

static void (*log)(const char* fmt, ...) = NULL;

PERSE_API void widglib_impl_SetLogger(void(*fn)(const char* fmt, ...)) {
	log = fn;
}

PERSE_API void widglib_impl_BackendCreateWidget(perse_widget_t* widget) {
	switch (widget->type) {
		case PERSE_WIDGET_INVALID:
	
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		case PERSE_WIDGET_HORIZONTAL_LAYOUT:
		case PERSE_WIDGET_VERTICAL_LAYOUT:
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
		
		case PERSE_WIDGET_WINDOW: {
			
		}
		case PERSE_WIDGET_MENU_BAR:
		case PERSE_WIDGET_STATUS_BAR:
		
		case PERSE_WIDGET_TAB_PANEL:
		
		case PERSE_GROUP_PANEL:
		case PERSE_SCROLL_PANEL:
		
		case PERSE_ARROW_BUTTON:
		case PERSE_TEXT_BUTTON:
		case PERSE_IMAGE_BUTTON:
		case PERSE_COMBO_BOX:
		
		case PERSE_TEXT_BOX:
		
		case PERSE_DATE_PICKER:
		case PERSE_IP_ADDRESS_PICKER:
		
		case PERSE_IMAGE:
		case PERSE_CANVAS:
		
		case PERSE_PROGRESS_BAR:
		case PERSE_PROPERTY_LIST:
		
		case PERSE_TREE_VIEW:
		
		
	}
}

PERSE_API void widglib_impl_BackendDestroyWidget(perse_widget_t* widget) {
	switch (widget->type) {
		case PERSE_WIDGET_INVALID:
	
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		case PERSE_WIDGET_HORIZONTAL_LAYOUT:
		case PERSE_WIDGET_VERTICAL_LAYOUT:
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
		
		case PERSE_WIDGET_WINDOW:
		case PERSE_WIDGET_MENU_BAR:
		case PERSE_WIDGET_STATUS_BAR:
		
		case PERSE_WIDGET_TAB_PANEL:
		
		case PERSE_GROUP_PANEL:
		case PERSE_SCROLL_PANEL:
		
		case PERSE_ARROW_BUTTON:
		case PERSE_TEXT_BUTTON:
		case PERSE_IMAGE_BUTTON:
		case PERSE_COMBO_BOX:
		
		case PERSE_TEXT_BOX:
		
		case PERSE_DATE_PICKER:
		case PERSE_IP_ADDRESS_PICKER:
		
		case PERSE_IMAGE:
		case PERSE_CANVAS:
		
		case PERSE_PROGRESS_BAR:
		case PERSE_PROPERTY_LIST:
		
		case PERSE_TREE_VIEW:
	}
}

PERSE_API void widglib_impl_BackendSetProperty(perse_widget_t* widget, perse_property_t* prop) {
	switch (widget->type) {
		case PERSE_WIDGET_INVALID:
	
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		case PERSE_WIDGET_HORIZONTAL_LAYOUT:
		case PERSE_WIDGET_VERTICAL_LAYOUT:
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
		
		case PERSE_WIDGET_WINDOW:
		case PERSE_WIDGET_MENU_BAR:
		case PERSE_WIDGET_STATUS_BAR:
		
		case PERSE_WIDGET_TAB_PANEL:
		
		case PERSE_GROUP_PANEL:
		case PERSE_SCROLL_PANEL:
		
		case PERSE_ARROW_BUTTON:
		case PERSE_TEXT_BUTTON:
		case PERSE_IMAGE_BUTTON:
		case PERSE_COMBO_BOX:
		
		case PERSE_TEXT_BOX:
		
		case PERSE_DATE_PICKER:
		case PERSE_IP_ADDRESS_PICKER:
		
		case PERSE_IMAGE:
		case PERSE_CANVAS:
		
		case PERSE_PROGRESS_BAR:
		case PERSE_PROPERTY_LIST:
		
		case PERSE_TREE_VIEW:
	}
}

PERSE_API void widglib_impl_BackendSetSizePos(perse_widget_t* widget) {
	switch (widget->type) {
		case PERSE_WIDGET_INVALID:
	
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		case PERSE_WIDGET_HORIZONTAL_LAYOUT:
		case PERSE_WIDGET_VERTICAL_LAYOUT:
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
		
		case PERSE_WIDGET_WINDOW:
		case PERSE_WIDGET_MENU_BAR:
		case PERSE_WIDGET_STATUS_BAR:
		
		case PERSE_WIDGET_TAB_PANEL:
		
		case PERSE_GROUP_PANEL:
		case PERSE_SCROLL_PANEL:
		
		case PERSE_ARROW_BUTTON:
		case PERSE_TEXT_BUTTON:
		case PERSE_IMAGE_BUTTON:
		case PERSE_COMBO_BOX:
		
		case PERSE_TEXT_BOX:
		
		case PERSE_DATE_PICKER:
		case PERSE_IP_ADDRESS_PICKER:
		
		case PERSE_IMAGE:
		case PERSE_CANVAS:
		
		case PERSE_PROGRESS_BAR:
		case PERSE_PROPERTY_LIST:
		
		case PERSE_TREE_VIEW:
	}
}