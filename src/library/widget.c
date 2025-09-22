#include "widget.h"

#include <stdlib.h>
#include <string.h>

/*
	BASIC EXPLANATION OF WIDGET STRUCTURE
	
	Widgets need to be allocated and destroyed by the appropriate function.
	Currently proxy for malloc()/free(), might change in the future.
	Note that allocation function will also fill out some of the fields with
	default values (like for the sizes).
	
	Widgets are meant to be exist in a tree structure. The root has a NULL 
	`parent` and `child` points to the first child. The children of a widget are
	added in a linked list, for this we use `next` pointer, where each child
	points to its sibling with it.
	The `system` and `data` pointers are reserved for use in the backend. Treat
	them as opaque pointers.
	Properties are stored in a linked list, first element pointed to by
	`properties` pointer.
	To modify parent/child hierarchy and property use the appropriate functions.
	To traverse the hierarchy, use the appropriate functions -- we might change
	the linked list structure to some other structure in the future.
	
	CREATING A NEW WIDGET
	
	You can use `perse_AllocateWidget()` to allocate a new widget structure, but
	it would be better to use any specific widget construction functions, if
	possible.
	
	DESTROYING A WIDGET
	
	
	
	ADDITION OF NEW WIDGET TYPES
	
	Simply add your widget type to `perse_widget_type_t` enum. Consider also
	adding a widget construction function.
*/

/// Allocates a new widget.
/// Some default values (layout, etc.) will also be set on the new widget, so
/// that it is ready to be used immediately.
/// Use perse_DestroyWidget() to get rid of unneeded widgets.
/// @return Pointer to new widget.
perse_widget_t* perse_AllocateWidget() {
	perse_widget_t* widget = calloc(1, sizeof(widget));

	widget->constraint_size.min_w = -1;
	widget->constraint_size.min_h = -1;
	widget->constraint_size.max_w = -1;
	widget->constraint_size.max_h = -1;
	
	return widget;
}

/// Destroys a widget.
/// To be used for destroying widgets created by perse_AllocateWidget() or other
/// widget creation functions.
/// All of the properties of the widget and its children will be destroyed as
/// well. If you don't want the children to be destroyed, set their parent to 
/// NULL or some other widget.
void perse_DestroyWidget(perse_widget_t* widget) {
	// TODO: call into backend to clean up the widget on their side
	
	// TODO: delete all properties
	// TODO: delete all children
	
	memset(widget, 0, sizeof(widget));
	free(widget);
}