#ifndef PERSE_PROPERTY_H
#define PERSE_PROPERTY_H

typedef enum {
	PERSE_TYPE_INVALID = 0,	//< default widget type on allocation
	PERSE_TYPE_INTEGER = 1,	//< same as C int type
	PERSE_TYPE_BOOLEAN = 2,	//< same as C bool type
	PERSE_TYPE_STRING = 3,	//< same as C char* type
} perse_type_t;

typedef enum {
	PERSE_NAME_
} perse_name_t;

typedef struct perse_property {
	perse_name_t name;
	perse_type_t type;
	
	union {
		int integer;
		int boolean;
		char* string;
	};
	
	struct perse_property* next;
} perse_property_t;

perse_type_t* perse_AllocateProperty();
void perse_DestroyProperty(perse_type_t*);

perse_type_t* perse_CreatePropertyInteger(int);
perse_type_t* perse_CreatePropertyBoolean(int);
perse_type_t* perse_CreatePropertyString(const char*);

#endif // PERSE_PROPERTY_H