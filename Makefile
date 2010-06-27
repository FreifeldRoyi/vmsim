.PHONY: all clean

OUTDIR := obj

LDFLAGS := ${LDFLAGS} -pthread
CFLAGS := -O2 -g3 -Wall -Wextra -pthread -Wno-unused-parameter -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast ${CFLAGS}

INCLUDE := .

APPNAME := sim
all: vmsim
main.o: main.c cunit/cunit.h vmsim/ui_app.h util/app_util.h \
 util/vmsim_types.h vmsim/ipt.h util/vmsim_types.h util/locks.h \
 util/queue.h util/list.h vmsim/mm.h vmsim/disk.h util/bitmap.h \
 vmsim/mmu.h util/map.h vmsim/ipt.h vmsim/mm.h vmsim/disk.h vmsim/pcb.h \
 util/pcb_util.h util/worker_thread.h util/locks.h
	gcc -I . ${CFLAGS} -c ./main.c -o ${OUTDIR}/main.o
disk.o: vmsim/disk.c vmsim/disk.h util/bitmap.h util/vmsim_types.h \
 util/locks.h util/vmsim_types.h tests/disk_tests.c cunit/cunit.h \
 util/worker_thread.h util/locks.h
	gcc -I . ${CFLAGS} -c ./vmsim/disk.c -o ${OUTDIR}/disk.o
ui_app.o: vmsim/ui_app.c vmsim/ui_app.h util/app_util.h \
 util/vmsim_types.h vmsim/ipt.h util/vmsim_types.h util/locks.h \
 util/queue.h util/list.h vmsim/mm.h vmsim/disk.h util/bitmap.h \
 vmsim/mmu.h util/map.h vmsim/ipt.h vmsim/mm.h vmsim/disk.h vmsim/pcb.h \
 util/pcb_util.h util/worker_thread.h util/locks.h util/logger.h \
 vmsim/prm.h vmsim/mmu.h vmsim/aging_daemon.h
	gcc -I . ${CFLAGS} -c ./vmsim/ui_app.c -o ${OUTDIR}/ui_app.o
mmu.o: vmsim/mmu.c vmsim/mmu.h util/queue.h util/list.h util/locks.h \
 util/vmsim_types.h util/map.h vmsim/ipt.h util/vmsim_types.h vmsim/mm.h \
 vmsim/disk.h util/bitmap.h vmsim/prm.h util/logger.h \
 vmsim/aging_daemon.h vmsim/mmu.h tests/mmu_tests.c cunit/cunit.h \
 util/worker_thread.h util/locks.h
	gcc -I . ${CFLAGS} -c ./vmsim/mmu.c -o ${OUTDIR}/mmu.o
mm.o: vmsim/mm.c vmsim/mm.h util/vmsim_types.h util/locks.h \
 util/vmsim_types.h
	gcc -I . ${CFLAGS} -c ./vmsim/mm.c -o ${OUTDIR}/mm.o
prm.o: vmsim/prm.c vmsim/prm.h util/vmsim_types.h vmsim/mmu.h \
 util/queue.h util/list.h util/locks.h util/vmsim_types.h util/map.h \
 vmsim/ipt.h vmsim/mm.h vmsim/disk.h util/bitmap.h util/worker_thread.h \
 util/locks.h util/logger.h
	gcc -I . ${CFLAGS} -c ./vmsim/prm.c -o ${OUTDIR}/prm.o
ipt.o: vmsim/ipt.c vmsim/ipt.h util/vmsim_types.h util/locks.h \
 util/vmsim_types.h util/queue.h util/list.h util/logger.h \
 tests/ipt_tests.c cunit/cunit.h
	gcc -I . ${CFLAGS} -c ./vmsim/ipt.c -o ${OUTDIR}/ipt.o
pcb.o: vmsim/pcb.c vmsim/pcb.h util/pcb_util.h util/worker_thread.h \
 util/vmsim_types.h util/locks.h util/vmsim_types.h vmsim/mmu.h \
 util/queue.h util/list.h util/locks.h util/map.h vmsim/ipt.h vmsim/mm.h \
 vmsim/disk.h util/bitmap.h util/logger.h
	gcc -I . ${CFLAGS} -c ./vmsim/pcb.c -o ${OUTDIR}/pcb.o
aging_daemon.o: vmsim/aging_daemon.c vmsim/aging_daemon.h vmsim/mmu.h \
 util/queue.h util/list.h util/locks.h util/vmsim_types.h util/map.h \
 vmsim/ipt.h util/vmsim_types.h vmsim/mm.h vmsim/disk.h util/bitmap.h \
 util/worker_thread.h util/locks.h util/logger.h
	gcc -I . ${CFLAGS} -c ./vmsim/aging_daemon.c -o ${OUTDIR}/aging_daemon.o
locks.o: util/locks.c util/locks.h util/vmsim_types.h util/logger.h
	gcc -I . ${CFLAGS} -c ./util/locks.c -o ${OUTDIR}/locks.o
logger.o: util/logger.c util/logger.h
	gcc -I . ${CFLAGS} -c ./util/logger.c -o ${OUTDIR}/logger.o
list.o: util/list.c util/list.h
	gcc -I . ${CFLAGS} -c ./util/list.c -o ${OUTDIR}/list.o
queue.o: util/queue.c util/queue.h util/list.h tests/queue_tests.c \
 util/queue.h cunit/cunit.h
	gcc -I . ${CFLAGS} -c ./util/queue.c -o ${OUTDIR}/queue.o
worker_thread.o: util/worker_thread.c util/worker_thread.h \
 util/vmsim_types.h util/locks.h tests/worker_thread_tests.c \
 cunit/cunit.h
	gcc -I . ${CFLAGS} -c ./util/worker_thread.c -o ${OUTDIR}/worker_thread.o
pcb_util.o: util/pcb_util.c util/pcb_util.h util/worker_thread.h \
 util/vmsim_types.h util/locks.h util/vmsim_types.h vmsim/mmu.h \
 util/queue.h util/list.h util/locks.h util/map.h vmsim/ipt.h vmsim/mm.h \
 vmsim/disk.h util/bitmap.h util/app_util.h vmsim/ipt.h vmsim/mm.h \
 vmsim/disk.h vmsim/pcb.h util/pcb_util.h util/logger.h
	gcc -I . ${CFLAGS} -c ./util/pcb_util.c -o ${OUTDIR}/pcb_util.o
app_util.o: util/app_util.c util/app_util.h util/vmsim_types.h \
 vmsim/ipt.h util/vmsim_types.h util/locks.h util/queue.h util/list.h \
 vmsim/mm.h vmsim/disk.h util/bitmap.h vmsim/mmu.h util/map.h vmsim/ipt.h \
 vmsim/mm.h vmsim/disk.h vmsim/pcb.h util/pcb_util.h util/worker_thread.h \
 util/locks.h vmsim/prm.h vmsim/mmu.h vmsim/aging_daemon.h util/logger.h \
 tests/app_util_tests.c cunit/cunit.h
	gcc -I . ${CFLAGS} -c ./util/app_util.c -o ${OUTDIR}/app_util.o
bitmap.o: util/bitmap.c util/bitmap.h util/vmsim_types.h \
 tests/bitmap_tests.c cunit/cunit.h
	gcc -I . ${CFLAGS} -c ./util/bitmap.c -o ${OUTDIR}/bitmap.o
map.o: util/map.c util/map.h util/vmsim_types.h tests/map_tests.c \
 cunit/cunit.h
	gcc -I . ${CFLAGS} -c ./util/map.c -o ${OUTDIR}/map.o
cunit.o: cunit/cunit.c cunit/cunit.h util/queue.h util/list.h \
 util/vmsim_types.h
	gcc -I . ${CFLAGS} -c ./cunit/cunit.c -o ${OUTDIR}/cunit.o
vmsim:  main.o disk.o ui_app.o mmu.o mm.o prm.o ipt.o pcb.o aging_daemon.o locks.o logger.o list.o queue.o worker_thread.o pcb_util.o app_util.o bitmap.o map.o cunit.o
	[ -d ${OUTDIR} ] || mkdir -p ${OUTDIR}
	cd ${OUTDIR}
	gcc ${LDFLAGS}  ${OUTDIR}/main.o ${OUTDIR}/disk.o ${OUTDIR}/ui_app.o ${OUTDIR}/mmu.o ${OUTDIR}/mm.o ${OUTDIR}/prm.o ${OUTDIR}/ipt.o ${OUTDIR}/pcb.o ${OUTDIR}/aging_daemon.o ${OUTDIR}/locks.o ${OUTDIR}/logger.o ${OUTDIR}/list.o ${OUTDIR}/queue.o ${OUTDIR}/worker_thread.o ${OUTDIR}/pcb_util.o ${OUTDIR}/app_util.o ${OUTDIR}/bitmap.o ${OUTDIR}/map.o ${OUTDIR}/cunit.o -o ${APPNAME}
