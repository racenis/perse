#include "property.h"

#include <stdlib.h>
#include <string.h>

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
		default:
			break;
	}
}

/// Destroys a property.
/// To be used for destroying properties created by perse_AllocateProperty() or
/// other property creation functions.
void perse_DestroyProperty(perse_property_t* property) {
	clean_property(property);
	
	memset(property, 0, sizeof(property));
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
		default:
			break;
	}
	
	dst->type = src->type;
	dst->changed = 1;
}

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
		default:
			return 0;
	}
}