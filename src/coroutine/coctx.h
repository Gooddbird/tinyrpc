#ifndef TINYRPC_COCTX_H
#define TINYRPC_COCTX_H

namespace tinyrpc{

enum {
  kRBP = 6,   // 栈底指针
  kRDI = 7,   // rdi,第一个参数
  kRSI = 8,   // rsi,第二个参数
  kRETAddr = 9,   // 赋值给rip寄存器,即下一条执行指令的地址
  kRSP = 13,   // 栈顶指针
};


struct coctx {
  void* regs[14];     // 寄存器数组
};

extern "C" {
// 直接调用汇编 coctx_swap 函数

// 切换协程上下文
// 保存当前所有寄存器到第一个coctx上, 将新协程的coctx里面的值取出来赋值给寄存器
extern void coctx_swap(coctx *, coctx *) asm("coctx_swap");
};

}

#endif
