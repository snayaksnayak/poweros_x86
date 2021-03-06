#include "types.h"
#include "sysbase.h"
#include "arch_config.h"

#include "exec_funcs.h"

__attribute__((no_instrument_function)) UINT32 arch_Int_Disable(void);
__attribute__((no_instrument_function)) UINT32 arch_Int_Enable(void);
__attribute__((no_instrument_function)) void arch_Int_Restore(UINT32 ipl);

void lib_Enable(struct SysBase *SysBase, UINT32 ipl)
{
	SysBase->IDNestCnt--;
	if (SysBase->IDNestCnt < 0) {
		arch_Int_Restore(ipl);
	 }
}

UINT32 lib_Disable(struct SysBase *SysBase)
{
	SysBase->IDNestCnt++;
	if (SysBase->IDNestCnt == 0) return  arch_Int_Disable();
	UINT32 v;
	asm volatile (
		"pushf\n"
		"popl %[v]\n"
		: [v] "=r" (v)
	);
	return v;
}

void lib_Permit(struct SysBase *SysBase)
{
//	lib_Print_uart0("Permit\n");
  	SysBase->TDNestCnt--;
//	lib_Print_uart0("Permit\n");
  	//if (SysBase->IDNestCnt < 0) arch_Enable();
}

void lib_Forbid(struct SysBase *SysBase)
{
//	lib_Print_uart0("Forbid\n");
  	SysBase->TDNestCnt++;
//	lib_Print_uart0("Forbid\n");
  	//if (SysBase->IDNestCnt < 0) arch_Enable();
}

