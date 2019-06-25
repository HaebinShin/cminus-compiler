#ifndef _CODE_H_
#define _CODE_H_

void emitComment(char const *text);

void emitGlobalVariable(char const *name, int size);
void emitFunctionEnter(char const *name);
void emitFunctionExit(void);

void emitBlockEnter(int size);
void emitBlockExit(int size);

#endif
