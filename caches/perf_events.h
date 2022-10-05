#ifndef PERF_EVENTS_TRACKER
#define PERF_EVENTS_TRACKER

/*for performance tracking*/
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

struct read_format {
  unsigned long nr;
  struct {
      unsigned long value;
      unsigned long id;
  } values[];
};

#ifdef RISCV64
#define SINGLE_MASK 0x1
#define MEMSYS_EVENTS 0x2
#elif ARMA55
#endif

void reset_and_enable_ioctl(int fd);
void disable_ioctl(int fd);
void config_perf(struct perf_event_attr *pe,int *fd,uint64_t type, uint64_t config);
void config_perf_multi(struct perf_event_attr *pe,int *fd1, int *fd2, uint64_t *id, uint64_t type, uint64_t config);
void parse_perf(int fd1, char *buf, struct read_format* rf, uint64_t *id, uint64_t *newarr,int n);
#endif
