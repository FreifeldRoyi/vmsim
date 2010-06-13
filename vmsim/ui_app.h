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
 *	@param stream - stream from which data will be read
 *
 *  @return the string that were read
 */
ui_cmd_t get_command();

/**
 * Starts a creation chain, which in the end a new process should be created and run.
 * If not a proper message will be printed
 *
 * @param cmd - the command
 * @param app_data -the app data
 *
 * @return TRUE if process is created FALSE otherwise
 */
BOOL do_create_process(ui_cmd_t* cmd, app_data_t* app_data);

/**
 * Sends a delete message to a process specified
 * UI waits untill deleting is done
 *
 * @param -the command
 * @param app_data - the app data
 */
BOOL do_del_process(ui_cmd_t* cmd, app_data_t* app_data);

/**
 * The next functions sends a message to a process' mail box
 * according to @param cmd params
 * UI will not wait until operation has finished
 */
BOOL do_read(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_loop_read(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_read_to_file(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_loop_read_to_file(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_write(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_loop_write(ui_cmd_t* cmd, app_data_t* app_data);

/**
 * The next function will cause a chain of function calling, causing
 * a corresponding printing in the end
 */
BOOL do_hit_rate(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_print_MM(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_print_MMU_table(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_print_registers(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_print_HAT(ui_cmd_t* cmd, app_data_t* app_data);

/**
 * change logger mode
 */
BOOL do_monitor(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_no_monitor(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_debug_mode(ui_cmd_t* cmd, app_data_t* app_data);

/**
 * read a batch file according to @param cmd params
 */
BOOL do_batch_file(ui_cmd_t* cmd, app_data_t* app_data);

/**
 * the main function of the application
 */
int app_main(int argc, char **argv);

#endif /* UI_APP_H_ */
