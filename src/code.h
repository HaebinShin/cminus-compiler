#ifndef _CODE_H_
#define _CODE_H_

void emitComment(char const *text);
void emitRaw(char const *raw);

void emitGlobalVariable(char const *name, int size);
void emitFunctionEnter(char const *name);
void emitFunctionExit(void);

void emitBlockEnter(int size);
void emitBlockExit(int size);

void emitBranching(char const *label, int cond);
void emitUncondBranching(char const *label);
void emitLabel(char const *label);

#endif
