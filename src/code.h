#ifndef _CODE_H_
#define _CODE_H_

void emitGlobal(char const *name);
void emitGlobalArray(char const *name, int size);
void emitFunctionInit(char const *name);
void emitFunctionTeardown(void);

#endif
