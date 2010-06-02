/*
 * ui_app.c
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */
#include "ui_app.h"
#include <string.h>

#define UNUSED(_x) if ((_x) != (_x)) {(_x)=(_x);}

ui_cmd_t get_command()
{
	ui_cmd_t ret;
	char sep;

	memset(ret.command, 0, MAX_CMD_LEN + 1);
	memset(ret.param, 0, FILENAME_MAX);

	printf(PROMPT);
	scanf("%14s",ret.command);

	sep = getc(stdin);
	if (sep == '\n')
		return ret;

	scanf("%s", ret.param);

	return ret;
}

BOOL do_create_process(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;
	int pid;

	if (!APP_DATA_INIT(app_data))
	{
		pid = create_process();

		if (pid < 0)
			to_return = FALSE;
		else
			to_return = TRUE;
	}
	else
		to_return = FALSE;

	return to_return;
}

BOOL do_del_process(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;

	unsigned pid;
	int err;

	if (!APP_DATA_INIT(app_data))
	{
		if (strlen(cmd->param) != 0)
		{
			err = sscanf(cmd->param, " %u ", &pid);
			if (err > 0)
			{
				del_process(pid);
				to_return = TRUE;
			}
			else
			{
				to_return = FALSE;
				printf("Illegal argument");
			}
		}
		else
		{
			to_return = FALSE;
			printf("A proc_id argument is needed");
		}
	}
	else
		to_return = FALSE;

	return to_return;
}

BOOL do_read(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;

	unsigned params[3];

	int err = 1;

	int tok_count = 0;

	char* token;

	if (!APP_DATA_INIT(app_data))
	{
		if (strlen(cmd->param) != 0)
		{
			token = strtok(cmd->param, " ");
			while (err > 0 && token != NULL && tok_count < 3)
			{
				err = sscanf(token, " %u ", &params[tok_count]);

				if (err > 0)
				{
					++tok_count;
					token = strtok(NULL," ");
				}
				else
					to_return = FALSE;
			}
			if (tok_count < 3)
			{
				to_return = FALSE;
				printf("Not enough or bad arguments. usage: read vAddr id amount");
			}

			sim_read(params[0], params[1], 1,params[2], NULL);
			//TODO supposed to use read function, but job description sux
			//implement a function read(vaddr,amount) what about id???
			to_return = TRUE;
		}
		else
		{
			to_return = FALSE;
			printf("Not enough arguments");
		}
	}
	else
		to_return = FALSE;

	return to_return;
	//TODO test
}

BOOL do_loop_read(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;

	unsigned params[4];

	int err = 1;

	int tok_count = 0;

	char* token;

	if (!APP_DATA_INIT(app_data))
	{
		if (strlen(cmd->param) != 0)
		{
			token = strtok(cmd->param, " ");
			while (err > 0 && token != NULL && tok_count < 4)
			{
				err = sscanf(token, " %u ", &params[tok_count]);

				if (err > 0)
				{
					++tok_count;
					token = strtok(NULL," ");
				}
				else
					to_return = FALSE;
			}
			if (tok_count < 4)
			{
				to_return = FALSE;
				printf("Not enough or bad arguments. usage: loopRead vAddr id off amount");
			}

			sim_read(params[0], params[1], params[2], params[3], NULL);
			//TODO supposed to use read function, but job description sux
			//implement a function read(vaddr,amount) what about id???
			to_return = TRUE;
		}
		else
		{
			to_return = FALSE;
			printf("Not enough arguments");
		}
	}
	else
		to_return = FALSE;

	return to_return;
	//TODO implement
}

BOOL do_read_to_file(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;

	unsigned params[3];

	int err = 1;

	int tok_count = 0;

	char* token;

	if (!APP_DATA_INIT(app_data))
	{
		if (strlen(cmd->param) != 0)
		{
			token = strtok(cmd->param, " ");
			while (err > 0 && token != NULL && tok_count < 3) //read numbers first
			{
				err = sscanf(token, " %u ", &params[tok_count]);

				if (err > 0)
				{
					++tok_count;
					token = strtok(NULL," ");
				}
				else
					to_return = FALSE;
			}
			if (tok_count < 4)
			{
				token = strtok(NULL," ");
				if (token != NULL)
				{
					++tok_count;

					sim_read(params[0],params[1], -1, params[2], token);
					//TODO supposed to use read function, but job description sux
					//implement a function read(vaddr,amount) what about id???
					to_return = TRUE;
				}
				else
				{
					to_return = FALSE;
					printf("Not enough or bad arguments. usage: readToFile vAddr id amount filename");
				}
			}
		}
		else
		{
			to_return = FALSE;
			printf("Not enough arguments");
		}
	}
	else
		to_return = FALSE;

	return to_return;
	//TODO implement
}

BOOL do_loop_read_to_file(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;

	unsigned params[4];

	int err = 1;

	int tok_count = 0;

	char* token;

	if (!APP_DATA_INIT(app_data))
	{
		if (strlen(cmd->param) != 0)
		{
			token = strtok(cmd->param, " ");
			while (err > 0 && token != NULL && tok_count < 4) //read numbers first
			{
				err = sscanf(token, " %u ", &params[tok_count]);

				if (err > 0)
				{
					++tok_count;
					token = strtok(NULL," ");
				}
				else
					to_return = FALSE;
			}
			if (tok_count < 5)
			{
				token = strtok(NULL," ");
				if (token != NULL)
				{
					++tok_count;

					sim_read(params[0],params[1],params[2],params[3],token);
					//TODO supposed to use read function, but job description sux
					//implement a function read(vaddr,amount) what about id???
					to_return = TRUE;
				}
				else
				{
					to_return = FALSE;
					printf("Not enough or bad arguments. usage: loopReadToFile vAddr id off amount filename");
				}
			}
		}
		else
		{
			to_return = FALSE;
			printf("Not enough arguments");
		}
	}
	else
		to_return = FALSE;

	return to_return;
	//TODO implement
}

BOOL do_write(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (!APP_DATA_INIT(app_data))
	{
		//print_MM(&APP_DATA_MM(app_data));
		return TRUE;
	}
	else
		return FALSE;
	//TODO implement
}

BOOL do_loop_write(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (!APP_DATA_INIT(app_data))
	{
		//print_MM(&APP_DATA_MM(app_data));
		return TRUE;
	}
	else
		return FALSE;
	//TODO implement
}

BOOL do_hit_rate(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (!APP_DATA_INIT(app_data))
	{
		//print_MM(&APP_DATA_MM(app_data));
		return TRUE;
	}
	else
		return FALSE;
	//TODO implement
}

BOOL do_print_MM(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (!APP_DATA_INIT(app_data))
	{
		print_MM(APP_DATA_MMU(app_data) -> mem);
		return TRUE;
	}
	else
		return FALSE;

}

BOOL do_print_MMU_table(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (!APP_DATA_INIT(app_data))
	{
		print_MMU_table(&(APP_DATA_MMU(app_data) -> mem_ipt));
		return TRUE;
	}
	else
		return FALSE;
}

BOOL do_print_registers(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (!APP_DATA_INIT(app_data))
	{
		//print_MM(&APP_DATA_MM(app_data));
		return TRUE;
	}
	else
		return FALSE;
	//TODO implement
}

BOOL do_print_HAT(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (!APP_DATA_INIT(app_data))
	{
		//print_MM(&APP_DATA_MM(app_data));
		return TRUE;
	}
	else
		return FALSE;
	//TODO implement
}

BOOL do_monitor(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (!APP_DATA_INIT(app_data))
	{
		//print_MM(&APP_DATA_MM(app_data));
		return TRUE;
	}
	else
		return FALSE;
	//TODO implement
}

BOOL do_no_monitor(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (!APP_DATA_INIT(app_data))
	{
		//print_MM(&APP_DATA_MM(app_data));
		return TRUE;
	}
	else
		return FALSE;
	//TODO implement
}

BOOL do_batch_file(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (!APP_DATA_INIT(app_data))
	{
		//print_MM(&APP_DATA_MM(app_data));
		return TRUE;
	}
	else
		return FALSE;
	//TODO implement
}

/**
 * the main function of the application
 */
int app_main(int argc, char **argv)
{
	app_data_t app_data;
	ui_cmd_t cmd;

	//memset(app_data, 0, sizeof(app_data));
	//memset(cmd, 0, sizeof(ui_cmd_t));

	BOOL exit = FALSE;

	APP_DATA_INIT(&app_data) = FALSE;
	//APP_DATA_MM(app_data) = 0; was set by memset above
	//APP_DATA_DISK(app_data) = 0; was set by memset above

	//ipt_init(&(app_data.ipt_table),IPT_SIZE,handlers...); TODO this
	//mm_init(app_data.main_memory...) TODO this
	//disk_init(APP_DATA_DISK(&app_data),DISK_NPAGES,PAGE_SIZE,BLOCK_SIZE);

	//load_app_data

	do
	{
		cmd = get_command();
		if (!strcmp("exit", cmd.command))
		{
			exit = TRUE;
		}
		else if (!strcmp("createProcess", cmd.command))
		{
			do_create_process(&cmd, &app_data);
		}
		else if (!strcmp("delProcess", cmd.command))
		{
			do_del_process(&cmd, &app_data);
		}
		else if (!strcmp("read", cmd.command))
		{
			do_read(&cmd, &app_data);
		}
		else if (!strcmp("loopRead", cmd.command))
		{
			do_loop_read(&cmd ,&app_data);
		}
		else if (!strcmp("readToFile", cmd.command))
		{
			do_read_to_file(&cmd ,&app_data);
		}
		else if (!strcmp("loopReadToFile", cmd.command))
		{
			do_loop_read_to_file(&cmd ,&app_data);
		}
		else if (!strcmp("write", cmd.command))
		{
			do_write(&cmd ,&app_data);
		}
		else if (!strcmp("loopWrite", cmd.command))
		{
			do_loop_write(&cmd ,&app_data);
		}
		else if (!strcmp("hitRate", cmd.command))
		{
			do_hit_rate(&cmd ,&app_data);
		}
		else if (!strcmp("printMM", cmd.command))
		{
			do_print_MM(&cmd ,&app_data);
		}
		else if (!strcmp("printMMUTable", cmd.command))
		{
			do_print_MMU_table(&cmd ,&app_data);
		}
		else if (!strcmp("printRegisters", cmd.command))
		{
			do_print_registers(&cmd ,&app_data);
		}
		else if (!strcmp("printHAT", cmd.command))
		{
			do_print_HAT(&cmd ,&app_data);
		}
		else if (!strcmp("monitor", cmd.command))
		{
			do_monitor(&cmd ,&app_data);
		}
		else if (!strcmp("noMonitor", cmd.command))
		{
			do_no_monitor(&cmd ,&app_data);
		}
		else if (!strcmp("batchFile", cmd.command))
		{
			do_batch_file(&cmd ,&app_data);
		}
		else
		{
			printf ("Wrong input, please type again\n");
		}
	} while (!exit);

	if (app_data.initialized)
	{
		free_app_data(&app_data);
	}

	return 0;
}

//TODO create lock for printing operations? - only UI thread is using printing..
//TODO create a function for checking APP_DATA_INIT instead of the current situation
