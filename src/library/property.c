#include <stdlib.h>
#include <string.h>

#include "property.h"

/*
	BASIC EXPLANATION OF PROPERTIES
	
	The `name` of the property determines how the backend will interpret its
	function. The `type` determines what data type is stored in the union in the 
	middle of the struct. Properties are stored in a linked list and `next`
	points to the next element in the list.
	
	
*/

/// Allocates a new property.
/// Use perse_DestroyProperty() to get rid of unneeded widgets.
/// @return Pointer to new property.
perse_property_t* perse_AllocateProperty() {
	return calloc(1, sizeof(perse_property_t));
}

// Removes any attached data.
static void clean_property(perse_property_t* property) {
	switch (property->type) {
		case PERSE_TYPE_STRING:
			free(property->string);
			break;
		case PERSE_TYPE_STRING_ARRAY:
			free(property->string_array);
			break;
		case PERSE_TYPE_CALLBACK_ARRAY:
			free(property->callback_array);
			break;
		case PERSE_TYPE_POINTER_ARRAY:
			free(property->pointer_array);
			break;
		default:
			break;
	}
}

/// Destroys a property.
/// To be used for destroying properties created by perse_AllocateProperty() or
/// other property creation functions.
void perse_DestroyProperty(perse_property_t* property) {
	clean_property(property);
	
	memset(property, 0, sizeof(*property));
	free(property);
}

/// Creates a new integer property.
/// Destroy using perse_DestroyProperty().
/// @param integer Integer to be copied into the property.
/// @return Pointer to new integer property.
perse_property_t* perse_CreatePropertyInteger(int integer) {
	perse_property_t* property = perse_AllocateProperty();
	
	property->type = PERSE_TYPE_INTEGER;
	property->integer = integer;
	
	return property;
}

/// Creates a new boolean property.
/// Destroy using perse_DestroyProperty().
/// @note Boolean value will be copied as either 0 or 1.
/// @param boolean Boolean to be copied into the property.
/// @return Pointer to new boolean property.
perse_property_t* perse_CreatePropertyBoolean(char boolean) {
	perse_property_t* property = perse_AllocateProperty();
	
	property->type = PERSE_TYPE_BOOLEAN;
	property->boolean = boolean ? 1 : 0;
	
	return property;
}

/// Creates a new string property.
/// The string passed in as `string` will be copied.
/// Destroy using perse_DestroyProperty().
/// @param string Null-terminated string to be copied into the property.
/// @return Pointer to new string property.
perse_property_t* perse_CreatePropertyString(const char* string) {
	perse_property_t* property = perse_AllocateProperty();
	
	property->type = PERSE_TYPE_STRING;
	property->string = malloc(strlen(string) + 1);
	
	strcpy(property->string, string);
	
	return property;
}

/// Creates a new callback property.
/// @param cb Callback function pointer to be copied into the new property.
/// @return Pointer to new callback property.
perse_property_t* perse_CreatePropertyCallback(void (*cb)(perse_widget_t*)) {
	perse_property_t* property = perse_AllocateProperty();
	
	property->type = PERSE_TYPE_CALLBACK;
	property->callback = cb;
	
	return property;
}

/// Copies the property value.
/// Copies the property value from `src` into `dst`. The `dst` property is
/// marked as `changed`. Whatever value `dst` contains is destroyed. All void*
/// pointer copies are shallow.
void perse_CopyPropertyValue(perse_property_t* dst, perse_property_t* src) {
	clean_property(dst);
	
	switch (src->type) {
		case PERSE_TYPE_INTEGER:
			dst->integer = src->integer;
			break;
		case PERSE_TYPE_BOOLEAN:
			dst->boolean = src->boolean;
			break;
		case PERSE_TYPE_STRING:
			dst->string = malloc(strlen(src->string) + 1);
			strcpy(dst->string, src->string);
			break;
		case PERSE_TYPE_STRING_ARRAY: {
			int string_count = 0;
			for (char** s = src->string_array; s; s++) string_count++;
			string_count++;
			dst->string_array = calloc(1, sizeof(char*) * string_count);
			for (char** s = src->string_array, **d = dst->string_array; *s;
				s++, d++) {
					*d = malloc(strlen(*s) + 1);
					strcpy(*d, *s);
				}
			} break;
		case PERSE_TYPE_CALLBACK:
			dst->callback = src->callback;
			break;
		case PERSE_TYPE_CALLBACK_ARRAY: {
			int count = 0;
			for (void (**s)(perse_widget_t*) = src->callback_array; *s; s++) count++;
			count++;
			dst->callback_array = calloc(1, sizeof(void (*)(perse_widget_t*)) * count);
			for (void (**s)(perse_widget_t*) = src->callback_array,
				(**d)(perse_widget_t*) = dst->callback_array; *s;
					s++, d++) *d = *s;
			} break;
		case PERSE_TYPE_POINTER:
			dst->callback = src->callback;
			break;
		case PERSE_TYPE_POINTER_ARRAY: {
			int count = 0;
			for (void** s = src->pointer_array; *s; s++) count++;
			count++;
			dst->pointer_array = calloc(1, sizeof(void*) * count);
			for (void** s = src->pointer_array,
				**d = dst->pointer_array; *s;
					s++, d++) *d = *s;
			} break;
		default:
			break;
	}
	
	dst->type = src->type;
	dst->changed = 1;
}

/// Compares the values of two properties.
/// Always returns 0 for void* and void** types (pointer & pointer array), since
/// a proper comparison cannot be performed.
/// @return 1 if matches, 0 if doesn't
int perse_IsPropertyMatching(perse_property_t* p1, perse_property_t* p2) {
	if (p1->type != p2->type) return 0;
	switch (p1->type) {
		case PERSE_TYPE_INVALID:
			return 1;
		case PERSE_TYPE_INTEGER:
			return p1->integer == p2->integer;
		case PERSE_TYPE_BOOLEAN:
			return p1->boolean == p2->boolean;
		case PERSE_TYPE_STRING:
			return strcmp(p1->string, p2->string) == 0;
		case PERSE_TYPE_STRING_ARRAY:
			for (char** str1 = p1->string_array, **str2 = p2->string_array; 
				*str1 && *str2; str1++, str2++) {
					if (strcmp(*str1, *str2) != 0) return 0;
				}
			return 1;
		case PERSE_TYPE_CALLBACK:
			return p1->callback == p2->callback;
		case PERSE_TYPE_CALLBACK_ARRAY:
			for (void (**c1)(perse_widget_t*) = p1->callback_array,
				(**c2)(perse_widget_t*) = p2->callback_array; *c1 && *c2; c1++, c2++) {
					if (c1 != c2) return 0;
				}
			return 1;
		case PERSE_TYPE_POINTER:		// opaque pointer -> cannot compare
			return 0;
		case PERSE_TYPE_POINTER_ARRAY:	// ditto
			return 0;
		default:
			return 0;
	}
}