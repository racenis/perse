#include "layout.h"

#include "backend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// Merges widget trees.
/// The `dst` tree should be the tree that already has layout calculated for it
/// and changes applied to it, but any two trees should work.
/// The `src` tree will be completely destroyed.
void perse_MergeTree(perse_widget_t* dst, perse_widget_t* src) {
	// assume that types of dst and src are the same
	if (dst->type != src->type) {
		printf("FATAL ERROR: DIFFING TYPE MISMATCH\n");
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
		
		next_prop:;
	}
	
	// add all remaining properties as new properties
	perse_property_t* prop = dst->property;
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
	
	// compare children
	perse_widget_t* dst_widg = dst->child;
	while (dst_widg) {
		if (dst_widg->key != -1) {
			// find other property with same key
			perse_widget_t* src_widg = src->child;
			while (src_widg) {
				if (src_widg->key != dst_widg->key) {
					src_widg = src_widg->next;
					continue;
				}
				
				// merge if type matches
				if (src_widg->type == dst_widg->type) {
					perse_MergeTree(dst_widg, src_widg);
					
					perse_SetParent(src_widg, NULL);
					perse_DestroyWidget(src_widg);
					
					goto next;
				}
				
				// otherwise replace
				perse_widget_t* next = dst_widg->next;
				
				perse_SetParent(dst_widg, NULL);
				perse_SetParent(src_widg, dst);
				
				perse_DestroyWidget(dst_widg);
				
				dst_widg = next;
				
				goto skip;
			}
		}
		
		// didn't find property with same key, try any other widget
		perse_widget_t* src_widg = src->child;
		while (src_widg) {
			// merge if type matches
			if (src_widg->type == dst_widg->type) {
				perse_MergeTree(dst_widg, src_widg);
				
				perse_SetParent(src_widg, NULL);
				perse_DestroyWidget(src_widg);
				
				goto next;
			}
			
			// otherwise replace
			perse_widget_t* next = dst_widg->next;
			
			perse_SetParent(dst_widg, NULL);
			perse_SetParent(src_widg, dst);
			
			perse_DestroyWidget(dst_widg);
			
			dst_widg = next;
			
			goto skip;
		}
	next:
		dst_widg = dst_widg->next;
	skip:
		continue;
	}
	
	// add any remaining new widgets
	perse_widget_t* src_widg = src->child;
	while (src_widg) {
		perse_SetParent(src_widg, dst);
	}
	
	// src widget is now childless, time to kill it
	perse_DestroyWidget(src_widg);
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
			// min width -> largest child min width
			// min height -> sum of child min height
			int largest_min = -1;
			int height_sum = 0;
			
			// TODO: switch to horizontal layout
			
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
		
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		default: {
			// min width -> largest child min width
			// min height -> sum of child min height
			int largest_min = -1;
			int height_sum = 0;
			
			// TODO: convert to ??
			// just do both min of x and y!!
			
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
			
			
			// TODO: switch this to doing stuff horizontally
			
			
			
			// first we'll try to divide the height equally
			int widgets = 0;
			for (perse_widget_t* w = widget->child; w; w = w->next) widgets++;
			int average_size = widget->current_size.h / widgets;
			
			// then we'll see if any widget doesn't like that height
			int used_height = 0;
			int widgets_left = 0;
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				if (w->want_size.max.h < average_size) {
					w->current_size.h = w->want_size.max.h;
					used_height += w->want_size.max.h;
				} else if (w->want_size.min.h > average_size) {
					w->current_size.h = w->want_size.min.h;
					used_height += w->want_size.max.h;
				} else {
					w->current_size.h = -1;
					widgets_left++;
				}
			}
			
			// then we'll re-calculate the equal height again
			if (widgets_left) average_size = used_height/widgets_left;
			
			// what if average_size violates any child's constraint?
			// idk, maybe try a greedy algorithm, idk
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				if (w->current_size.h != -1) continue;
				w->current_size.h = average_size;
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
		
		case PERSE_WIDGET_VERTICAL_LAYOUT: {
			// first we'll try to divide the height equally
			int widgets = 0;
			for (perse_widget_t* w = widget->child; w; w = w->next) widgets++;
			int average_size = widget->current_size.h / widgets;
			
			// then we'll see if any widget doesn't like that height
			int used_height = 0;
			int widgets_left = 0;
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				if (w->want_size.max.h < average_size) {
					w->current_size.h = w->want_size.max.h;
					used_height += w->want_size.max.h;
				} else if (w->want_size.min.h > average_size) {
					w->current_size.h = w->want_size.min.h;
					used_height += w->want_size.max.h;
				} else {
					w->current_size.h = -1;
					widgets_left++;
				}
			}
			
			// then we'll re-calculate the equal height again
			if (widgets_left) average_size = used_height/widgets_left;
			
			// what if average_size violates any child's constraint?
			// idk, maybe try a greedy algorithm, idk
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				if (w->current_size.h != -1) continue;
				w->current_size.h = average_size;
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
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
		
		case PERSE_WIDGET_ABSOLUTE_LAYOUT:
		default: {
			
			// TODO: switch this to .. hmm..
			// for each widget
			// - if has min size, set that
			// - if does not have, set max
			// - otherwise set some random value
			
			
			// first we'll try to divide the height equally
			int widgets = 0;
			for (perse_widget_t* w = widget->child; w; w = w->next) widgets++;
			int average_size = widget->current_size.h / widgets;
			
			// then we'll see if any widget doesn't like that height
			int used_height = 0;
			int widgets_left = 0;
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				if (w->want_size.max.h < average_size) {
					w->current_size.h = w->want_size.max.h;
					used_height += w->want_size.max.h;
				} else if (w->want_size.min.h > average_size) {
					w->current_size.h = w->want_size.min.h;
					used_height += w->want_size.max.h;
				} else {
					w->current_size.h = -1;
					widgets_left++;
				}
			}
			
			// then we'll re-calculate the equal height again
			if (widgets_left) average_size = used_height/widgets_left;
			
			// what if average_size violates any child's constraint?
			// idk, maybe try a greedy algorithm, idk
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				if (w->current_size.h != -1) continue;
				w->current_size.h = average_size;
			}
			
			// set the widths
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				if (w->want_size.min.w < widget->current_size.w) {
					w->current_size.w = w->want_size.min.w;
				} else {
					w->current_size.w = widget->current_size.w;
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
			// TODO: switch to horizontal
			int current_h = 0;
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				int offset = widget->current_size.w - w->constraint_size.min.w;

				if (offset != 0 && offset/2 > 0) {
					widget->position.x = offset/2;
				}
				
				w->position.y = current_h;
				current_h += w->current_size.h;
			}
		}
		case PERSE_WIDGET_VERTICAL_LAYOUT: {
			int current_h = 0;
			for (perse_widget_t* w = widget->child; w; w = w->next) {
				int offset = widget->current_size.w - w->constraint_size.min.w;

				if (offset != 0 && offset/2 > 0) {
					widget->position.x = offset/2;
				}
				
				w->position.y = current_h;
				current_h += w->current_size.h;
			}
		}
		case PERSE_WIDGET_GRID_LAYOUT:
		case PERSE_WIDGET_FLOW_LAYOUT:
		case PERSE_WIDGET_SPLITTER_LAYOUT:
		case PERSE_WIDGET_FLEX_LAYOUT:
		
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
}

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
}

void perse_ApplyChanges(perse_widget_t* widget) {
	apply_changes(widget, 0);
}