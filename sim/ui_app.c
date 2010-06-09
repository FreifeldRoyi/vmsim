/*
 * ui_app.c
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */
#include "ui_app.h"
#include "util/logger.h"
#include <string.h>
#include <assert.h>

static BOOL command_handler(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = TRUE;

	if (!strcmp("exit", cmd -> command))
	{
		to_return = FALSE;
	}
	else if (!strcmp("createProcess", cmd -> command))
	{
		do_create_process(cmd, app_data);
	}
	else if (!strcmp("delProcess", cmd -> command))
	{
		do_del_process(cmd, app_data);
	}
	else if (!strcmp("read", cmd -> command))
	{
		do_read(cmd, app_data);
	}
	else if (!strcmp("loopRead", cmd -> command))
	{
		do_loop_read(cmd ,app_data);
	}
	else if (!strcmp("readToFile", cmd -> command))
	{
		do_read_to_file(cmd ,app_data);
	}
	else if (!strcmp("loopReadToFile", cmd -> command))
	{
		do_loop_read_to_file(cmd ,app_data);
	}
	else if (!strcmp("write", cmd -> command))
	{
		do_write(cmd ,app_data);
	}
	else if (!strcmp("loopWrite", cmd -> command))
	{
		do_loop_write(cmd ,app_data);
	}
	else if (!strcmp("hitRate", cmd -> command))
	{
		do_hit_rate(cmd ,app_data);
	}
	else if (!strcmp("printMM", cmd -> command))
	{
		do_print_MM(cmd ,app_data);
	}
	else if (!strcmp("printMMUTable", cmd -> command))
	{
		do_print_MMU_table(cmd ,app_data);
	}
	else if (!strcmp("printRegisters", cmd -> command))
	{
		do_print_registers(cmd ,app_data);
	}
	else if (!strcmp("printHAT", cmd -> command))
	{
		do_print_HAT(cmd ,app_data);
	}
	else if (!strcmp("monitor", cmd -> command))
	{
		do_monitor(cmd ,app_data);
	}
	else if (!strcmp("noMonitor", cmd -> command))
	{
		do_no_monitor(cmd ,app_data);
	}
	else if (!strcmp("debug", cmd -> command))
	{
		do_debug_mode(cmd, app_data);
	}
	else if (!strcmp("batchFile", cmd -> command))
	{
		to_return = do_batch_file(cmd ,app_data);
	}
	else
	{
		printf ("Wrong input, please type again\n");
	}

	return to_return;
}

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

	errcode_t err;

	if (APP_DATA_INIT(app_data))
	{
		pid = create_process(app_data);

		if (pid < 0)
			to_return = FALSE;
		else
		{
			to_return = TRUE;
			err = process_start(APP_DATA_PROC_CONT(app_data), pid);
		}
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

	if (APP_DATA_INIT(app_data))
	{
		if (strlen(cmd->param) != 0)
		{
			err = sscanf(cmd->param, " %u ", &pid);
			if (err > 0)
			{
				del_process(app_data, pid);
				to_return = TRUE;
			}
			else
			{
				to_return = FALSE;
				printf("Illegal argument\n");
			}
		}
		else
		{
			to_return = FALSE;
			printf("A proc_id argument is needed\n");
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

	if (APP_DATA_INIT(app_data))
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
				printf("Not enough or bad arguments. usage: read vAddr id amount\n");
			}
			else
			{
				read_process(APP_DATA_PROC_CONT(app_data), params[0], params[1], params[2]);
				to_return = TRUE;
			}
		}
		else
		{
			to_return = FALSE;
			printf("Not enough arguments\n");
		}
	}
	else
		to_return = FALSE;

	return to_return;
}

BOOL do_loop_read(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;

	unsigned params[4];

	int err = 1;

	int tok_count = 0;

	char* token;

	if (APP_DATA_INIT(app_data))
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
				printf("Not enough or bad arguments. usage: loopRead vAddr id off amount\n");
			}
			else
			{
				loop_read_process(APP_DATA_PROC_CONT(app_data), params[0], params[1], params[2], params[3]);
				to_return = TRUE;
			}
		}
		else
		{
			to_return = FALSE;
			printf("Not enough arguments\n");
		}
	}
	else
		to_return = FALSE;

	return to_return;
}

BOOL do_read_to_file(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;

	unsigned params[3];

	int err = 1;

	int tok_count = 0;

	char* token;

	if (APP_DATA_INIT(app_data))
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

					read_to_file_process(APP_DATA_PROC_CONT(app_data), params[0], params[1], params[2], token);
					to_return = TRUE;
				}
				else
				{
					to_return = FALSE;
					printf("Not enough or bad arguments. usage: readToFile vAddr id amount filename\n");
				}
			}
		}
		else
		{
			to_return = FALSE;
			printf("Not enough arguments\n");
		}
	}
	else
		to_return = FALSE;

	return to_return;
}

BOOL do_loop_read_to_file(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;

	unsigned params[4];

	int err = 1;

	int tok_count = 0;

	char* token;

	if (APP_DATA_INIT(app_data))
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

					loop_read_to_file_process(APP_DATA_PROC_CONT(app_data), params[0], params[1], params[2], params[3], token);
					to_return = TRUE;
				}
				else
				{
					to_return = FALSE;
					printf("Not enough or bad arguments. usage: loopReadToFile vAddr id off amount filename\n");
				}
			}
		}
		else
		{
			to_return = FALSE;
			printf("Not enough or bad arguments. usage: loopReadToFile vAddr id off amount filename\n");
		}
	}
	else
		to_return = FALSE;

	return to_return;
}

BOOL do_write(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;

	unsigned params[2];
	char* s;

	int err = 1;
	int tok_count = 0;

	char* token;
	DEBUG2("cmd->command = %s\ncmd->param = %s\n", cmd->command, cmd->param);
	if (APP_DATA_INIT(app_data))
	{
		if (strlen(cmd->param) != 0)
		{
			token = strtok(cmd->param," ");

			while (err > 0 && token != NULL && tok_count < 2)
			{
				err = sscanf(token, " %u ", &params[tok_count]);

				if (err > 0)
				{
					DEBUG1("err = %d\n", err);
					DEBUG1("before token = %s\n", token);
					DEBUG1("before ++tok_count = %d\n", tok_count);
					++tok_count;
					DEBUG1("after ++tok_count = %d\n", tok_count);
					token = strtok(NULL," ");
					DEBUG1("after token = %s\n", token);
				}
				else
					to_return = FALSE;
			}
			if (tok_count < 2)
			{
				token = strtok(NULL, " ");

				if (token != NULL)
				{
					s = token;
					++tok_count;

					DEBUG3("Calling write_process with: %d %d %s\n", params[0], params[1], token);
					write_process(APP_DATA_PROC_CONT(app_data), params[0], params[1], token);

					to_return = TRUE;
				}
				else
				{
					to_return = FALSE;
					printf("Not enough or bad arguments. usage: write vAddr id s\n");
				}
			}
		}
		else
		{
			to_return = FALSE;
			printf("Not enough or bad arguments. usage: write vAddr id s\n");
		}
	}
	else
		to_return = FALSE;

	return to_return;
}

BOOL do_loop_write(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;

	unsigned params[4];
	char* s;

	int err = 1;
	int tok_count = 0;

	char* token;

	if (APP_DATA_INIT(app_data))
	{
		if (strlen(cmd->param) != 0)
		{
			token = strtok(cmd->param," ");
			while (err > 0 && token != NULL && tok_count < 5)
			{
				//Ugly code!
				if (tok_count < 2)
					err = sscanf(token, " %u ", &params[tok_count]);
				else if (tok_count == 2)
				{
					s = token;
					err = strlen(s);
					if (err != 1)
						err = -1;
				}
				else
					err = sscanf(token, " %u ", &params[tok_count-1]);

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
				to_return = FALSE;
				printf("Not enough or bad arguments. usage: loopWrite vAddr id c off amount\n");
			}
			else
			{
				loop_write_process(APP_DATA_PROC_CONT(app_data), params[0], params[1], s[0], params[2], params[3]);
			}
		}
		else
		{
			to_return = FALSE;
			printf("Not enough or bad arguments. usage: loopWrite vAddr id c off amount\n");
		}
	}
	else
		to_return = FALSE;

	return to_return;
}

BOOL do_hit_rate(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (APP_DATA_INIT(app_data))
	{
		print_hit_rate(APP_DATA_MMU(app_data));
		return TRUE;
	}
	else
		return FALSE;
}

BOOL do_print_MM(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (APP_DATA_INIT(app_data))
	{
		print_MM(APP_DATA_MMU(app_data) -> mem);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL do_print_MMU_table(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (APP_DATA_INIT(app_data))
	{
		print_MMU_table(&(APP_DATA_MMU(app_data) -> mem_ipt));
		return TRUE;
	}
	else
		return FALSE;
}

BOOL do_print_registers(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (APP_DATA_INIT(app_data))
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
	if (APP_DATA_INIT(app_data))
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
	monitor_on();
	return TRUE;
}

BOOL do_no_monitor(ui_cmd_t* cmd, app_data_t* app_data)
{
	monitor_off();
	return TRUE;
}

BOOL do_debug_mode(ui_cmd_t* cmd, app_data_t* app_data)
{
	debug_on();
	return TRUE;
}

BOOL do_batch_file(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;

	ui_cmd_t batch_cmd;

	FILE* f;
	BOOL cmd_ret = TRUE;
	BOOL done = FALSE;
	int line = 0;

	bzero(&batch_cmd,sizeof(ui_cmd_t));

	if (APP_DATA_INIT(app_data))
	{
		f = fopen(cmd -> param, "r");

		if (f == NULL)
		{
			printf("Can't open the file specified: %s\n", cmd -> param);
			to_return = TRUE; //see comment below about done flag
		}
		else
		{
			while (fscanf(f, " %s %s \n", batch_cmd.command, batch_cmd.param) != EOF
					&& cmd_ret
					&& !done)
			{
				if (strcmp("exit", batch_cmd.command) == 0)
					done = TRUE;
				else
				{
					++line;
					cmd_ret = command_handler(&batch_cmd, app_data);
				}
			}
			
			if (done)
			{
				// we return false just in case that the exit command appeared in the file
				// a little bit ugly though
				to_return = FALSE;
			}
			else if (cmd_ret == FALSE)
			{
				printf("Batch file %s command error:\nLine %d: %s %s\n", cmd -> param, line, batch_cmd.command, batch_cmd.param);
				to_return = TRUE;
			}
			else
				to_return = TRUE;
		}

		fclose(f);
	}
	else
		to_return = FALSE;

	return to_return;
}

/**
 * the main function of the application
 */
int app_main(int argc, char** argv)
{
	app_data_t app_data;
	ui_cmd_t cmd;

	BOOL done = FALSE;
	BOOL cmd_ret = FALSE;
	memset(&app_data, 0, sizeof(app_data));

	if (argc != 2)
	{
		printf("\nInput error\n-----------\nUsage: sim filename\nShutting down...\n\n");
		return -1;
	}

	if (!load_app_data(argv[1], &app_data))
	{
		return -1;
	}

	do
	{
		cmd = get_command();

		cmd_ret = command_handler(&cmd, &app_data);

		// ohhhh... this is ugly
		if (!cmd_ret && strcmp("batchFile", cmd.command))
			done = TRUE;
	} while (!done);

	if (app_data.initialized)
	{
		free_app_data(&app_data);
	}
	printf("VMSim has finished. Have a nice day! =)");
	return 0;
}

//TODO create lock for printing operations? - only UI thread is using printing..
//TODO create a function for checking APP_DATA_INIT instead of the current situation
