#ifndef PERSE_CPP_PERSE
#define PERSE_CPP_PERSE

#include "widget.h"

namespace perse {

void Init();

void SetRoot(Widget(*)());

void Render();

bool Wait();

}

#endif // PERSE_CPP_PERSE