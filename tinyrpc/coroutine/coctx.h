#ifndef TINYRPC_COROUTINE_COCTX_H
#define TINYRPC_COROUTINE_COCTX_H 

namespace tinyrpc{

enum {
  kRBP = 6,   // rbp, bottom of stack
  kRDI = 7,   // rdi, first para when call function
  kRSI = 8,   // rsi, second para when call function
  kRETAddr = 9,   // the next excute cmd address, it will be assigned to rip
  kRSP = 13,   // rsp, top of stack
};


struct coctx {
  void* regs[14];
};

extern "C" {
// save current register's state to fitst coctx, and from second coctx take out register's state to assign register
extern void coctx_swap(coctx *, coctx *) asm("coctx_swap");

};

}

#endif
