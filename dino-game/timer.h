/*
   timer.h
   Types related to timing.
*/

// Interval at which the driver timer is invoked to move everything along (ms)
#define DRIVER_INTERVAL 50

// Identifies a job which requires regularly scheduled execution.
typedef enum {
  MOVE_OBSTACLES,
  SPEED_UP_OBSTACLES,
  MAYBE_DIRECTION_CHANGE,
} job_id_t;

typedef struct {
  // Unique ID for this job.
  job_id_t id;
  // The function which causes the job to actually happen.
  void (*handler)(void);
  // Multiple of the DRIVER_INTERVAL at which this job should run.
  size_t interval_multiple;
} job_t;

// Global list of all active jobs.
LinkedPointerList<job_t> *all_jobs;
