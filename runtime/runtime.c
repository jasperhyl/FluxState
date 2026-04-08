#define _POSIX_C_SOURCE 200809L

#include "runtime.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum {
  FS_EVENT_QUEUE_INITIAL_CAP = 16u, // 初始事件队列分配能放16个事件指针的空间。
  FS_TIMER_TABLE_INITIAL_CAP = 8u,  // 定时器表刚创建出来时，先分配能放8个定时器的空间。
};

// 如果当前没有事件可处理，也没有即将到期的 timer，那 runtime 不要空转死循环，先 sleep 1ms，再继续下一轮
static const uint64_t FS_IDLE_SLEEP_NS = 1000ULL * 1000ULL;

// 当 runtime 内存分配失败时，立刻打印错误并终止程序。
static void fs_fatal_oom(void) { // out of memory
  // 在标准错误输出
  fputs("FluxState runtime: out of memory\n", stderr);
  // 直接异常终止进程
  abort();
}

// xcalloc带失败即退出的calloc
static void *fs_xcalloc(size_t count, size_t size) {
  void *ptr = calloc(count, size);
  if (ptr == NULL) {
    fs_fatal_oom();
  }
  return ptr;
}

// 把一块已有内存 ptr 重新调整到新的大小 size
static void *fs_xrealloc(void *ptr, size_t size) {
  void *next = realloc(ptr, size);
  if (next == NULL) {
    fs_fatal_oom();
  }
  return next;
}

// 获取当前单调时钟时间，并把它转换成纳秒返回。
static uint64_t fs_now_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000ULL * 1000ULL * 1000ULL + (uint64_t)ts.tv_nsec;
}

//
static void fs_sleep_ns(uint64_t ns) {
  if (ns == 0) {
    return;
  }

  struct timespec req;
  req.tv_sec = (time_t)(ns / (1000ULL * 1000ULL * 1000ULL));
  req.tv_nsec = (long)(ns % (1000ULL * 1000ULL * 1000ULL));
  while (nanosleep(&req, &req) != 0 && errno == EINTR) {
  }
}

static void fs_destroy_event(FS_Event *event) {
  if (event == NULL) {
    return;
  }
  if ((event->flags & FS_EVENT_FREE_PAYLOAD) != 0 && event->payload != NULL) {
    free(event->payload);
  }
  free(event);
}

static FS_EventQueue *fs_create_event_queue(void) {
  FS_EventQueue *queue = (FS_EventQueue *)fs_xcalloc(1, sizeof(FS_EventQueue));
  queue->cap = FS_EVENT_QUEUE_INITIAL_CAP;
  queue->data = (FS_Event **)fs_xcalloc(queue->cap, sizeof(FS_Event *));
  return queue;
}

static void fs_grow_event_queue(FS_EventQueue *queue) {
  uint32_t new_cap = queue->cap == 0 ? FS_EVENT_QUEUE_INITIAL_CAP : queue->cap * 2u;
  FS_Event **new_data = (FS_Event **)fs_xcalloc(new_cap, sizeof(FS_Event *));
  for (uint32_t i = 0; i < queue->size; ++i) {
    new_data[i] = queue->data[(queue->head + i) % queue->cap];
  }
  free(queue->data);
  queue->data = new_data;
  queue->cap = new_cap;
  queue->head = 0;
  queue->tail = queue->size;
}

static void fs_destroy_event_queue(FS_EventQueue *queue) {
  if (queue == NULL) {
    return;
  }
  while (queue->size > 0) {
    FS_Event *event = queue->data[queue->head];
    queue->data[queue->head] = NULL;
    queue->head = (queue->head + 1u) % queue->cap;
    queue->size -= 1u;
    fs_destroy_event(event);
  }
  free(queue->data);
  free(queue);
}

static FS_Event *fs_pop_event(FS_EventQueue *queue) {
  if (queue == NULL || queue->size == 0) {
    return NULL;
  }
  FS_Event *event = queue->data[queue->head];
  queue->data[queue->head] = NULL;
  queue->head = (queue->head + 1u) % queue->cap;
  queue->size -= 1u;
  return event;
}

static FS_TimerTable *fs_create_timer_table(void) {
  FS_TimerTable *timers = (FS_TimerTable *)fs_xcalloc(1, sizeof(FS_TimerTable));
  timers->cap = FS_TIMER_TABLE_INITIAL_CAP;
  timers->data = (FS_Timer *)fs_xcalloc(timers->cap, sizeof(FS_Timer));
  return timers;
}

static void fs_grow_timer_table(FS_TimerTable *timers) {
  uint32_t new_cap = timers->cap == 0 ? FS_TIMER_TABLE_INITIAL_CAP : timers->cap * 2u;
  timers->data = (FS_Timer *)fs_xrealloc(timers->data, sizeof(FS_Timer) * new_cap);
  memset(timers->data + timers->cap, 0, sizeof(FS_Timer) * (new_cap - timers->cap));
  timers->cap = new_cap;
}

static void fs_destroy_timer_table(FS_TimerTable *timers) {
  if (timers == NULL) {
    return;
  }
  free(timers->data);
  free(timers);
}

static FS_Timer *fs_find_timer(FS_TimerTable *timers, uint32_t event_tag) {
  if (timers == NULL) {
    return NULL;
  }
  for (uint32_t i = 0; i < timers->count; ++i) {
    if (timers->data[i].event_tag == event_tag) {
      return &timers->data[i];
    }
  }
  return NULL;
}

static FS_Timer *fs_get_or_add_timer(FS_TimerTable *timers, uint32_t event_tag) {
  FS_Timer *timer = fs_find_timer(timers, event_tag);
  if (timer != NULL) {
    return timer;
  }

  if (timers->count == timers->cap) {
    fs_grow_timer_table(timers);
  }

  timer = &timers->data[timers->count++];
  memset(timer, 0, sizeof(FS_Timer));
  timer->event_tag = event_tag;
  return timer;
}

static FS_Timer *fs_find_expired_timer(FS_TimerTable *timers, uint64_t now_ns) {
  FS_Timer *expired = NULL;
  if (timers == NULL) {
    return NULL;
  }
  for (uint32_t i = 0; i < timers->count; ++i) {
    FS_Timer *timer = &timers->data[i];
    if (!timer->active || timer->deadline_ns > now_ns) {
      continue;
    }
    if (expired == NULL || timer->deadline_ns < expired->deadline_ns) {
      expired = timer;
    }
  }
  return expired;
}

static uint64_t fs_time_until_next_timer(const FS_TimerTable *timers, uint64_t now_ns) {
  uint64_t best = UINT64_MAX;
  if (timers == NULL) {
    return best;
  }
  for (uint32_t i = 0; i < timers->count; ++i) {
    const FS_Timer *timer = &timers->data[i];
    if (!timer->active) {
      continue;
    }
    if (timer->deadline_ns <= now_ns) {
      return 0;
    }
    uint64_t delta = timer->deadline_ns - now_ns;
    if (delta < best) {
      best = delta;
    }
  }
  return best;
}

static FS_Event *fs_create_timeout_event(uint32_t event_tag, uint64_t now_ns) {
  FS_Event *event = (FS_Event *)fs_xcalloc(1, sizeof(FS_Event));
  event->tag = event_tag;
  event->flags = 0;
  event->reserved = 0;
  event->payload_size = 0;
  event->payload = NULL;
  event->timestamp_ns = now_ns;
  return event;
}

static bool fs_enqueue_expired_timer(FS_MachineHeader *machine) {
  uint64_t now_ns = fs_now_ns();
  FS_Timer *timer = fs_find_expired_timer(machine->timers, now_ns);
  if (timer == NULL) {
    return false;
  }

  FS_Event *event = fs_create_timeout_event(timer->event_tag, now_ns);
  if (timer->periodic && timer->period_ns > 0) {
    timer->deadline_ns = now_ns + timer->period_ns;
  } else {
    timer->active = 0;
  }

  fs_enqueue_event(machine, event);
  return true;
}

void fs_enqueue_event(FS_MachineHeader *machine, FS_Event *event) {
  if (machine == NULL || event == NULL) {
    return;
  }
  if (machine->queue == NULL) {
    machine->queue = fs_create_event_queue();
  }
  if (event->timestamp_ns == 0) {
    event->timestamp_ns = fs_now_ns();
  }
  if (machine->queue->size == machine->queue->cap) {
    fs_grow_event_queue(machine->queue);
  }
  machine->queue->data[machine->queue->tail] = event;
  machine->queue->tail = (machine->queue->tail + 1u) % machine->queue->cap;
  machine->queue->size += 1u;
}

void fs_start_timer(FS_MachineHeader *machine, uint32_t event_tag, uint64_t delay_ns, uint64_t retry_ns) {
  if (machine == NULL) {
    return;
  }
  if (machine->timers == NULL) {
    machine->timers = fs_create_timer_table();
  }

  FS_Timer *timer = fs_get_or_add_timer(machine->timers, event_tag);
  timer->deadline_ns = fs_now_ns() + delay_ns;
  timer->period_ns = retry_ns;
  timer->active = 1;
  timer->periodic = retry_ns > 0 ? 1 : 0;
}

void fs_cancel_timer(FS_MachineHeader *machine, uint32_t event_tag) {
  if (machine == NULL || machine->timers == NULL) {
    return;
  }
  FS_Timer *timer = fs_find_timer(machine->timers, event_tag);
  if (timer != NULL) {
    timer->active = 0;
  }
}

bool fs_init_machine(FS_MachineHeader *machine, uint32_t initial_state, FS_DispatchFn dispatch) {
  if (machine == NULL) {
    return false;
  }

  if (machine->queue == NULL) {
    machine->queue = fs_create_event_queue();
  }
  if (machine->timers == NULL) {
    machine->timers = fs_create_timer_table();
  }
  if (machine->dispatch == NULL) {
    machine->dispatch = dispatch;
  }

  if ((machine->flags & FS_MACHINE_INITIALIZED) != 0) {
    machine->flags |= FS_MACHINE_RUNNING;
    return false;
  }

  machine->current_state = initial_state;
  machine->entered_at_ns = fs_now_ns();
  machine->dispatch = dispatch;
  machine->flags = FS_MACHINE_INITIALIZED | FS_MACHINE_RUNNING;
  return true;
}

void fs_cleanup_machine(FS_MachineHeader *machine) {
  if (machine == NULL) {
    return;
  }
  fs_destroy_event_queue(machine->queue);
  fs_destroy_timer_table(machine->timers);
  machine->queue = NULL;
  machine->timers = NULL;
  machine->dispatch = NULL;
  machine->entered_at_ns = 0;
  machine->current_state = 0;
  machine->flags = 0;
}

bool fs_runtime_step(FS_MachineHeader *machine) {
  if (machine == NULL || machine->dispatch == NULL) {
    return false;
  }
  if ((machine->flags & FS_MACHINE_INITIALIZED) == 0 || (machine->flags & FS_MACHINE_RUNNING) == 0) {
    return false;
  }

  FS_Event *event = fs_pop_event(machine->queue);
  if (event == NULL && !fs_enqueue_expired_timer(machine)) {
    return false;
  }
  if (event == NULL) {
    event = fs_pop_event(machine->queue);
    if (event == NULL) {
      return false;
    }
  }

  uint32_t before_state = machine->current_state;
  machine->dispatch(machine, event);
  if (machine->current_state != before_state) {
    machine->entered_at_ns = fs_now_ns();
  }

  fs_destroy_event(event);
  return true;
}

void fs_runtime_run(FS_MachineHeader *machine) {
  if (machine == NULL || machine->dispatch == NULL) {
    return;
  }
  while ((machine->flags & FS_MACHINE_RUNNING) != 0) {
    if (fs_runtime_step(machine)) {
      continue;
    }

    uint64_t now_ns = fs_now_ns();
    uint64_t wait_ns = fs_time_until_next_timer(machine->timers, now_ns);
    if (wait_ns == UINT64_MAX) {
      wait_ns = FS_IDLE_SLEEP_NS;
    }
    fs_sleep_ns(wait_ns);
  }
}

void fs_stop_machine(FS_MachineHeader *machine) {
  if (machine == NULL) {
    return;
  }
  machine->flags &= ~FS_MACHINE_RUNNING;
}
