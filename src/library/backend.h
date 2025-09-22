#ifndef PERSE_BACKEND_H
#define PERSE_BACKEND_H

#include "widget.h"

extern void (*perse_BackendCreateWidget)(perse_widget_t*);
extern void (*perse_BackendDestroyWidget)(perse_widget_t*);

extern void (*perse_BackendSetProperty)(perse_widget_t*, perse_property_t*);
extern void (*perse_BackendSetSizePos)(perse_widget_t*);

void perse_LoadBackend();

#endif // PERSE_BACKEND_H