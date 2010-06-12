/*
 * ui_app.c
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */
#include "ui_app.h"
#include "util/logger.h"
#include <malloc.h>
#include "prm.h"
#include "aging_daemon.h"
#include <string.h>
#include <assert.h>

static BOOL command_handler(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = TRUE;
	BOOL async_wait = TRUE;
	BOOL ret_cmd;

	if (!strcmp("exit", cmd -> command))
	{
		to_return = FALSE;
		async_wait=FALSE;
	}
	else if (!strcmp("createProcess", cmd -> command))
	{
		ret_cmd = do_create_process(cmd, app_data);
		async_wait=FALSE;
	}
	else if (!strcmp("delProcess", cmd -> command))
	{
		ret_cmd = do_del_process(cmd, app_data);
		async_wait=FALSE;
	}
	else if (!strcmp("read", cmd -> command))
	{
		ret_cmd = do_read(cmd, app_data);
	}
	else if (!strcmp("loopRead", cmd -> command))
	{
		ret_cmd = do_loop_read(cmd ,app_data);
	}
	else if (!strcmp("readToFile", cmd -> command))
	{
		ret_cmd = do_read_to_file(cmd ,app_data);
	}
	else if (!strcmp("loopReadToFile", cmd -> command))
	{
		ret_cmd = do_loop_read_to_file(cmd ,app_data);
	}
	else if (!strcmp("write", cmd -> command))
	{
		ret_cmd = do_write(cmd ,app_data);
	}
	else if (!strcmp("loopWrite", cmd -> command))
	{
		ret_cmd = do_loop_write(cmd ,app_data);
	}
	else if (!strcmp("hitRate", cmd -> command))
	{
		ret_cmd = do_hit_rate(cmd ,app_data);
		signal_job_done();
	}
	else if (!strcmp("printMM", cmd -> command))
	{
		ret_cmd = do_print_MM(cmd ,app_data);
		signal_job_done();
	}
	else if (!strcmp("printMMUTable", cmd -> command))
	{
		ret_cmd = do_print_MMU_table(cmd ,app_data);
		signal_job_done();
	}
	else if (!strcmp("printRegisters", cmd -> command))
	{
		ret_cmd = do_print_registers(cmd ,app_data);
		signal_job_done();
	}
	else if (!strcmp("printHAT", cmd -> command))
	{
		ret_cmd = do_print_HAT(cmd ,app_data);
		signal_job_done();
	}
	else if (!strcmp("monitor", cmd -> command))
	{
		ret_cmd = do_monitor(cmd ,app_data);
		signal_job_done();
	}
	else if (!strcmp("noMonitor", cmd -> command))
	{
		ret_cmd = do_no_monitor(cmd ,app_data);
		signal_job_done();
	}
	else if (!strcmp("debug", cmd -> command))
	{
		ret_cmd = do_debug_mode(cmd, app_data);
		signal_job_done();
	}
	else if (!strcmp("batchFile", cmd -> command))
	{
		to_return = do_batch_file(cmd ,app_data);
	}
	else
	{
		printf ("Wrong input, please type again\n");
		signal_job_done();
	}

	if (!ret_cmd)
		signal_job_done();

	if (async_wait)
	{
		wait_job_done();
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
	{
		return ret;
	}

	fgets(ret.param, FILENAME_MAX, stdin);

	//fgets stores '\n' in the buffer FFS!
	//who was the idiot who wrote that function?!?!?!
	if (ret.param[strlen(ret.param) - 1] == '\n')
		ret.param[strlen(ret.param) - 1] = '\0';

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
			if (tok_count != 3)
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
			if (tok_count != 4)
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
			if (tok_count == 3)
			{
				//token = strtok(NULL," ");
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
			else
			{
				to_return = FALSE;
				printf ("Bad or not enough arguments. usage: readToFile vAddr id amount filename\n");
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
			if (tok_count == 4)
			{
				//token = strtok(NULL," ");
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
			else
			{
				to_return = FALSE;
				printf ("Bad or not enough arguments. usage: loopReadToFile vAddr id off amount filename\n");
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
					++tok_count;
					token = strtok(NULL," ");
				}
				else
					to_return = FALSE;
			}
			if (tok_count == 2)
			{
				//token = strtok(NULL, " ");

				if (token != NULL)
				{
					s = token;
					++tok_count;

					write_process(APP_DATA_PROC_CONT(app_data), params[0], params[1], token);

					to_return = TRUE;
				}
				else
				{
					to_return = FALSE;
					printf("Not enough or bad arguments. usage: write vAddr id s\n");
				}
			}
			else
			{
				to_return = FALSE;
				printf ("Bad or not enough arguments. usage: write vAddr id s\n");
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

			if (tok_count != 5)
			{
				to_return = FALSE;
				printf("Not enough or bad arguments. usage: loopWrite vAddr id c off amount\n");
			}
			else
			{
				loop_write_process(APP_DATA_PROC_CONT(app_data), params[0], params[1], s[0], params[2], params[3]);
				to_return = TRUE;
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

//TODO fix batch parser
BOOL do_batch_file(ui_cmd_t* cmd, app_data_t* app_data)
{
	BOOL to_return = FALSE;

	ui_cmd_t batch_cmd;

	FILE* f;
	//BOOL cmd_ret = TRUE;
	char* raw_cmd = (char*)malloc((MAX_CMD_LEN + 1 + FILENAME_MAX) * sizeof(char));
	char* raw_cmd_for;
	//int nchar_raw; //number of chars read from sscanf
	//int nchar_cmd;
	BOOL not_done = TRUE;
	int line = 1;

	assert(raw_cmd != NULL);

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

			DEBUG1("Reading line %d\n",line);
			raw_cmd = fgets(raw_cmd, MAX_CMD_LEN + 1 + FILENAME_MAX, f);

			if (raw_cmd != NULL && raw_cmd[strlen(raw_cmd) - 1] == '\n')
				raw_cmd[strlen(raw_cmd) - 1] = '\0';

			while (not_done && raw_cmd != NULL)//nchar_raw > 0 && nchar_raw != EOF)
			{
				memset(batch_cmd.command, 0, MAX_CMD_LEN + 1);
				memset(batch_cmd.param, 0, FILENAME_MAX);

				DEBUG2("[line %d] \"%s\" was read\n", line, raw_cmd);
				++line;

				sscanf(raw_cmd, "%14s", batch_cmd.command);
				DEBUG1("command is: %s\n", batch_cmd.command);

				if (strcmp(raw_cmd, batch_cmd.command)) //do if raw_cmd and command are different
				{
					raw_cmd_for = raw_cmd + strlen(batch_cmd.command) + 1;
					memcpy(batch_cmd.param,raw_cmd_for, strlen(raw_cmd_for) * sizeof(char));
					DEBUG1("params are: %s\n", batch_cmd.param);
				}

				not_done = command_handler(&batch_cmd, app_data);

				DEBUG1("Reading line %d\n",line);
				raw_cmd = fgets(raw_cmd, MAX_CMD_LEN + 1 + FILENAME_MAX, f);

				if (raw_cmd != NULL && raw_cmd[strlen(raw_cmd) - 1] == '\n')
					raw_cmd[strlen(raw_cmd) - 1] = '\0';
			}

			DEBUG1("No commands in line %d\nBatch file end.\n",line);
			fclose(f);
			to_return = not_done;
		}
	}
	else
		to_return = FALSE;

	free(raw_cmd);
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

	init_job_done();

	do
	{
		cmd = get_command();

		cmd_ret = command_handler(&cmd, &app_data);

		if (!cmd_ret)
			done = TRUE;
	} while (!done);

	if (app_data.initialized)
	{
		free_app_data(&app_data);
	}

	destroy_job_done();
	printf("VMSim has finished. Have a nice day! =)\n");
	return 0;
}
