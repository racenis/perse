#ifndef PERSE_PROPERTY_H
#define PERSE_PROPERTY_H

typedef enum {
	PERSE_TYPE_INVALID = 0,			//< default widget type on allocation
	PERSE_TYPE_INTEGER = 1,			//< same as C int type
	PERSE_TYPE_BOOLEAN = 2,			//< same as C bool type
	PERSE_TYPE_STRING = 3,			//< same as C char* type
	PERSE_TYPE_STRING_ARRAY	= 4,	//< null-terminated char* array
	PERSE_TYPE_CALLBACK = 5,		//< callback function pointer
	PERSE_TYPE_CALLBACK_ARRAY = 6,	//< null-terminated function pointer array
	PERSE_TYPE_POINTER = 7,			//< same as C void* type
	PERSE_TYPE_POINTER_ARRAY = 8,	//< null-terminated void* array
} perse_type_t;

typedef enum {
	PERSE_NAME_INVALID = 0
} perse_name_t;

typedef struct perse_widget perse_widget_t;

typedef struct perse_property {
	perse_name_t name;
	perse_type_t type;
	
	char changed;
	
	union {
		int integer;
		char boolean;
		char* string;
		char** string_array;
		void (*callback)(perse_widget_t*);
		void (**callback_array)(perse_widget_t*);
		void* pointer;
		void** pointer_array;
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