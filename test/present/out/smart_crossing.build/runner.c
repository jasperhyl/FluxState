#include "runtime.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

extern void fs_module_init(void);
extern void fs_module_cleanup(void);
extern bool fs_step_CrossingController(void);
extern void fs_stop_CrossingController(void);
extern void *fs_get_machine_CrossingController(void);
extern bool fs_step_DisplayPanel(void);
extern void fs_stop_DisplayPanel(void);
extern void *fs_get_machine_DisplayPanel(void);

static uint64_t runner_now_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)ts.tv_nsec / 1000000ull;
}

static void runner_sleep_ms(uint64_t ms) {
  struct timespec ts;
  ts.tv_sec = (time_t)(ms / 1000ull);
  ts.tv_nsec = (long)((ms % 1000ull) * 1000000ull);
  nanosleep(&ts, NULL);
}

static int inject_initial_events(void) {
  {
    FS_MachineHeader *machine = (FS_MachineHeader *)fs_get_machine_CrossingController();
    FS_Event *event = (FS_Event *)calloc(1, sizeof(FS_Event));
    if (event == NULL) {
      return 1;
    }
    typedef struct RunnerPayload_CrossingController_PedButton_0 {
      int32_t sensor_id;
    } RunnerPayload_CrossingController_PedButton_0;
    RunnerPayload_CrossingController_PedButton_0 *payload = (RunnerPayload_CrossingController_PedButton_0 *)malloc(sizeof(RunnerPayload_CrossingController_PedButton_0));
    if (payload == NULL) {
      free(event);
      return 1;
    }
    payload->sensor_id = (int32_t)(101);
    event->flags = FS_EVENT_FREE_PAYLOAD;
    event->payload_size = (uint32_t)sizeof(RunnerPayload_CrossingController_PedButton_0);
    event->payload = payload;
    event->tag = (uint32_t)0;
    event->timestamp_ns = 0ull;
    fs_enqueue_event(machine, event);
  }
  return 0;
}

int main(void) {
  fs_module_init();
  if (inject_initial_events() != 0) {
    fs_module_cleanup();
    return 1;
  }
  const uint64_t start_ms = runner_now_ms();
  uint64_t last_progress_ms = start_ms;
  for (;;) {
    bool progressed = false;
    progressed = fs_step_CrossingController() || progressed;
    progressed = fs_step_DisplayPanel() || progressed;
    const uint64_t now_ms = runner_now_ms();
    if (progressed) {
      last_progress_ms = now_ms;
    } else {
      if (now_ms - last_progress_ms >= 50u) {
        break;
      }
      runner_sleep_ms(1u);
    }
    if (now_ms - start_ms >= 4000u) {
      break;
    }
  }
  fs_stop_CrossingController();
  fs_stop_DisplayPanel();
  fs_module_cleanup();
  return 0;
}
