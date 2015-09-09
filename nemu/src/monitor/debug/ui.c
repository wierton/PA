#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },

	/* TODO: Add more commands */
	{ "si", "Execute the program x steps",cmd_si},
	{ "info", "Print the reg or watch_point",cmd_info},
	{ "x", "Read memory from given address",cmd_x}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

static int cmd_si(char *args)
{
	int steps;
	if(args == NULL)
		steps=1;
	else
		sscanf(args,"%u",&steps);
	cpu_exec(steps);
	return 0;
}

static int cmd_info(char *args)
{

	if(strcmp(args,"r")==0)
	{
		printf("eax\t%04x\t%d\necx\t%04x\t%d\nedx\t%04x\t%d\nebx\t%04x\t%d\nesp\t%04x\t%04x\nebp\t%04x\t%04x\nesi\t%04x\t%d\nedi\t%04x\t%d\neip\t%04x\t%04x\n",cpu.eax,cpu.eax,cpu.ecx,cpu.ecx,cpu.edx,cpu.edx,cpu.ebx,cpu.ebx,cpu.esp,cpu.esp,cpu.ebp,cpu.ebp,cpu.esi,cpu.esi,cpu.edi,cpu.edi,cpu.eip,cpu.eip);
	}
	if(strcmp(args,"w")==0)
	{
/*		WP *p = head;
		while(p != NULL)
		{
			printf("%d\n",p->NO);
			p = p->next;
		}
*/	}
	return 0;
}

static int cmd_x(char *args)
{
	char xar;
	size_t len;
	uint32_t read_addr;
	char *para = args;

	if(*para == 'x' || *para == 'X')
	{
		sscanf(args,"0x%x",&read_addr);
		printf("0x%0x\t0x%0x\n",read_addr,swaddr_read(read_addr,4));
	}
	else if(*para == 's' || *para == 'S')
	{
	}
	else if(*para == 'c' || *para == 'C')
	{
	}
	else if(*para >= '0' && *para <= '9')
	{
		sscanf(para,"%u%c0x%x",&len,&xar,&read_addr);
//		printf("%u :%c: %x",len,xar,read_addr);
		if(xar == 'x' || xar == 'X')
		{}
		else if(xar == 's' || xar == 'S')
		{
			
		}
		else if(xar == 'c' || xar == 'C')
		{}
		else if(xar == ' ' || xar == '\t')
		{
			int i;
			for(i=0;i<len;i++)
				printf("0x%0x\t0x%0x\n",read_addr+i,swaddr_read(read_addr+i,4));
		}
	}
	else
		printf("%s\n","Unkown parameter!");
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
