#include "globals.h"
#include "code.h"

/* callee saved regs: $ra, $fp */
#define N_CALLEE_SAVED_REGS 2

static enum memory_section {
  NONE_SECTION,
  DATA_SECTION,
  TEXT_SECTION
} current_section = NONE_SECTION;

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
    current_section = DATA_SECTION;
  }

  fprintf(code, "  %s: .space %d\n", name, size);
}

void emitFunctionEnter(char const *name) {
  if(current_section != TEXT_SECTION) {
    if(current_section != NONE_SECTION) fputc('\n', code);

    fprintf(code, ".text\n");
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
  if(cond) fprintf(code, "bne $v0, $zero, %s\n", label);
  else fprintf(code, "beq $v0, $zero, %s\n", label);
}

void emitUncondBranching(char const *label) {
  fprintf(code, "b %s\n", label);
}

void emitLabel(char const *label) {
  fprintf(code, "%s: \n", label);
}
