#include "globals.h"
#include "code.h"

/* callee saved regs: $ra, $fp */
#define N_CALLEE_SAVED_REGS 2

static enum memory_section {
  NONE_SECTION,
  DATA_SECTION,
  TEXT_SECTION
} current_section = NONE_SECTION;

void emitInitial(void) {
  fputs(".globl\tmain\n", code);
  fprintf(code, ".align 4\n");
  fprintf(code, ".data\n");
  fprintf(code, "newline:\t.asciiz\t\"\\n\"\n");
  fprintf(code, "output_text:\t.asciiz\t\"Output : \"\n");
  fprintf(code, "input_text:\t.asciiz\t\"Input : \"\n");
  fprintf(code, "\n");
}

void emitComment(char const *text) {
  fprintf(code, "# %s\n", text);
}

void emitRaw(char const *raw) {
  fputs(raw, code);
}

void emitGlobalVariable(char const *name, int size) {
  if(current_section != DATA_SECTION) {
    if(current_section != NONE_SECTION) fputc('\n', code);

    fprintf(code, ".data\n");
    fprintf(code, ".align 4\n");
    current_section = DATA_SECTION;
  }

  fprintf(code, "  _%s: .space %d\n", name, size);
}

void emitFunctionEnter(char const *name) {
  if(current_section != TEXT_SECTION) {
    if(current_section != NONE_SECTION) fputc('\n', code);

    fprintf(code, ".text\n");
    fprintf(code, ".align 4\n");
    current_section = TEXT_SECTION;
  }

  int upperLimit = 0;

  emitComment("function enter");
  fprintf(code, "%s:\n", name);
  fprintf(code, "  subu\t$sp,\t$sp,\t%d\n", N_CALLEE_SAVED_REGS * 4);
  upperLimit += N_CALLEE_SAVED_REGS * 4;

  fprintf(code, "  sw\t$ra,\t%d($sp)\n", upperLimit - 4*1);
  fprintf(code, "  sw\t$fp,\t%d($sp)\n", upperLimit - 4*2);

  /* push registers here */

  fprintf(code, "  addu\t$fp,\t$sp,\t%d\n", upperLimit - 4*1);

  fputc('\n', code);
}

void emitFunctionExit(void) {
  emitComment("function exit");
  fprintf(code, "  subu\t$sp,\t$fp,\t%d\n", N_CALLEE_SAVED_REGS * 4 - 4);

  /* pop registers here */

  fprintf(code, "  lw\t$fp,\t($sp)\n");
  fprintf(code, "  addu\t$sp\t$sp,\t4\n");
  fprintf(code, "  lw\t$ra,\t($sp)\n");
  fprintf(code, "  addu\t$sp\t$sp,\t4\n");

  fprintf(code, "  jr\t$ra\n");

  fputc('\n', code);
}

void emitBlockEnter(int size) {
  if(size == 0) return;
  fprintf(code, "  subu\t$sp,\t$sp,\t%d\n", size);
}
void emitBlockExit(int size) {
  if(size == 0) return;
  fprintf(code, "  addu\t$sp,\t$sp,\t%d\n", size);
}

void emitBranching(char const *label, int cond) {
  if(cond) fprintf(code, "  bne\t$v0,\t$zero,\t%s\n", label);
  else fprintf(code, "  beq\t$v0,\t$zero,\t%s\n", label);
}

void emitUncondBranching(char const *label) {
  fprintf(code, "  b\t%s\n", label);
}

void emitLabel(char const *label) {
  fprintf(code, "%s: \n", label);
}

void emitPushValue(void) {
  fprintf(code, "  subu\t$sp,\t$sp,\t4\n");
  fprintf(code, "  sw\t$v0,\t($sp)\n");
}

void emitPopLHS(void) {
  fprintf(code, "  lw\t$t0,\t($sp)\n");
  fprintf(code, "  addu\t$sp,\t$sp,\t4\n");
}

void emitPopMultiple(int cnt) {
  if(cnt == 0) return;
  fprintf(code, "  addu\t$sp,\t$sp,\t%d\n", cnt * 4);
}

void emitGlobalRef(char const *name, enum addressing_mode mode) {
  if(mode == GET_VALUE) fprintf(code, "  lw\t$v0,\t_%s\n", name);
  else fprintf(code, "  la\t$v0,\t_%s\n", name);
}

void emitLocalRef(int relativeOffset, enum addressing_mode mode) {
  int offset;
  if(relativeOffset >= 1) {
    offset = relativeOffset * 4;
  }
  else {
    offset = - (N_CALLEE_SAVED_REGS * 4 - 4) + (relativeOffset + 1) * 4;
  }
    
  if(mode == GET_VALUE) {
    fprintf(code, "  lw\t$v0,\t%d($fp)\n", offset);
  }
  else {
    fprintf(code, "  addu\t$v0,\t$fp,\t%d\n", offset);
  }
}

void emitConstExpr(int value) {
  fprintf(code, "  li\t$v0,\t%d\n", value);
}

void emitCallFunction(char const *funcName) {
  fprintf(code, "  jal\t%s\n", funcName);
}

void emitBinaryOp(int op) {
  switch(op) {
  case ASSIGN:
    fprintf(code, "  sw\t$v0,\t($t0)\n");
    break;
  case LE:
    fprintf(code, "  sle\t$v0,\t$t0,\t$v0\n");
    break;
  case LT:
    fprintf(code, "  slt\t$v0,\t$t0,\t$v0\n");
    break;
  case GE:
    fprintf(code, "  sge\t$v0,\t$t0,\t$v0\n");
    break;
  case GT:
    fprintf(code, "  sgt\t$v0,\t$t0,\t$v0\n");
    break;
  case EQ:
    fprintf(code, "  seq\t$v0,\t$t0,\t$v0\n");
    break;
  case NE:
    fprintf(code, "  sne\t$v0,\t$t0,\t$v0\n");
    break;
  case PLUS:
    fprintf(code, "  add\t$v0,\t$t0,\t$v0\n");
    break;
  case MINUS:
    fprintf(code, "  sub\t$v0,\t$t0,\t$v0\n");
    break;
  case STAR:
    fprintf(code, "  mul\t$v0,\t$t0,\t$v0\n");
    break;
  case SLASH:
    fprintf(code, "  div\t$v0,\t$t0,\t$v0\n");
    break;
  default:
    assert(!"unreachable code");
    break;
  }
}

void emitArrayOp(enum addressing_mode mode) {
  fprintf(code, "  li\t$t1,\t4\n");
  fprintf(code, "  mul\t$v0,\t$v0,\t$t1\n");
  fprintf(code, "  add\t$v0,\t$t0,\t$v0\n"); 
  if(mode == GET_VALUE) {
    fprintf(code, "  lw $v0, ($v0)\n");
  }
}

void emitInputSyscall(void) {
  emitComment("\n");
  emitComment("**** Input Syscall ****");
  emitComment("\n");

  /* print text */
  fprintf(code, "  li\t$v0,\t4\n");
  fprintf(code, "  la\t$a0,\tinput_text\n");
  fprintf(code, "  syscall\n");

  /* get value */
  fprintf(code, "  li\t$v0,\t5\n");
  fprintf(code, "  syscall\n");

  emitComment("\n");
  emitComment("***********************");
  emitComment("\n");
}

void emitOutputSyscall(void) {
  emitComment("\n");
  emitComment("**** Output Syscall ****");
  emitComment("\n");

  /* store value*/
  fprintf(code, "  move\t$t0,\t$v0\n");

  /* print text */
  fprintf(code, "  li\t$v0,\t4\n");
  fprintf(code, "  la\t$a0,\toutput_text\n");
  fprintf(code, "  syscall\n");

  /* print int */
  fprintf(code, "  move\t$a0,\t$t0\n");
  fprintf(code, "  li\t$v0,\t1\n");
  fprintf(code, "  syscall\n");

  /* newline */
  fprintf(code, "  li\t$v0,\t4\n");
  fprintf(code, "  la\t$a0,\tnewline\n");
  fprintf(code, "  syscall\n");

  emitComment("\n");
  emitComment("**********************");
  emitComment("\n");
}
