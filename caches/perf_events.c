#include "perf_events.h"

void reset_and_enable_ioctl(int fd){
    ioctl(fd, PERF_EVENT_IOC_RESET,PERF_IOC_FLAG_GROUP);
    ioctl(fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
}

void disable_ioctl(int fd){
    ioctl(fd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
}

void config_perf(struct perf_event_attr *pe,int *fd,uint64_t type, uint64_t config){
  memset(pe, 0, sizeof(*pe));
  pe->type = type;
  pe->size = sizeof(*pe);
  pe->config = config;
  pe->disabled = 1;
  pe->exclude_kernel = 1;
  pe->exclude_hv = 1;
  *fd = syscall(__NR_perf_event_open, pe, 0, -1, -1, 0);
}

void config_perf_multi(struct perf_event_attr *pe,int *fd1, int *fd2, uint64_t *id, uint64_t type, uint64_t config){
  memset(pe, 0, sizeof(*pe));
  pe->type = type;
  pe->size = sizeof(*pe);
  pe->config = config;
  pe->disabled = 1;
  pe->exclude_kernel = 1;
  pe->exclude_hv = 1;
  pe->read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
  *fd1 = syscall(__NR_perf_event_open, pe, 0, -1, *fd2, 0);
  ioctl(*fd1, PERF_EVENT_IOC_ID, id);
}

void parse_perf(int fd1, char *buf, struct read_format *rf, uint64_t id[], uint64_t newarr[],int n){
  read(fd1, *buf, sizeof(*buf));
  for (int i = 0; i < rf->nr; i++) {
    for (int j = 0; j<n;j++){
      if(rf->values[i].id==id[j])
        newarr[i]=rf->values[i].value;
    }
}
}
