#ifndef PERSE_PROPERTY_H
#define PERSE_PROPERTY_H

typedef enum {
	PERSE_TYPE_INVALID = 0,	//< default widget type on allocation
	PERSE_TYPE_INTEGER = 1,	//< same as C int type
	PERSE_TYPE_BOOLEAN = 2,	//< same as C bool type
	PERSE_TYPE_STRING = 3,	//< same as C char* type
} perse_type_t;

typedef enum {
	PERSE_NAME_INVALID = 0
} perse_name_t;

typedef struct perse_property {
	perse_name_t name;
	perse_type_t type;
	
	char changed;
	
	union {
		int integer;
		char boolean;
		char* string;
	};
	
	struct perse_property* next;
} perse_property_t;

perse_property_t* perse_AllocateProperty();
void perse_DestroyProperty(perse_property_t*);

perse_property_t* perse_CreatePropertyInteger(int);
perse_property_t* perse_CreatePropertyBoolean(char);
perse_property_t* perse_CreatePropertyString(const char*);

void perse_CopyPropertyValue(perse_property_t*, perse_property_t*);
int perse_IsPropertyMatching(perse_property_t*, perse_property_t*);

#endif // PERSE_PROPERTY_H