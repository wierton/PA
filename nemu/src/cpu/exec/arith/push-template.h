#include "cpu/exec/template-start.h"

#define instr push

static void do_execute() {
	cpu.eip = op_dest->val;
}

make_instr_helper(i)


#include "cpu/exec/template-end.h"
