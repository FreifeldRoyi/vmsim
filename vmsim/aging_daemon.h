#ifndef AGING_DAEMON_H_
#define AGING_DAEMON_H_

/**Aging daemon thread, as described in the assignment*/

#include "vmsim/mmu.h"
#include "util/vmsim_types.h"

/**Start running the aging thread.
 *
 * @param mmu the mmu that the thread should use.
 *
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t aging_daemon_start(mmu_t* mmu);

/**Request the aging daemon to update the ages of all the pages in it's
 * MMU.
 *
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t aging_daemon_update_pages();

/**Stop the aging daemon.
 * */
void aging_daemon_stop();

#endif
