#ifndef PERSE_WIDGET_H
#define PERSE_WIDGET_H

#include "property.h"

typedef enum {
	PERSE_WIDGET_INVALID = 0,
	
	PERSE_WIDGET_ABSOLUTE_LAYOUT,
	PERSE_WIDGET_HORIZONTAL_LAYOUT,
	PERSE_WIDGET_VERTICAL_LAYOUT,
	PERSE_WIDGET_GRID_LAYOUT,		// TODO: decide what it should do exactly
	PERSE_WIDGET_FLOW_LAYOUT,		// TODO: ditto
	PERSE_WIDGET_SPLITTER_LAYOUT,	// TODO: ditto
	PERSE_WIDGET_FLEX_LAYOUT,		// TODO: ditto
	
	PERSE_WIDGET_WINDOW,
	PERSE_WIDGET_MENU_BAR,
	PERSE_WIDGET_STATUS_BAR,
	
	PERSE_WIDGET_ITEM,
	
	PERSE_WIDGET_TAB_PANEL,
	PERSE_WIDGET_TAB_GROUP,			// TODO: check if we actually need it
	
	PERSE_WIDGET_PANEL,
	
	PERSE_WIDGET_GROUP_PANEL,
	PERSE_WIDGET_SCROLL_PANEL,
	
	PERSE_WIDGET_ARROW_BUTTON,
	PERSE_WIDGET_TEXT_BUTTON,
	PERSE_WIDGET_IMAGE_BUTTON,
	PERSE_WIDGET_COMBO_BOX,
	
	PERSE_WIDGET_CHECK_BOX,
	PERSE_WIDGET_RADIO_BUTTON,
	PERSE_WIDGET_LIST_BOX,
	
	PERSE_WIDGET_TEXT_BOX,
	PERSE_WIDGET_TEXT_AREA,
	
	PERSE_WIDGET_LABEL,
	
	PERSE_WIDGET_DATE_PICKER,
	PERSE_WIDGET_IP_ADDRESS_PICKER,
	
	PERSE_WIDGET_IMAGE,
	PERSE_WIDGET_CANVAS,
	
	PERSE_WIDGET_PROGRESS_BAR,
	PERSE_WIDGET_PROPERTY_LIST,
	
	PERSE_WIDGET_TREE_VIEW
} perse_widget_type_t;

typedef struct {
	int w, h;
} perse_size_t;

typedef struct {
	int x, y;
} perse_position_t;

typedef struct {
	perse_size_t min, max;
} perse_range_t;

typedef struct perse_widget {
	perse_widget_type_t type;		//< type of the widget
	
	void* system;					//< pointer to win32/motif widget
	void* data;						//< additional pointer for backend
	void* user;						//< pointer for user (frontend) to set
	void(*destroy)(void*);			//< destroy callback
	
	int key;						//< optional layout key
	
	perse_range_t constraint_size;	//< min/max size for layout
	perse_range_t want_size;		//< for layout calculation
	perse_size_t current_size;		//< size calculated by layout
	perse_size_t actual_size;		//< size in backend
	
	perse_position_t position;		//< relative position to parent
	perse_position_t absolute;		//< relative position to window
	perse_position_t actual_pos;	//< actual position of the widget
	
	char changed;					//< if needs layout recalculation
	
	struct perse_widget* parent;	//< parent widget
	struct perse_widget* child;		//< first child 
	
	perse_property_t* property;		//< list of widget's properties
	struct perse_widget* next;		//< next sibling
} perse_widget_t;

perse_widget_t* perse_AllocateWidget();
void perse_DestroyWidget(perse_widget_t*);

void perse_SetParent(perse_widget_t* widget, perse_widget_t* parent);
void perse_Substitute(perse_widget_t* widget, perse_widget_t* substitute);

void perse_AddProperty(perse_widget_t* widget, perse_property_t* property);
void perse_RemoveProperty(perse_widget_t* widget, perse_property_t* property);

void perse_AddChild(perse_widget_t* widget, perse_widget_t* child);


#endif // PERSE_WIDGET_H