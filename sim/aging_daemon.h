#ifndef AGING_DAEMON_H_
#define AGING_DAEMON_H_

#include "sim/mmu.h"
#include "util/vmsim_types.h"

errcode_t aging_daemon_start(mmu_t* mmu);

errcode_t aging_daemon_update_pages();

void aging_daemon_stop();

#endif
