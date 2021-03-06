#include "cpu/exec/helper.h"
#include "monitor/monitor.h"
#include "SDL/SDL.h"

inline void FCC_start();
inline void stop_nemu();
uint32_t swaddr_read(swaddr_t, size_t, uint8_t);

make_helper(cli)
{
	cpu.IF = 0;
	return 1;
}

make_helper(sti)
{
	cpu.IF = 1;
	return 1;
}

make_helper(info_c)
{
	printf("%c", cpu.eax);
	fflush(stdout);
	return 1;
}

make_helper(info_i)
{
	printf("0x%x", cpu.eax);
	fflush(stdout);
	return 1;
}

make_helper(print)
{
	int i;
	char str[201];
	for(i=0;i<200;i++)
	{
		str[i] = swaddr_read(cpu.eax + i, 1, R_DS);
		if(str[i] == 0)
			break;
	}
	str[i] = 0;
	printf("%s", str);
	fflush(stdout);
	return 2;
}

make_helper(inv) {
	/* invalid opcode */

	uint32_t temp[8];
	temp[0] = instr_fetch(eip, 4);
	temp[1] = instr_fetch(eip + 4, 4);

	uint8_t *p = (void *)temp;
	printf("invalid opcode(eip = 0x%08x): %02x %02x %02x %02x %02x %02x %02x %02x ...\n\n", 
			eip, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

	extern char logo [];
	printf("There are two cases which will trigger this unexpected exception:\n\
1. The instruction at eip = 0x%08x is not implemented.\n\
2. Something is implemented incorrectly.\n", eip);
	printf("Find this eip value(0x%08x) in the disassembling result to distinguish which case it is.\n\n", eip);
	printf("\33[1;31mIf it is the first case, see\n%s\nfor more details.\n\nIf it is the second case, remember:\n\
* The machine is always right!\n\
* Every line of untested code is always wrong!\33[0m\n\n", logo);

	assert(0);
}

make_helper(nemu_trap) {
	print_asm("nemu trap (eax = %d)", cpu.eax);
	int i;

	switch(cpu.eax) {
		case 2:
			for(i = 0; i < cpu.edx; i++)
			{
				uint8_t tmp = swaddr_read(cpu.ecx + i, 1, R_DS);
				printf("%c", (char)tmp);
			}
		   	break;
		case 3:
			FCC_start();
			break;
		default:
			printf("\33[1;31mnemu: HIT %s TRAP\33[0m at eip = 0x%08x\n\n",
					(cpu.eax == 0 ? "GOOD" : "BAD"), cpu.eip);
			nemu_state = STOP;
			break;
	}

	return 1;
}

typedef struct {
	uint32_t sw,dw,sp,dp,sx,sy,dx,dy,cw,ch;
} ACC;

typedef struct {
	uint32_t lpBitmapRLE, uiWidth, uiLen, pitch, dx, dy, dw, dh, dp;
} ACC2;

make_helper(nemu_acc)
{
	ACC ac;
	int i, j;
	uint32_t *p = (uint32_t *)(&ac);
	for(i = 0; i < sizeof(ac)/sizeof(uint32_t); i++)
	{
		*(p + i) = swaddr_read(cpu.eax + 4*i, 4, R_DS);
	}
	
//	fputc('|', stdout);
//	fflush(stdout);

	int SrcPos = 0 + ac.sx + ac.sw * (0 + ac.sy);
	int DstPos = 0 + ac.dx + ac.dw * (0 + ac.dy);
	for(j = 0; j < ac.ch; j++)
	{
		for(i = 0; i < ac.cw; i++)
		{
			uint8_t data = swaddr_read(ac.sp + SrcPos, 1, R_DS);
			swaddr_write(ac.dp + DstPos, 1, data, R_DS);
			SrcPos ++;
			DstPos ++;
		}
		SrcPos = SrcPos - ac.cw + ac.sw;
		DstPos = DstPos - ac.cw + ac.dw;
	}

	return 1;
}

make_helper(nemu_acc2)
{
	ACC2 ac;
	int i, j;
	uint32_t *p = (uint32_t *)(&ac);
	for(i = 0; i < sizeof(ac)/sizeof(uint32_t); i++)
	{
		*(p + i) = swaddr_read(cpu.eax + 4*i, 4, R_DS);
	}

	int dx = ac.dx, dy = ac.dy;
	uint32_t uiWidth = ac.uiWidth;
	assert(uiWidth != 0);
	uint32_t lpBitmapRLE = ac.lpBitmapRLE;
	lpBitmapRLE += 4;
	for (i = 0; i < ac.uiLen;)
	{
		uint8_t T = swaddr_read(lpBitmapRLE, 1, R_DS);
		lpBitmapRLE++;
		if ((T & 0x80) && T <= 0x80 + uiWidth)
		{
			i += T - 0x80;
		}
		else
		{
			for (j = 0; j < T; j++)
			{

				int y = (i + j) / uiWidth + dy;
				int x = (i + j) % uiWidth + dx;


				if (x < 0)
				{
					j += -x - 1;
					continue;
				}
				else if (x >= ac.dw)
				{
					j += x - ac.dw;
					continue;
				}

				if (y < 0)
				{
					j += -y * uiWidth - 1;
					continue;
				}
				else if (y >= ac.dh)
				{
					goto end; 
				}

				int dstpos = y * ac.pitch + x;
				uint8_t data = swaddr_read(lpBitmapRLE + j, 1, R_DS);
				swaddr_write(ac.dp + dstpos, 1, data, R_DS);

			}
			lpBitmapRLE += T;
			i += T;
		}
	}

end:
	return 1;
}
