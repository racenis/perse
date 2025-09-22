#include "layout.h"

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
		
		// default layout -- stack widgets vertically
		default: {
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
		// default layout -- stack widgets vertically
		default: {
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

void perse_CalculateLayout(perse_widget_t* widget) {
	calculate_want(widget);
	calculate_size(widget);
	// 1. recursively calculate want sizes
	// 2. recursively set concrete sizes
	// 3. figure out xy coords
}

void perse_ApplyChanges(perse_widget_t* widget) {
	
}