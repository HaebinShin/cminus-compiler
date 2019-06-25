#ifndef _CODE_H_
#define _CODE_H_

enum addressing_mode {
  GET_VALUE,
  GET_ADDRESS
};

void emitInitial(void);

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

void emitPushValue(void);
void emitPopLHS(void);
void emitPopMultiple(int cnt);
void emitGlobalRef(char const *name, enum addressing_mode mode);

/* 
 * relativeOffset
 *    params: 1, 2, 3, ... (left to right)
 *    locals: -2, -3, -4, ... (top to bottom)
 *    saved $ra:   0
 *    saved $fp:   1
 */
void emitLocalRef(int relativeOffset, enum addressing_mode mode);
void emitConstExpr(int value);
void emitCallFunction(char const *funcName);

void emitBinaryOp(int op);
void emitArrayOp(enum addressing_mode mode);

void emitInputSyscall(void);
void emitOutputSyscall(void);

#endif
