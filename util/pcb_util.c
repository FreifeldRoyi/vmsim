/*
 * pcb_util.c
 *
 *  Created on: Jun 2, 2010
 *      Author: Freifeld Royi
 */

#include "pcb_util.h"
#include <malloc.h>
#include <assert.h>

post_err_t compose_mail(process_t* prc, post_t* post)
{
	assert(prc != NULL);
	assert(post != NULL);

	pthread_mutex_lock(&PROC_LOCK(prc));
	if (prc == NULL)
	{
		pthread_mutex_unlock(&PROC_LOCK(prc));
		return peFail;
	}

	if (PROC_DEL(prc)) //process got exit flag
	{
		pthread_mutex_unlock(&PROC_LOCK(prc));
		return peEnd;
	}

	queue_push(PROC_MAIL(prc), post);
	pthread_cond_signal(&PROC_COND(prc));
	pthread_mutex_unlock(&PROC_LOCK(prc));
	return peSuccess;
}

post_t* create_post(func_t func, void** args, int nargs)
{
	post_t* to_return = (post_t*)malloc(sizeof(post_t));

	to_return -> args = args;
	to_return -> func = func;
	to_return -> n_args = nargs;

	return to_return;
}

void func_arg_destroy(func_arg* arg)
{
	assert(arg != NULL);
	free(arg);
}

void post_destroy(post_t* post)
{
	int i;
	assert(post != NULL);

	if (post -> args != NULL)
	{
		for (i = 0; i < post -> n_args; ++i)
		{
			free((post -> args)[i]);
		}
	}

	free(post);
}

/****************************************process functions*******************************/

errcode_t process_dealloc(proc_cont_t* proc_cont, procid_t pid)
{
	//TODO implement
	// man don't forget to delete pages from MM and DISK
}
