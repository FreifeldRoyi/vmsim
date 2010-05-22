/*
 * ui_app.c
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */
#include "ui_app.h"
#include <string.h>

#define UNUSED(_x) if ((_x) != (_x)) {(_x)=(_x);}
#define IPT_SIZE 1024 //TODO initial size. needs change later
#define DISK_NPAGES 1024//TODO initial size. needs change later
#define PAGE_SIZE 32 //TODO initial size. needs change later
#define BLOCK_SIZE 8 //TODO initial size. needs change later

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

BOOL do_create_process(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_del_process(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_read(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_loop_read(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_read_to_file(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_loop_read_to_file(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_write(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_loop_write(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_hit_rate(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_print_MM(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (!(app_data->initialized))
	{
		print_MM(&APP_DATA_MM(app_data));
		return TRUE;
	}
	else
		return FALSE;

}

BOOL do_print_MMU_table(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (!(app_data->initialized))
	{
		print_MMU_table(&app_data->ipt_table);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL do_print_registers(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_print_HAT(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_monitor(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_no_monitor(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_batch_file(ui_cmd_t* cmd, app_data_t* app_data);

/**
 * the main function of the application
 */
int app_main(int argc, char **argv)
{
	app_data_t *app_data;
	ui_cmd_t cmd;

	memset(app_data, 0, sizeof(app_data));

	BOOL exit = FALSE;

	APP_DATA_INIT(app_data) = FALSE;
	//APP_DATA_MM(app_data) = 0; was set by memset above
	//APP_DATA_DISK(app_data) = 0; was set by memset above

	//ipt_init(&(app_data.ipt_table),IPT_SIZE,handlers...); TODO this
	//mm_init(app_data.main_memory...) TODO this
	disk_init(&APP_DATA_DISK(app_data),DISK_NPAGES,PAGE_SIZE,BLOCK_SIZE);

	do
	{
		cmd = get_command();
		if (!strcmp("exit", cmd.command))
		{
			exit = TRUE;
		}
		else if ()
	} while (!exit);
}
