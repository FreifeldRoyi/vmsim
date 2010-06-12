/*
 * ui_app.h
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */

#ifndef UI_APP_H_
#define UI_APP_H_

#include "util/app_util.h"
#include <stdio.h>

#define PROMPT ">"
#define MAX_CMD_LEN 14

typedef struct _ui_cmd_t
{
	char command[MAX_CMD_LEN+1];
	char param[FILENAME_MAX];
} ui_cmd_t;

/**
 * Displays a prompt and read a command form the user.
 *	The command that was read, is stored in ui_cmd_t.
 *	The command is stored in cmd.command and the parameters are stored in cmd.params
 *
 *  @return the string that were read
 */
ui_cmd_t get_command();

BOOL do_create_process(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_del_process(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_read(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_loop_read(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_read_to_file(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_loop_read_to_file(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_write(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_loop_write(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_hit_rate(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_print_MM(ui_cmd_t* cmd, app_data_t* app_data);

/**
 *	prints the IPT
 */
BOOL do_print_MMU_table(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_print_registers(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_print_HAT(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_monitor(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_no_monitor(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_debug_mode(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_batch_file(ui_cmd_t* cmd, app_data_t* app_data);

/**
 * the main function of the application
 */
int app_main(int argc, char **argv);

#endif /* UI_APP_H_ */
