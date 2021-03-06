#include "cpu/exec/template-start.h"

#define instr movs

make_helper(concat(movs_,SUFFIX))
{
	int IncDec = cpu.DF?-DATA_BYTE:DATA_BYTE;
	DATA_TYPE src = MEM_R(cpu.esi, R_DS);
	MEM_W(cpu.edi, src, R_ES);
	cpu.edi += IncDec;
	cpu.esi += IncDec;

	print_asm("movs"str(SUFFIX)" $ds:($esi),$es:($edi)");
	return 1;
}

#include "cpu/exec/template-end.h"
