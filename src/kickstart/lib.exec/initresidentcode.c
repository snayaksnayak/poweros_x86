#include "types.h"
#include "resident.h"
#include "sysbase.h"

#include "exec_funcs.h"

void lib_InitResidentCode(struct SysBase * SysBase, UINT32 startClass)
{
	struct ResidentNode *res;
	switch (startClass)
	{
		case RTF_AFTERDOS:
			DPrintF("--------------RTF_AFTERDOS--------------\n");
			break;
		case RTF_COLDSTART:
			DPrintF("-------------RTF_COLDSTART--------------\n");
			break;
		case RTF_SINGLETASK:
			DPrintF("------------RTF_SINGLETASK---------------\n");
			break;
		case RTF_TESTCASE:
			DPrintF("------------RTF_TESTCASE---------------\n");
			break;
	}

	ForeachNode(&SysBase->ResidentList, res)
	{
        if (res->rn_Resident->rt_Flags & startClass)
        {
			DPrintF("InitResidentCode %s \n", res->rn_Resident->rt_Name);
            if (InitResident(res->rn_Resident, NULL)== NULL)
            {
				DPrintF("Module %s will not be loaded\n", res->rn_Resident->rt_Name);
            }
		}
	}
}
