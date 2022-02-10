#ifndef TINYRPC_COCTX_H
#define TINYRPC_COCTX_H
namespace tinyrpc{

struct coctx {
  void* regs[14];     // 寄存器数组
};

extern "C" {
extern void coctx_swap(coctx *, coctx *) asm("coctx_swap");
};

}


#endif