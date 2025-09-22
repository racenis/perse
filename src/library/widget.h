#ifndef PERSE_WIDGET_H
#define PERSE_WIDGET_H

#include "property.h"

typedef enum {
	PERSE_WIDGET_INVALID = 0,
	// TODO: list all widgets
	
	// layout widgets
	
	// misc
	
	// inputs
} perse_widget_type_t;

typedef struct {
	int min_w, min_h, max_w, max_h;
} perse_size_t;

typedef struct perse_widget {
	perse_widget_type_t type;		//< type of the widget
	
	void* system;					//< pointer to win32/motif widget
	void* data;						//< additional pointer for backend
	
	int key;						//< optional layout key
	
	perse_size_t constraint_size;	//< min/max size for layout
	perse_size_t want_size;			//< for layout calculation
	perse_size_t current_size;		//< size calculated by layout
	perse_size_t actual_size;		//< size in backend
	
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