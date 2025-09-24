#ifndef PERSE_PERSE_H
#define PERSE_PERSE_H

void perse_Log(const char* fmt, ...);
void perse_SetLogger(void(*fn)(const char* fmt, ...));

#endif // PERSE_PERSE_H