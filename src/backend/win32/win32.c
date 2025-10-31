#include "../../library/widget.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifdef _WIN32
  #define PERSE_API __declspec(dllexport)
#else
  #define PERSE_API __attribute__((visibility("default")))
#endif

static void (*log)(const char* fmt, ...) = NULL;

static int should_quit = 0;

static HWND main_window = NULL;
static perse_widget_t* main_window_widg = NULL;

PERSE_API void perse_impl_BackendSetLogger(void(*fn)(const char* fmt, ...)) {
	log = fn;
}

PERSE_API void perse_impl_BackendProcessEvents() {
	MSG msg = {};

	if (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

PERSE_API int perse_impl_BackendShouldQuit() {
	return should_quit;
}

// finds widget's window
static perse_widget_t* window(perse_widget_t* widg) {
	// TODO: instead of finding root widget, find one that is a WINDOW type
	while (widg->parent) widg = widg->parent;
	return widg;
}

// finds first ancestor widget that has a hwnd associated with it
static perse_widget_t* parent(perse_widget_t* widg) {
	while ((widg = widg->parent) && !widg->system);
	return widg;
}

// finds a given property
static perse_property_t* prop(perse_name_t name, perse_widget_t* widg) {
	perse_property_t* p = widg->property;
	while (p && p->name != name && (p = p->next));
	return p;
}

// finds an index of a child widget
static int index_in_parent(perse_widget_t* widg) {
	int index = 0;
	for (perse_widget_t* it = widg->parent->child; it; it = it->next) {
		if (it == widg) return index;
		index++;
	}
	return -1;
}

// finds a child widget from an index
static perse_widget_t* child_from_index(perse_widget_t* parent, int index) {
	perse_widget_t* child = parent->child;
	for (int i = 0; child && i <= index; i++, child = child->next) {
		if (i == index) return child;
	}
	
	return NULL;
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
	
	// offsetting the index a bit, so that it doesn't conflict with reserved
	// win32 indexes
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

void recursive_show(perse_widget_t* w) {
	if (w->system) ShowWindow(w->system, SW_SHOW);
	for (perse_widget_t* c = w->child; c; c = c->next){
		recursive_show(c);
	} 
}

void recursive_hide(perse_widget_t* w) {
	if (w->system) ShowWindow(w->system, SW_HIDE);
	for (perse_widget_t* c = w->child; c; c = c->next){
		recursive_hide(c);
	} 
}

static LRESULT CALLBACK perse_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	//log("WIN32:: received %hx\n", uMsg);
	
    switch (uMsg) {
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		
		perse_widget_t* widget = LookupWidget(wmId);
		
		// ignore messages for uninitialized widgets
		if (!widget->system) break;
		
		switch (widget->type) {
			
			// button events
			case PERSE_WIDGET_TEXT_BUTTON:
			switch (wmEvent) {
				case BN_CLICKED: {
					perse_property_t* p = prop(PERSE_NAME_ON_CLICK, widget);
					if (!p) {
						log("ERROR WIN32:: perse_WindowProc button on click missing\n");
					} else if (p->type != PERSE_TYPE_CALLBACK) {
						log("ERROR WIN32:: perse_WindowProc button on click wrong type\n");
					} else {
						p->callback(widget, NULL);
					}
				} break;
			} break;
			
			// listbox events
			case PERSE_WIDGET_LIST_BOX:
			switch (wmEvent) {
				case LBN_SELCHANGE: {
					
					int index = SendMessage(widget->system, LB_GETCURSEL, 0, 0);
					
					
					perse_property_t* p = prop(PERSE_NAME_ON_CLICK, widget);
					if (p && p->type != PERSE_TYPE_CALLBACK) {
						log("ERROR WIN32:: perse_WindowProc listbox on click wrong type\n");
					} else if (p) {
						// TODO: implement
						//p->callback(widget);
						// actually we could call the callback on the LIST with
						// the widget parameter being the ITEM...
						// buut then we need to make up a new name for the
						// callback property!
					}
					
					if (index < 0) break;
					
					perse_widget_t* child = child_from_index(widget, index);
					
					p = prop(PERSE_NAME_ON_CLICK, child);
					if (p && p->type != PERSE_TYPE_CALLBACK) {
						log("ERROR WIN32:: perse_WindowProc listbox on click wrong type\n");
					} else if (p) {
						p->callback(child, NULL);
					}
				
				} break;
			} break;
			
			// textbox events
			case PERSE_WIDGET_TEXT_BOX: {
				switch (wmEvent) {
					case EN_CHANGE: {
						char text_string[256];
						GetWindowText(widget->system, text_string, sizeof(text_string));
						perse_property_t* text = perse_CreatePropertyString(text_string);

						perse_property_t* p = prop(PERSE_NAME_ON_CHANGE, widget);
						if (p && p->type != PERSE_TYPE_CALLBACK) {
							log("ERROR WIN32:: perse_WindowProc textbox on change wrong type\n");
						} else if (p) {
							p->callback(widget, text);
						}
						
						perse_DestroyProperty(text);
						
					} break;
				}
			} break;
		}
	} break;

	case WM_NOTIFY: {
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		
		perse_widget_t* widget = LookupWidget(wmId);
		
		// ignore messages for uninitialized widgets
		if (!widget->system) break;
		
		switch (widget->type) {
			case PERSE_WIDGET_TAB_GROUP: {
				int selection = TabCtrl_GetCurSel(widget->system);
				log("cliclked on tab groop : %i \n", selection);
				
				int child = 0;
				for (perse_widget_t* c = widget->child; c; c = c->next){
					if (child == selection) {
						recursive_show(c);
					} else {
						recursive_hide(c);
					}
					child++;
				}
			} break;
		}
	} break;
	
	case WM_SIZE: {
		
		int new_width = LOWORD(lParam);
		int new_height = HIWORD(lParam);
		
		//log("it is %i by %i\n", new_width, new_height);
		//log("and was %i by %i\n", main_window_widg->current_size.w, main_window_widg->current_size.h);
		
		if (main_window != hwnd) {
			log("ERROR WIN32:: resize generated by not main window\n");
			break;
		}
		
		if (main_window_widg->current_size.w == new_width) {
			if (main_window_widg->current_size.h == new_height) {
				break;
			}
		}
		
		main_window_widg->constraint_size.min.w = new_width;
		main_window_widg->constraint_size.max.w = new_width;
		
		main_window_widg->constraint_size.min.h = new_height;
		main_window_widg->constraint_size.max.h = new_height;
		
		main_window_widg->current_size.w = new_width;
		main_window_widg->current_size.h = new_height;
		main_window_widg->actual_size.w = new_width;
		main_window_widg->actual_size.h = new_height;
		
		perse_property_t* p = prop(PERSE_NAME_ON_RESIZE, main_window_widg);
		if (!p) {
			log("ERROR WIN32:: main window has no ON_RESIZE\n");
		} else if (p->type != PERSE_TYPE_CALLBACK) {
			log("ERROR WIN32:: main window ON_RESIZE wrong type\n");
		} else {
			p->callback(main_window_widg, NULL);
		}
		
	} break;
	
	case WM_CLOSE:
		DestroyWindow(hwnd);
		log("WIN32:: received WM_CLOSE\n");
		break;
	case WM_DESTROY:
		should_quit = 1;
		PostQuitMessage(0);
		log("WIN32:: received WM_DESTROY\n");
		return 0;
	case WM_QUIT:
		should_quit = 1;
		log("WIN32:: received WM_QUIT\n");
		break;
		
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		//FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

		EndPaint(hwnd, &ps);
	} return 0;
	}
	
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK textbox_subclass_handler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    perse_widget_t* widget = LookupWidget(GetDlgCtrlID(hwnd));
	
	switch (msg) {
        case WM_KEYDOWN:
            if (wParam == VK_RETURN) {
				perse_property_t* p = prop(PERSE_NAME_ON_SUBMIT, widget);
				if (p && p->type != PERSE_TYPE_CALLBACK) {
					log("ERROR WIN32:: textbox_subclass_handler textbox on submit wrong type\n");
				} else if (p) {
					p->callback(widget, NULL);
				}
				
                return 0;
            }
            break;
            
        case WM_CHAR:
            if (wParam == VK_RETURN) {
                return 0;
            }
            break;
			
		case WM_NCDESTROY:
            RemoveWindowSubclass(hwnd, textbox_subclass_handler, uIdSubclass);
            break;
    }
    
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

PERSE_API void perse_impl_BackendCreateWidget(perse_widget_t* widget) {
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
			
			// TODO: check if this actually works
			INITCOMMONCONTROLSEX icc = {0};
			icc.dwSize = sizeof(icc);
			icc.dwICC = ICC_WIN95_CLASSES;
			InitCommonControlsEx(&icc);
			
			perse_property_t* p = NULL;
			
			// gather parameters
			const char* title = "libperse window";
			if (p = prop(PERSE_NAME_TITLE, widget)) {
				if (p->type != PERSE_TYPE_STRING) {
					log("ERROR WIN32:: WIDGET_WINDOW property TITLE not string");
				} else {
					title = p->string;
					p->changed = 0;
				}
			}
			
			// set up window class
			const char CLASS_NAME[]  = "libperse Window Class";
			
			WNDCLASS wc = {};

			wc.lpfnWndProc   = perse_WindowProc;
			wc.hInstance     = GetModuleHandle(NULL);
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
			wc.lpszClassName = CLASS_NAME;

			RegisterClass(&wc);
			
			// calculate window size
			DWORD dwStyle = WS_OVERLAPPEDWINDOW;
			DWORD dwExStyle = 0;
			
			RECT desired_rect = { 0, 0, widget->current_size.w, widget->current_size.h};
			
			AdjustWindowRectEx(
				&desired_rect,
				dwStyle,
				FALSE, // set to true if menu
				dwExStyle
			);
			
			int requiredWidth = desired_rect.right - desired_rect.left;
			int requiredHeight = desired_rect.bottom - desired_rect.top;
			
			// create the window
			HWND hwnd = CreateWindowEx(
				dwExStyle,
				CLASS_NAME,
				title,
				dwStyle,

				widget->actual_pos.x, widget->actual_pos.y,
				requiredWidth, requiredHeight,

				NULL,       // Parent window
				NULL,       // Menu
				wc.hInstance,  // Instance handle
				NULL        // Additional application data
			);

			if (hwnd == NULL) {
				log("ERROR WIN32:: WIDGET_WINDOW CreateWindowEx failed");
				return;
			}
			
			// this means that this is the main window
			if (!widget->parent) {
				main_window = hwnd;
				main_window_widg = widget;
			}

			widget->system = hwnd;
			
			ShowWindow(hwnd, SW_NORMAL);
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
		
		
		case PERSE_WIDGET_ITEM: {
			if (!widget->parent) log("ERROR WIN32:: item has no parent");
			
			// !! insertion bug? debug!
			/*for (perse_widget_t* c = widget->parent->child; c; c = c->next){
				perse_property_t* p = prop(PERSE_NAME_TITLE, c);
				log("child: %s\n", p->string);
			}*/
			
			switch (widget->parent->type) {
			case PERSE_WIDGET_LIST_BOX: {
				perse_property_t* p = NULL;
				
				const char* title = "list item";
				if (p = prop(PERSE_NAME_TITLE, widget)) {
					if (p->type != PERSE_TYPE_STRING) {
						log("ERROR WIN32:: WIDGET_ITEM property TITLE not string");
					} else {
						title = p->string;
						p->changed = 0;
					}
				}
				
				void* listbox = widget->parent->system;
				int index = index_in_parent(widget);
				
				int insert = SendMessage(listbox, LB_INSERTSTRING, (WPARAM)index, (LPARAM)title);
				SendMessage(listbox, LB_SETITEMDATA, insert, (LPARAM)widget);
				
				//widget->system = (void*)SendMessage(, LB_ADDSTRING, 0, (LPARAM)title);
				
				// system needs to be set to non-null
				widget->system = (void*)(long long)index + 1;
			} break;
			default:
				log("ERROR WIN32:: item parent unsupported type '%i'\n",
					widget->parent->type);
			}
		} break;
		
		case PERSE_WIDGET_TAB_GROUP : {			
			perse_widget_t* w = window(widget);
			
			HWND hwnd = CreateWindow( 
				WC_TABCONTROL,
				NULL,
				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
				widget->actual_pos.x, widget->actual_pos.y,
				widget->current_size.w, widget->current_size.h,
				w->system,
				(HMENU)(long long)AllocateIndex(widget),
				(HINSTANCE)GetWindowLongPtr(w->system, GWLP_HINSTANCE), 
				NULL
			);
			
			if (hwnd == NULL) {
				log("ERROR WIN32:: TAB_GROUP CreateWindow failed");
				return;
			}

			HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
			
			// sets selected tab to default (hardcoded to zero)
			TabCtrl_SetCurSel(hwnd, 0);
			PostMessage(w->system, WM_NOTIFY, (long long)widget->data, 
				(LPARAM)&(NMHDR){.hwndFrom = hwnd, .idFrom = (long long)widget->data, .code = TCN_SELCHANGE});
			
			widget->system = hwnd;
		} break;
		
		case PERSE_WIDGET_TAB_PANEL: {
			perse_property_t* p = NULL;
			const char* title = "libperse tab";
			if (p = prop(PERSE_NAME_TEXT, widget)) {
				if (p->type != PERSE_TYPE_STRING) {
					log("ERROR WIN32:: TAB_PANEL property TEXT not string");
				} else {
					title = p->string;
					p->changed = 0;
				}
			}
			
			
			// TODO: do a check for parent type
			
			
			TCITEM tie;
            tie.mask = TCIF_TEXT;
            
            tie.pszText = (char*)title; // this should be read only???
            TabCtrl_InsertItem(widget->parent->system, 0, &tie);
			
			/*
				this should have tab name property
				
				we should also add note book widget to hold this all together
			*/
			
		} break;
		
		case PERSE_WIDGET_GROUP_PANEL: {
			// TODO: implement
			
			/*
				needs property label to put on top
			*/
			
		} break;
		case PERSE_WIDGET_SCROLL_PANEL: {
			// TODO: implement
			
			/*
				no idea, maybe properties to determine which way to scroll?
			*/
			
		} break;
		
		case PERSE_WIDGET_ARROW_BUTTON: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		case PERSE_WIDGET_TEXT_BUTTON: {
			
			perse_property_t* p = NULL;
			const char* title = "libperse button";
			if (p = prop(PERSE_NAME_TEXT, widget)) {
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
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
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

			HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
			
			widget->system = hwnd;
		} break;
		case PERSE_WIDGET_IMAGE_BUTTON: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		case PERSE_WIDGET_COMBO_BOX: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		case PERSE_WIDGET_LIST_BOX: {			
			perse_widget_t* w = window(widget);
			
			HWND hwnd = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				"LISTBOX",
				"",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
				widget->actual_pos.x, widget->actual_pos.y,
				widget->current_size.w, widget->current_size.h,
				w->system,
				(HMENU)(long long)AllocateIndex(widget),
				(HINSTANCE)GetWindowLongPtr(w->system, GWLP_HINSTANCE), 
				NULL
			);
			
			if (hwnd == NULL) {
				log("ERROR WIN32:: LIST_BOX CreateWindow failed");
				return;
			}

			widget->system = hwnd;
			
			HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
			
		} break;
		
		case PERSE_WIDGET_TEXT_BOX: {
			
			log ("creating text box\n");
			
			perse_property_t* p = NULL;
			const char* text = "libperse textbox";
			if (p = prop(PERSE_NAME_TEXT, widget)) {
				if (p->type != PERSE_TYPE_STRING) {
					log("ERROR WIN32:: TEXT_BUTTON property TEXT not string");
				} else {
					text = p->string;
					p->changed = 0;
				}
			}
			
			perse_widget_t* w = window(widget);
			
			HWND hwnd = CreateWindowEx( 
				WS_EX_CLIENTEDGE,
				"EDIT",
				text,
				WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
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

			SetWindowSubclass(hwnd, textbox_subclass_handler, 0, 0);
			
			HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
			
			widget->system = hwnd;
			
			log ("finished text box\n");
			
		} break;
		
		case PERSE_WIDGET_LABEL: {
			perse_property_t* p = NULL;
			const char* title = "libperse button";
			if (p = prop(PERSE_NAME_TEXT, widget)) {
				if (p->type != PERSE_TYPE_STRING) {
					log("ERROR WIN32:: TEXT_BUTTON property TITLE not string");
				} else {
					title = p->string;
					p->changed = 0;
				}
			}
			
			perse_widget_t* w = window(widget);
			
			HWND hwnd = CreateWindow( 
				"STATIC",
				title,
				WS_CHILD | WS_VISIBLE,
				widget->actual_pos.x, widget->actual_pos.y,
				widget->current_size.w, widget->current_size.h,
				w->system,
				(HMENU)(long long)AllocateIndex(widget),
				(HINSTANCE)GetWindowLongPtr(w->system, GWLP_HINSTANCE), 
				NULL
			);
			
			if (hwnd == NULL) {
				log("ERROR WIN32:: LABEL CreateWindow failed");
				return;
			}

			HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
			
			widget->system = hwnd;
		} break;
		
		case PERSE_WIDGET_DATE_PICKER: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		case PERSE_WIDGET_IP_ADDRESS_PICKER: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		
		case PERSE_WIDGET_IMAGE: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		case PERSE_WIDGET_CANVAS: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		
		case PERSE_WIDGET_PROGRESS_BAR: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		case PERSE_WIDGET_PROPERTY_LIST: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		
		case PERSE_WIDGET_TREE_VIEW: {
			// TODO: implement
			
			/*
			
			*/
			
		} break;
		
		
	}
}

PERSE_API void perse_impl_BackendDestroyWidget(perse_widget_t* widget) {
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
				
		case PERSE_WIDGET_ITEM: {
			if (!widget->parent) log("ERROR WIN32:: item has no parent when destroy");
			
			switch (widget->parent->type) {
			case PERSE_WIDGET_LIST_BOX: {
				void* listbox = widget->parent->system;
					
				int count = (int)SendMessage(listbox, LB_GETCOUNT, 0, 0);
				if (count == LB_ERR) log("ERROR WIN32:: LB_GETCOUNT return LB_ERR");
				
				int index = -1;
				for (int i = 0; i < count; i++) {
					LPARAM data = SendMessage(listbox, LB_GETITEMDATA, i, 0);
					if (data == (LPARAM)widget) {
						SendMessage(listbox, LB_DELETESTRING, i, 0);
						break;
					}
				}
				
				widget->system = NULL;
			} break;
			default:
				log("ERROR WIN32:: item parent unsupported type '%i'\n",
					widget->parent->type);
			}
		} break;
		
		case PERSE_WIDGET_TAB_PANEL:
			// doesn't have a win32 control attached
			break;
		
		case PERSE_WIDGET_WINDOW:
		case PERSE_WIDGET_MENU_BAR:
		case PERSE_WIDGET_STATUS_BAR:
		
		
		case PERSE_WIDGET_TAB_GROUP:
		
		case PERSE_WIDGET_GROUP_PANEL:
		case PERSE_WIDGET_SCROLL_PANEL:
		
		case PERSE_WIDGET_ARROW_BUTTON:
		case PERSE_WIDGET_TEXT_BUTTON:
		case PERSE_WIDGET_IMAGE_BUTTON:
		case PERSE_WIDGET_COMBO_BOX:
		
		case PERSE_WIDGET_TEXT_BOX:
		
		case PERSE_WIDGET_LABEL:
		
		case PERSE_WIDGET_DATE_PICKER:
		case PERSE_WIDGET_IP_ADDRESS_PICKER:
		
		case PERSE_WIDGET_IMAGE:
		case PERSE_WIDGET_CANVAS:
		
		case PERSE_WIDGET_PROGRESS_BAR:
		case PERSE_WIDGET_PROPERTY_LIST:
		
		case PERSE_WIDGET_TREE_VIEW:
		default:
			if (!widget->system) log("ERROR WIN32:: widg t %i no system??\n", widget->type);
			DestroyWindow(widget->system);
			widget->system = NULL;
			break;
	}
}

PERSE_API void perse_impl_BackendSetProperty(perse_widget_t* widget, perse_property_t* p) {
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
			// TODO: implement
		case PERSE_WIDGET_MENU_BAR:
		case PERSE_WIDGET_STATUS_BAR:
		
		case PERSE_WIDGET_TAB_PANEL: switch (p->name) {
			case PERSE_NAME_TITLE: {
				
				
				const char* title = "perse tab";
				if (p->type != PERSE_TYPE_STRING) {
					log("ERROR WIN32:: TAB_PANEL property TEXT not string");
				} else {
					title = p->string;
					p->changed = 0;
				}
				
				// TODO: assert that parent is group && inited
				perse_widget_t* parent = widget->parent;
				
				TCITEM tie;
				tie.mask = TCIF_TEXT;
				tie.pszText = (char*)title;
				TabCtrl_SetItem(parent->system, index_in_parent(widget), &tie);
				
			} break;
		} break;
		
		
		case PERSE_WIDGET_ITEM: {
			if (!widget->parent) log("ERROR WIN32:: item has no parent");
			
			switch (widget->parent->type) {
			case PERSE_WIDGET_LIST_BOX: switch (p->name) {
				case PERSE_NAME_TITLE: {
					perse_property_t* p = NULL;
				
					const char* title = "list item";
					if (p = prop(PERSE_NAME_TITLE, widget)) {
						if (p->type != PERSE_TYPE_STRING) {
							log("ERROR WIN32:: WIDGET_ITEM property TITLE not string");
						} else {
							title = p->string;
							p->changed = 0;
						}
					}
					
					

					void* listbox = widget->parent->system;
					
					int count = (int)SendMessage(listbox, LB_GETCOUNT, 0, 0);
					if (count == LB_ERR) log("ERROR WIN32:: LB_GETCOUNT return LB_ERR");
					
					int index = -1;
					for (int i = 0; i < count; i++) {
						LPARAM data = SendMessage(listbox, LB_GETITEMDATA, i, 0);
						if (data == (LPARAM)widget) {
							index = i;
							break;
						}
					}
					
					if (index == -1) log("ERROR WIN32:: listbox item title update not found");
					
				
				
				
					char otitle[256];
					SendMessage(listbox, LB_GETTEXT, (WPARAM)index, (LPARAM)otitle);
				
					
					// win32 API doesn't allow changing the text of a listbox
					// item, so we have to re-create it
					LRESULT selected = SendMessage(listbox, LB_GETSEL, (WPARAM)index, 0);

					SendMessage(listbox, LB_DELETESTRING, (WPARAM)index, 0);
					index = SendMessage(listbox,
										LB_INSERTSTRING,
										(WPARAM)index,
										(LPARAM)title);
					SendMessage(listbox, LB_SETITEMDATA, index, (LPARAM)widget);
					
					if (selected > 0) {
						SendMessage(listbox, LB_SETSEL, TRUE, index);
					}
					
					// system needs to be set to non-null
					widget->system = (void*)(long long)index + 1;
				} break;
				
				default:
				
					break;
			}
			break;
			default:
				log("ERROR WIN32:: item parent unsupported type '%i'\n",
					widget->parent->type);
			}
		
			// TODO: do a switch here based on parent type
		
		} break;
		
		case PERSE_WIDGET_GROUP_PANEL:
		case PERSE_WIDGET_SCROLL_PANEL:
			break;
		
		case PERSE_WIDGET_ARROW_BUTTON:
		case PERSE_WIDGET_TEXT_BUTTON:
			if (p->name == PERSE_NAME_TEXT && p->type == PERSE_TYPE_STRING) {
				SetWindowText(widget->system,  p->string);
			}
			break;
		case PERSE_WIDGET_IMAGE_BUTTON:
		case PERSE_WIDGET_COMBO_BOX:
		
		case PERSE_WIDGET_TEXT_BOX: {
			if (p->name == PERSE_NAME_TEXT && p->type == PERSE_TYPE_STRING) {
				char prev_text[256];
				GetWindowText(widget->system, prev_text, sizeof(prev_text));
				
				// prevents event being generated if new text is the same as old
				if (strcmp(prev_text, p->string) != 0) {
					SetWindowText(widget->system, p->string);
				}
			}
		} break;
		
		case PERSE_WIDGET_LABEL:
			if (p->name == PERSE_NAME_TEXT && p->type == PERSE_TYPE_STRING) {
				SetWindowText(widget->system,  p->string);
			}
			break;
		
		case PERSE_WIDGET_DATE_PICKER:
		case PERSE_WIDGET_IP_ADDRESS_PICKER:
		
		case PERSE_WIDGET_IMAGE:
		case PERSE_WIDGET_CANVAS:
		
		case PERSE_WIDGET_PROGRESS_BAR:
		case PERSE_WIDGET_PROPERTY_LIST:
		
		case PERSE_WIDGET_TREE_VIEW:
	}
}

PERSE_API void perse_impl_BackendSetSizePos(perse_widget_t* widget) {
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
			
		case PERSE_WIDGET_ITEM:
		case PERSE_WIDGET_TAB_PANEL:
			// also fake items
			break;
		
		// I think that for these things in here in the win32 we could just have
		// like a single thing? and that single thing would just change for all.
		
		// because in windows all windows are windows
		
		case PERSE_WIDGET_WINDOW:
			log("-- setting window sizepos");
			break;
		case PERSE_WIDGET_MENU_BAR:
		case PERSE_WIDGET_STATUS_BAR:
		
		
		
		case PERSE_WIDGET_GROUP_PANEL:
		case PERSE_WIDGET_SCROLL_PANEL:
		
		case PERSE_WIDGET_LIST_BOX:
		case PERSE_WIDGET_ARROW_BUTTON:
		case PERSE_WIDGET_TEXT_BUTTON:
		case PERSE_WIDGET_TEXT_BOX:
		case PERSE_WIDGET_TAB_GROUP:
			MoveWindow(
				widget->system, 
				widget->actual_pos.x, widget->actual_pos.y,
				widget->current_size.w, widget->current_size.h,
				TRUE
			);
			break;
		case PERSE_WIDGET_IMAGE_BUTTON:
		case PERSE_WIDGET_COMBO_BOX:
		
		case PERSE_WIDGET_DATE_PICKER:
		case PERSE_WIDGET_IP_ADDRESS_PICKER:
		
		case PERSE_WIDGET_IMAGE:
		case PERSE_WIDGET_CANVAS:
		
		case PERSE_WIDGET_PROGRESS_BAR:
		case PERSE_WIDGET_PROPERTY_LIST:
		
		case PERSE_WIDGET_TREE_VIEW:
	}
}

// evil hack..
// TODO: fix
#include "../../library/property.c"
