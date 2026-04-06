#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FS_String {
  char *data;
  uint64_t len;
} FS_String;

typedef struct FS_Event {
  uint32_t tag;
  uint32_t flags;
  uint16_t reserved;
  uint32_t payload_size;
  void *payload;
  uint64_t timestamp_ns;
} FS_Event;

typedef struct FS_EventQueue {
  uint32_t head;
  uint32_t tail;
  uint32_t size;
  uint32_t cap;
  FS_Event **data;
} FS_EventQueue;

typedef struct FS_Timer {
  uint64_t deadline_ns;
  uint64_t period_ns;
  uint32_t event_tag;
  uint8_t active;
  uint8_t periodic;
  uint16_t reserved;
} FS_Timer;

typedef struct FS_TimerTable {
  uint32_t count;
  uint32_t cap;
  FS_Timer *data;
} FS_TimerTable;

typedef void (*FS_DispatchFn)(void *machine, FS_Event *event);

typedef struct FS_MachineHeader {
  uint32_t current_state;
  uint32_t flags;
  uint64_t entered_at_ns;
  FS_EventQueue *queue;
  FS_TimerTable *timers;
  FS_DispatchFn dispatch;
} FS_MachineHeader;

enum {
  FS_EVENT_FREE_PAYLOAD = 1u << 0,
};

enum {
  FS_MACHINE_INITIALIZED = 1u << 0,
  FS_MACHINE_RUNNING = 1u << 1,
};

void fs_enqueue_event(FS_MachineHeader *machine, FS_Event *event);
void fs_start_timer(FS_MachineHeader *machine, uint32_t event_tag, uint64_t delay_ns, uint64_t retry_ns);
void fs_cancel_timer(FS_MachineHeader *machine, uint32_t event_tag);

bool fs_init_machine(FS_MachineHeader *machine, uint32_t initial_state, FS_DispatchFn dispatch);
void fs_cleanup_machine(FS_MachineHeader *machine);
bool fs_runtime_step(FS_MachineHeader *machine);
void fs_runtime_run(FS_MachineHeader *machine);
void fs_stop_machine(FS_MachineHeader *machine);

#ifdef __cplusplus
}
#endif
