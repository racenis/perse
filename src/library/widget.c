#include "widget.h"

#include "backend.h"

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
	The `user` pointer can be set by the user and will be transfered between
	widgets when merging them. The `destroy` callback is called with the `user`
	pointer when a widget is destroyed and should be used to clean up memory
	that the `user` pointer points to.
	
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
	
	Use the `perse_DestroyWidget()` function to destroy a widget allocated by
	`perse_AllocateWidget()`.
	
	ADDITION OF NEW WIDGET TYPES
	
	Simply add your widget type to `perse_widget_type_t` enum. Consider also
	adding a widget construction function.
	You will also need to add widget to layout calculation code and in the 
	backends, if needed.
	
*/

/// Allocates a new widget.
/// Some default values (layout, etc.) will also be set on the new widget, so
/// that it is ready to be used immediately.
/// Use perse_DestroyWidget() to get rid of unneeded widgets.
/// @return Pointer to new widget.
perse_widget_t* perse_AllocateWidget() {
	perse_widget_t* widget = calloc(1, sizeof(*widget));

	widget->constraint_size.min.w = -1;
	widget->constraint_size.min.h = -1;
	widget->constraint_size.max.w = -1;
	widget->constraint_size.max.h = -1;
	
	widget->changed = 1;
	widget->key = -1;
	
	return widget;
}

/// Destroys a widget.
/// To be used for destroying widgets created by perse_AllocateWidget() or other
/// widget creation functions.
/// All of the properties of the widget and its children will be destroyed as
/// well. If you don't want the children to be destroyed, set their parent to 
/// NULL or some other widget.
void perse_DestroyWidget(perse_widget_t* widget) {
	
	// some widget types need the parent pointer to be intact in order to be
	// properly cleared out of the backend (like the win32 list items)
	if (widget->system) {
		perse_BackendDestroyWidget(widget);
	}
	
	// *now* we can clear out the parent pointer
	if (widget->parent) {
		perse_SetParent(widget, NULL);
	}
	
	for (perse_widget_t* child = widget->child; child;) {
		perse_widget_t* next = child->next;
		// at this point the child list is partially destroyed and so calling
		// perse_SetParent() with a NULL parent will segfault the program, so we
		// make sure that it won't be called
		child->parent = NULL;
		
		perse_DestroyWidget(child);
		child = next;
	}
	
	for (perse_property_t* property = widget->property; property;) {
		perse_property_t* next = property->next;
		perse_DestroyProperty(property);
		property = next;
	}
		
	if (widget->destroy) {
		widget->destroy(widget->user);
	}
	
	memset(widget, 0, sizeof(*widget));
	free(widget);
}

/// Sets the parent of a widget.
/// If a widget already has a parent, the widget will be removed from that
/// parent's children.
/// If `parent` is set to NULL, the widget will become parentless.
void perse_SetParent(perse_widget_t* widget, perse_widget_t* parent) {
	if (widget->parent) {
		// find sibling in parent's list before widget
		perse_widget_t* sibling = widget->parent->child;
		while (sibling && sibling->next != widget) sibling = sibling->next;
		
		// no such sibling
		if (!sibling) {
			if (widget->parent->child != widget) {
				// shouldn't happen
			} else {
				widget->parent->child = widget->next;
			}
		} else if (sibling == widget) {
			widget->parent->child = widget->next;
		} else {
			// splice out the widget
			sibling->next = widget->next;
		}
	}
	
	// insert widget in front of parent's other children 
	if (parent) {
		widget->next = parent->child;
		parent->child = widget;
	}
	
	widget->parent = parent;
}

/// Swaps out a wdiget with a substitute.
/// Replaces a widget with another widget, while retaining the order in the old
/// widget's child widget list.
/// The substitute widget is *not* removed from it's parent, if it has one.
void perse_Substitute(perse_widget_t* widget, perse_widget_t* substitute) {
	if (widget->parent->child == widget) {
		widget->parent->child = substitute;
	} else {
		perse_widget_t* sibling = widget->parent->child;
		while (sibling && sibling->next != widget) sibling = sibling->next;
		
		sibling->next = substitute;
	}
	
	substitute->next = widget->next;
	substitute->parent = widget->parent;
	
	widget->parent = NULL;
	widget->next = NULL;
}

/// Removes a property from a widget.
void perse_AddProperty(perse_widget_t* widget, perse_property_t* property) {
	property->next = widget->property;
	widget->property = property;
}

/// Adds a property to a widget.
void perse_RemoveProperty(perse_widget_t* widget, perse_property_t* property) {
	if (widget->property == property) {
		// Property is first in the list
		widget->property = property->next;
	} else {
		// Find the previous property
		perse_property_t* prev = widget->property;
		while (prev && prev->next != property) prev = prev->next;
		if (prev) {
			prev->next = property->next;
		}
	}
}

/// Adds a child widget to a parent widget.
/// Essentially same as perse_SetParent(), but appends the child to the end of
/// the parent's child list.
void perse_AddChild(perse_widget_t* widget, perse_widget_t* child) {
	if (child->parent) {
		perse_SetParent(child, NULL);
	}
	
	if (!widget->child) {
		widget->child = child;
		child->next = NULL;
		child->parent = widget;
	} else {
		perse_widget_t* last = widget->child;
		while (last->next) last = last->next;
		last->next = child;
		child->next = NULL;
		child->parent = widget;
	}
}