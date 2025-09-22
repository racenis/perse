#ifndef PERSE_WIDGET_H
#define PERSE_WIDGET_H

#include "property.h"

typedef enum {
	PERSE_WIDGET_INVALID = 0,
	
	PERSE_WIDGET_ABSOLUTE_LAYOUT,
	PERSE_WIDGET_HORIZONTAL_LAYOUT,
	PERSE_WIDGET_VERTICAL_LAYOUT,
	PERSE_WIDGET_GRID_LAYOUT,
	PERSE_WIDGET_FLOW_LAYOUT,
	PERSE_WIDGET_SPLITTER_LAYOUT,
	PERSE_WIDGET_FLEX_LAYOUT,
	
	PERSE_WIDGET_MENU_BAR,
	PERSE_WIDGET_STATUS_BAR,
	
	PERSE_WIDGET_TAB_PANEL,
	
	PERSE_GROUP_PANEL,
	PERSE_SCROLL_PANEL,
	
	PERSE_ARROW_BUTTON,
	PERSE_TEXT_BUTTON,
	PERSE_IMAGE_BUTTON,
	PERSE_COMBO_BOX,
	
	PERSE_TEXT_BOX,
	
	PERSE_DATE_PICKER,
	PERSE_IP_ADDRESS_PICKER,
	
	PERSE_IMAGE,
	PERSE_CANVAS,
	
	PERSE_PROGRESS_BAR,
	PERSE_PROPERTY_LIST,
	
	PERSE_TREE_VIEW
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
	void* user;						//< pointer for user to set
	
	int key;						//< optional layout key
	
	perse_range_t constraint_size;	//< min/max size for layout
	perse_range_t want_size;		//< for layout calculation
	perse_size_t current_size;		//< size calculated by layout
	perse_size_t actual_size;		//< size in backend
	
	perse_position_t position;		//< position on screen
	
	char changed;					//< if needs layout recalculation
	
	struct perse_widget* parent;	//< parent widget
	struct perse_widget* child;		//< first child 
	
	perse_property_t* property;		//< list of widget's properties
	struct perse_widget* next;		//< next sibling
} perse_widget_t;

perse_widget_t* perse_AllocateWidget();
void perse_DestroyWidget(perse_widget_t*);

void perse_SetParent(perse_widget_t* widget, perse_widget_t* parent);

void perse_AddProperty(perse_widget_t* widget, perse_property_t* property);
void perse_RemoveProperty(perse_widget_t* widget, perse_property_t* property);

#endif // PERSE_WIDGET_H