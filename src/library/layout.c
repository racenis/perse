#include "layout.h"

#include "perse.h"
#include "backend.h"

#include <stdlib.h>
#include <string.h>

/*
	For now we'll keep the kinda awful switch statement. Later we can refactor
	it, so that it is easier to add new widget types.
	
	LAYOUT ALGORITHM
	
	1. WANT CALCULATION

	We start at the root and recurse until we find leaf widgets. Then we
	remember the minimum size for each widget and come back to the root, adding
	up the minimum sizes, until we calculate the minimum size for the root.
	
	Essentially we're bubbling up the widget constraints to parent widgets.

	2. SIZE CALCULATION

	We start at the root and, recursively, for each child widget we give it a
	size, respecting its or its child's minimum size.

	3. POSITION CALCULATION

	We start at the root and recursively calculate the position of each child
	widget in its parent.
	
*/

/// Merges widget trees.
/// The `dst` tree should be the tree that already has layout calculated for it
/// and changes applied to it, but any two trees should work.
/// The `src` tree will be completely destroyed.
void perse_MergeTree(perse_widget_t* dst, perse_widget_t* src) {
	// assume that types of dst and src are the same
	if (dst->type != src->type) {
		perse_Log("FATAL ERROR: DIFFING TYPE MISMATCH\n");
		abort();
	}
	
	// compare the properties
	perse_property_t* dst_prop = dst->property;
	while (dst_prop) {
		
		// find another property to diff with
		perse_property_t* src_prop = src->property;
		while (src_prop) {
			
			if (src_prop->name != dst_prop->name) {
				src_prop = src_prop->next;
				continue;
			}
			
			if (!perse_IsPropertyMatching(dst_prop, src_prop)) {
				perse_CopyPropertyValue(dst_prop, src_prop);
			}
			
			perse_RemoveProperty(src, src_prop);
			perse_DestroyProperty(src_prop);
			
			goto next_prop;
		}
		
	next_prop:	
		dst_prop = dst_prop->next;
	}
	
	// add all remaining properties as new properties
	perse_property_t* prop = src->property;
	while (prop) {
		perse_property_t* next = prop->next;
		
		perse_RemoveProperty(src, prop);
		perse_AddProperty(dst, prop);
		
		prop->changed = 1;
		
		prop = next;
	}
	
	// compare constraints
	if (memcmp(&dst->constraint_size, &src->constraint_size,
		sizeof(dst->constraint_size)) != 0) {
		memcpy(&dst->constraint_size, &src->constraint_size,
				sizeof(dst->constraint_size));
		dst->changed = 1;
	}
	
	// move user pointer
	if (dst->destroy) {
		dst->destroy(dst->user);
	}
	dst->user = src->user;
	dst->destroy = src->destroy;
	
	src->user = NULL;
	src->destroy = NULL;
	
	// compare children
	perse_widget_t* dst_widg = dst->child;
	while (dst_widg) {
		while (dst_widg->key != -1) {
			abort();
			
			// find other property with same key
			perse_widget_t* src_widg = src->child;
			if (src_widg) {
				if (src_widg->key != dst_widg->key) {
					src_widg = src_widg->next;
					continue;
				}
				
				// merge if type matches
				if (src_widg->type == dst_widg->type) {
					perse_MergeTree(dst_widg, src_widg);
					
					goto next;
				}
				
				// otherwise replace
				perse_widget_t* next = dst_widg->next;
				
				perse_BackendDestroyWidget(dst_widg);
				
				perse_SetParent(dst_widg, NULL);
				perse_SetParent(src_widg, dst);
				
				perse_DestroyWidget(dst_widg);
				
				dst_widg = next;
				
				goto skip;
			}
		}
		
		// didn't find property with same key, try any other widget
		perse_widget_t* src_widg = src->child;
		if (src_widg) {
			// merge if type matches
			if (src_widg->type == dst_widg->type) {
				perse_MergeTree(dst_widg, src_widg);
				
				goto next;
			}
			
			// otherwise replace
			perse_widget_t* next = dst_widg->next;
			
			perse_BackendDestroyWidget(dst_widg);
			
			perse_SetParent(src_widg, NULL);
			perse_Substitute(dst_widg, src_widg);
			
			perse_DestroyWidget(dst_widg);
			
			dst_widg = next;
			
			goto skip;
		}
		
		perse_widget_t* next = dst_widg->next;
		
		perse_DestroyWidget(dst_widg);
		
		dst_widg = next;
		
		continue;
		
	next:
		dst_widg = dst_widg->next;
	skip:
		continue;
	}
	
	// add any remaining new widgets
	perse_widget_t* src_widg = src->child;
	while (src_widg) {
		perse_widget_t* next = src_widg->next;
		perse_AddChild(dst, src_widg);
		src_widg = next;
	}
	
	// src widget is now childless, time to kill it
	perse_DestroyWidget(src);
}

// this function calculates `want` size for widgets. basically for each widget
// we recursively find what are the minimum/maximum sizes for its child widgets
// and add them together
static void calculate_want(perse_widget_t* widget) {
	
	// for leaves we just copy constraint into want
	if (!widget->child) {
		memcpy(&widget->want_size, &widget->constraint_size,
				sizeof(widget->want_size));
		return;
	}
	
	// for other widgets, calculate their want first
	for (perse_widget_t* c = widget->child; c; c = c->next) {
		calculate_want(c);
	}
	
	// otherwise we calculate the want size
	switch (widget->type) {
		case PERSE_WIDGET_HORIZONTAL_LAYOUT: {
			// min height -> largest child min height
			// min width -> sum of child min width
			int largest_min = -1;
			int width_sum = 0;
	
			for (perse_widget_t* c = widget->child; c; c = c->next) {
				if (c->constraint_size.min.h > largest_min) {
					largest_min = c->constraint_size.min.h;
				}
				if (c->constraint_size.min.w > 0) {
					width_sum += c->constraint_size.min.w;
				}
			}
			
			widget->want_size.min.h = largest_min;
			widget->want_size.min.w = width_sum;
			
			widget->want_size.max.h = widget->constraint_size.max.h;
			widget->want_size.max.w = widget->constraint_size.max.w;
			
		} break;
		
		case PERSE_WIDGET_VERTICAL_LAYOUT: {
			// min width -> largest child min width
			// min height -> sum of child min height
			int largest_min = -1;
			int height_sum = 0;
			
			for (perse_widget_t* c = widget->child; c; c = c->next) {
				if (c->constraint_size.min.w > largest_min) {
					largest_min = c->constraint_size.min.w;
				}
				if (c->constraint_size.min.h > 0) {
					height_sum += c->constraint_size.min.h;
				}
			}
			
			widget->want_size.min.w = largest_min;
			widget->want_size.min.h = height_sum;
			
			widget->want_size.max.w = widget->constraint_size.max.w;
			widget->want_size.max.h = widget->constraint_size.max.h;
		} break;
		
		// TODO: implement
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
		
		// child widgets irrelevant; copy contraint into want
		case PERSE_WIDGET_ITEM:
		case PERSE_WIDGET_LIST_BOX:
			memcpy(&widget->want_size, &widget->constraint_size,
				sizeof(widget->want_size));
		break;
		
		// window just stretches its child to be same size as it is
		case PERSE_WIDGET_WINDOW:
			for (perse_widget_t* c = widget->child; c; c = c->next) {
				c->want_size.min.w = widget->current_size.w;
				c->want_size.min.h = widget->current_size.h;
				
				c->want_size.max.w = widget->current_size.w;
				c->want_size.max.h = widget->current_size.h;
			}
		break;
		
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		default: {
			int min_width = -1;
			int min_height = -1;
			
			for (perse_widget_t* c = widget->child; c; c = c->next) {
				if (c->constraint_size.min.w > min_width) {
					min_width = c->constraint_size.min.w;
				}
				if (c->constraint_size.min.h > min_height) {
					min_height = c->constraint_size.min.h;
				}
			}
			
			widget->want_size.min.w = min_width;
			widget->want_size.min.h = min_height;
			
			widget->want_size.max.w = widget->constraint_size.max.w;
			widget->want_size.max.h = widget->constraint_size.max.h;
		}
	}
}

static void calculate_size(perse_widget_t* widget) {
	
	// this will only apply to root
	if (!widget->current_size.w || !widget->current_size.h) {
		widget->current_size.w = widget->constraint_size.min.w;
		widget->current_size.h = widget->constraint_size.min.h;
	}
	
	if (!widget->child) return;
	
	// for each child, calculate their SIZE based on their WANT
	switch (widget->type) {
		case PERSE_WIDGET_HORIZONTAL_LAYOUT: {
			
			// mark all as unprocessed
			for (perse_widget_t* w = widget->child; w; w = w->next) w->current_size.w = -1;
			
			for (;;) {
	
				// divide free space equally among the widgets
				int widgets_left = 0;
				int used_width = 0;
				for (perse_widget_t* w = widget->child; w; w = w->next) {
					if (w->current_size.w == -1) {
						widgets_left++;
					} else {
						used_width += w->current_size.w;
					}
				}
				
				if (!widgets_left) break;
				
				int width_left = widget->current_size.w - used_width;
				int avg_width = width_left / widgets_left;
				
				// check if any constraint violated
				char violated = 0;
				for (perse_widget_t* w = widget->child; w; w = w->next) {
					if (w->current_size.w != -1) {
						continue;
					} else if (w->want_size.max.w != -1 && w->want_size.max.w < avg_width) {
						w->current_size.w = w->want_size.max.w;
						violated = 1;
					} else if (w->want_size.max.w != -1 && w->want_size.min.w > avg_width) {
						w->current_size.w = w->want_size.min.w;
						violated = 1;
					}
				}

				if (violated) continue;
				
				// assign average size to remaining widgets
				for (perse_widget_t* w = widget->child; w; w = w->next) {
					if (w->current_size.w < 0) {
						w->current_size.w = avg_width;
					}
				}
				
				break;
			}
			
			// set the heights
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				if (w->want_size.min.h < widget->current_size.h) {
					w->current_size.h = w->want_size.min.h;
				} else {
					w->current_size.h = widget->current_size.h;
				}
			}
			
		} break;
		
		case PERSE_WIDGET_VERTICAL_LAYOUT: {
			
			// mark all as unprocessed
			for (perse_widget_t* w = widget->child; w; w = w->next) w->current_size.h = -1;
			
			for (;;) {
	
				// divide free space equally among the widgets
				int widgets_left = 0;
				int used_height = 0;
				for (perse_widget_t* w = widget->child; w; w = w->next) {
					if (w->current_size.h == -1) {
						widgets_left++;
					} else {
						used_height += w->current_size.h;
					}
				}
				
				if (!widgets_left) break;
				
				int height_left = widget->current_size.h - used_height;
				int avg_height = height_left / widgets_left;
				
				// check if any constraint violated
				char violated = 0;
				for (perse_widget_t* w = widget->child; w; w = w->next) {
					if (w->current_size.h != -1) {
						continue;
					} else if (w->want_size.max.h != -1 && w->want_size.max.h < avg_height) {
						w->current_size.h = w->want_size.max.h;
						violated = 1;
					} else if (w->want_size.max.h != -1 && w->want_size.min.h > avg_height) {
						w->current_size.h = w->want_size.min.h;
						violated = 1;
					}
				}

				if (violated) continue;
				
				// assign average size to remaining widgets
				for (perse_widget_t* w = widget->child; w; w = w->next) {
					if (w->current_size.h < 0) {
						w->current_size.h = avg_height;
					}
				}
				
				break;
			}
			
			// set the widths
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				if (w->want_size.min.w < widget->current_size.w) {
					w->current_size.w = w->want_size.min.w;
				} else {
					w->current_size.w = widget->current_size.w;
				}
			}
		} break;
		
		// TODO: implement
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
		
		// do not process; child layout irrelevant
		case PERSE_WIDGET_ITEM:
		case PERSE_WIDGET_LIST_BOX:
		case PERSE_WIDGET_TEXT_BOX:
		break;
		
		// idk, this might cause issues if more than one child for window
		case PERSE_WIDGET_WINDOW:
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				w->current_size.w = widget->current_size.w;
				w->current_size.h = widget->current_size.h;
			}
			break;
		
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		default: {
			const int default_size = 32;
			
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				if (w->constraint_size.min.w > 0) {
					w->current_size.w = w->constraint_size.min.w;
				} else if (w->constraint_size.max.w > 0) {
					w->current_size.w = w->constraint_size.max.w;
				} else {
					w->current_size.w = default_size;
				}
				
				if (w->constraint_size.min.h > 0) {
					w->current_size.h = w->constraint_size.min.h;
				} else if (w->constraint_size.max.h > 0) {
					w->current_size.h = w->constraint_size.max.h;
				} else {
					w->current_size.h = default_size;
				}
			}
		}
	}
	
	for (perse_widget_t* w = widget->child; w; w = w->next) {
		calculate_size(w);
	}
}

static void calculate_position(perse_widget_t* widget) {
	
	// child position is determined by their parent, if reached leaf, return
	if (!widget->child) return;

	// for each child, calculate their SIZE based on their WANT
	switch (widget->type) {
		case PERSE_WIDGET_HORIZONTAL_LAYOUT: {
			int current_w = 0;
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				int offset = widget->current_size.h - w->current_size.h;

				if (offset != 0 && offset/2 > 0) {
					w->position.y = offset/2;
				}
				
				w->position.x = current_w;
				current_w += w->current_size.w;
			}
		} break;
		
		case PERSE_WIDGET_VERTICAL_LAYOUT: {
			int current_h = 0;
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				int offset = widget->current_size.w - w->current_size.w;

				if (offset != 0 && offset/2 > 0) {
					w->position.x = offset/2;
				}
				
				w->position.y = current_h;
				current_h += w->current_size.h;
			}
		} break;
		
		// TODO: implement
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
		
		// do not process; child layout irrelevant
		case PERSE_WIDGET_ITEM:
		case PERSE_WIDGET_LIST_BOX:
		case PERSE_WIDGET_TEXT_BOX:
		return;
		
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		default:
			// absolute layout - position remains as was set
			break;
	}
	
	// then calculate absolute position
	switch (widget->type) {
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		case PERSE_WIDGET_HORIZONTAL_LAYOUT:
		case PERSE_WIDGET_VERTICAL_LAYOUT:
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				w->absolute.x = widget->absolute.x + w->position.x;
				w->absolute.y = widget->absolute.y + w->position.y;
			}
			break;
		default:
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				w->absolute.x = w->position.x;
				w->absolute.y = w->position.y;
			}	
	}
	
	for (perse_widget_t* w = widget->child; w; w = w->next) {
		calculate_position(w);
	}
}

/// Calculates widget layout.
/// Calculates the layout of widget and its child widgets.
void perse_CalculateLayout(perse_widget_t* widget) {
	calculate_want(widget);
	calculate_size(widget);
	calculate_position(widget);
}

static void apply_changes(perse_widget_t* widget, char recalc_pos) {
	char update_size = 0;
	
	if (widget->actual_size.w != widget->current_size.w) {
		widget->actual_size.w = widget->current_size.w;
		recalc_pos = 1;
	}
	
	if (widget->actual_size.h != widget->current_size.h) {
		widget->actual_size.h = widget->current_size.h;
		recalc_pos = 1;
	}
	
	
	if (widget->actual_pos.x != widget->absolute.x) {
		widget->actual_pos.x = widget->absolute.x;
		recalc_pos = 1;
	}
	
	if (widget->actual_pos.y != widget->absolute.y) {
		widget->actual_pos.y = widget->absolute.y;
		recalc_pos = 1;
	}
	
	if (!widget->system) {
		perse_BackendCreateWidget(widget);
	} else if (recalc_pos) {
		perse_BackendSetSizePos(widget);
	}
	
	for (perse_property_t* p = widget->property; p; p = p->next) {
		if (!p->changed) continue;
		perse_BackendSetProperty(widget, p);
		p->changed = 0;
	}
	
	for (perse_widget_t* w = widget->child; w; w = w->next) {
		apply_changes(w, recalc_pos);
	}
}

/// Applies changes.
/// Forwards the changes created by perse_MergeTree() and 
/// perse_CalculateLayout() to backend.
void perse_ApplyChanges(perse_widget_t* widget) {
	apply_changes(widget, 0);
}