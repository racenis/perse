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

PERSE_API void perse_impl_BackendProcessEvents() {
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

static perse_widget_t* window(perse_widget_t* widg) {
	while (widg->parent) widg = widg->parent;
	return widg;
}

static perse_widget_t* parent(perse_widget_t* widg) {
	while ((widg = widg->parent) && !widg->system);
	return widg;
}

static perse_property_t* prop(perse_name_t name, perse_widget_t* widg) {
	perse_property_t* p = widg->property;
	while (p->name != name && (p = p->next));
	return p;
}

typedef struct {
	perse_widget_t* widget;
} win32_widget_t;

const int WIN32_WIDGET_SIZE = 1000;
static win32_widget_t win32_widgets[1000];

int AllocateIndex(perse_widget_t* widget) {
	
	// TODO: add free list
	
	int index = -1;
	for (int i = 0; i < WIN32_WIDGET_SIZE; i++) {
		if (win32_widgets[i].widget) continue;
			
		index = i;
		break;
	}
	
	if (index == -1) {
		log("ERROR WIN32:: ran out of WIN32_WIDGET_SIZE\n");
		abort();
	}
	
	win32_widgets[index].widget = widget;
	
	index += 1000;
	
	widget->data = (void*)(long long)index;
	
	return index;
}

void FreeIndex(int index) {
	if (index < 1000 || index >= 1000 + WIN32_WIDGET_SIZE) {
		log("ERROR WIN32:: windows index %i out of bounds\n", index);
		abort();
	}
	
	index -= 1000;
	win32_widgets[index].widget = NULL;
}

perse_widget_t* LookupWidget(int index) {
	if (index < 1000 || index >= 1000 + WIN32_WIDGET_SIZE) {
		log("ERROR WIN32:: windows index %i out of bounds\n", index);
		abort();
	}
	
	index -= 1000;
	return win32_widgets[index].widget;
}


static LRESULT CALLBACK perse_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		
		perse_widget_t* widget = LookupWidget(wmId);
		
		switch (wmEvent) {
			case BN_CLICKED: {
				perse_property_t* p = prop(PERSE_NAME_CALLBACK, widget);
				if (!p) {
					log("ERROR WIN32:: perse_WindowProc button callback missing\n");
				} else if (p->type = PERSE_TYPE_CALLBACK) {
					log("ERROR WIN32:: perse_WindowProc button callback wrong type\n");
				} else {
					p->callback(widget);
				}
			} break;
		}
	} break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

		EndPaint(hwnd, &ps);
	} return 0;
	}
	
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

PERSE_API void widglib_impl_BackendCreateWidget(perse_widget_t* widget) {
	switch (widget->type) {
		case PERSE_WIDGET_INVALID:
			log("ERROR WIN32:: BackendCreateWidget passed in an INVALID\n");
		break;
	
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		case PERSE_WIDGET_HORIZONTAL_LAYOUT:
		case PERSE_WIDGET_VERTICAL_LAYOUT:
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
			// layouts don't need anything!!! fake widgets
			break;
		
		case PERSE_WIDGET_WINDOW: {
			perse_property_t* p = NULL;
			
			const char* title = "libperse window";
			if (p = prop(PERSE_NAME_TITLE, widget)) {
				if (p->type != PERSE_TYPE_STRING) {
					log("ERROR WIN32:: WIDGET_WINDOW property TITLE not string");
				} else {
					title = p->string;
					p->changed = 0;
				}
			}
			
			const char CLASS_NAME[]  = "libperse Window Class";
			
			WNDCLASS wc = {};

			wc.lpfnWndProc   = perse_WindowProc;
			wc.hInstance     = GetModuleHandle(NULL);
			wc.lpszClassName = CLASS_NAME;

			RegisterClass(&wc);

			HWND hwnd = CreateWindowEx(
				0,
				CLASS_NAME,
				title,
				WS_OVERLAPPEDWINDOW,

				widget->actual_pos.x, widget->actual_pos.y,
				widget->current_size.w, widget->current_size.h,

				NULL,       // Parent window
				NULL,       // Menu
				wc.hInstance,  // Instance handle
				NULL        // Additional application data
			);

			if (hwnd == NULL) {
				log("ERROR WIN32:: WIDGET_WINDOW CreateWindowEx failed");
				return;
			}

			ShowWindow(hwnd, SW_NORMAL);
			
			widget->system = hwnd;
		} break;
		case PERSE_WIDGET_MENU_BAR: {
			// TODO: implement
			
			/*
				we need to add menu bar item widget!!
				then we can use that to assemble the menu bar
			*/
			
		} break;
		case PERSE_WIDGET_STATUS_BAR: {
			// TODO: implement
			
			/*
				idk what to even put in here
			*/
			
		} break;
		
		
		case PERSE_WIDGET_TAB_PANEL: {
			// TODO: implement
			
			/*
				this should have tab name property
				
				we should also add note book widget to hold this all together
			*/
			
		} break;
		
		case PERSE_GROUP_PANEL: {
			// TODO: implement
			
			/*
				needs property label to put on top
			*/
			
		} break;
		case PERSE_SCROLL_PANEL: {
			// TODO: implement
			
			/*
				no idea, maybe properties to determine which way to scroll?
			*/
			
		} break;
		
		case PERSE_ARROW_BUTTON: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		case PERSE_TEXT_BUTTON: {
			
			perse_property_t* p = NULL;
			const char* title = "libperse button";
			if (p = prop(PERSE_NAME_TITLE, widget)) {
				if (p->type != PERSE_TYPE_STRING) {
					log("ERROR WIN32:: TEXT_BUTTON property TITLE not string");
				} else {
					title = p->string;
					p->changed = 0;
				}
			}
			
			perse_widget_t* w = window(widget);
			
			HWND hwnd = CreateWindow( 
				"BUTTON",
				title,
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				widget->actual_pos.x, widget->actual_pos.y,
				widget->current_size.w, widget->current_size.h,
				w->system,
				(HMENU)(long long)AllocateIndex(widget),
				(HINSTANCE)GetWindowLongPtr(w->system, GWLP_HINSTANCE), 
				NULL
			);
			
			if (hwnd == NULL) {
				log("ERROR WIN32:: TEXT_BUTTON CreateWindow failed");
				return;
			}

			ShowWindow(hwnd, SW_NORMAL);
			
			widget->system = hwnd;
			
		} break;
		case PERSE_IMAGE_BUTTON: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		case PERSE_COMBO_BOX: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		
		case PERSE_TEXT_BOX: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		
		case PERSE_DATE_PICKER: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		case PERSE_IP_ADDRESS_PICKER: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		
		case PERSE_IMAGE: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		case PERSE_CANVAS: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		
		case PERSE_PROGRESS_BAR: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		case PERSE_PROPERTY_LIST: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		
		case PERSE_TREE_VIEW: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		
		
	}
}

PERSE_API void widglib_impl_BackendDestroyWidget(perse_widget_t* widget) {
	switch (widget->type) {
		case PERSE_WIDGET_INVALID:
			log("ERROR WIN32:: BackendDestroyWidget passed in an INVALID");
		break;
	
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		case PERSE_WIDGET_HORIZONTAL_LAYOUT:
		case PERSE_WIDGET_VERTICAL_LAYOUT:
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
			// layouts don't need anything!!! fake widgets
			break;
		
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
			log("ERROR WIN32:: BackendSetProperty passed in an INVALID");
		break;
	
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		case PERSE_WIDGET_HORIZONTAL_LAYOUT:
		case PERSE_WIDGET_VERTICAL_LAYOUT:
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
			// layouts don't need anything!!! fake widgets
			break;
		
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
			log("ERROR WIN32:: BackendSetSizePos passed in an INVALID");
		break;
	
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		case PERSE_WIDGET_HORIZONTAL_LAYOUT:
		case PERSE_WIDGET_VERTICAL_LAYOUT:
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
			// layouts don't need anything!!! fake widgets
			break;
		
		// I think that for these things in here in the win32 we could just have
		// like a single thing? and that single thing would just change for all.
		
		// because in windows all windows are windows
		
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