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
		sizeof(perse_size_t)) != 0) {
		memcpy(&dst->constraint_size, &src->constraint_size,
				sizeof(perse_size_t));
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

void perse_CalculateLayout(perse_widget_t*) {
	
}

void perse_ApplyChanges(perse_widget_t*) {
	
}