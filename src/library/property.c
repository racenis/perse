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

perse_type_t* perse_AllocateProperty() {
	return calloc(1, sizeof(perse_type_t));
}

void perse_DestroyProperty(perse_type_t* property) {
	// TODO: remove stuff
	
	memset(property, 0, sizeof(property));
	free(property);
}
