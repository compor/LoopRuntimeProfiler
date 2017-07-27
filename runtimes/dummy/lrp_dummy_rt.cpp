//
//
//

#include <time.h>
// using clock
// using CLOCKS_PER_SEC

#include <stdlib.h>
// using atexit
// using abort
// using getenv
// using atol

#include <stdio.h>
// using fprintf
// using fopen
// using fclose

#include <stdint.h>
// using uint32_t

bool lrp_ProfilingEnabled = true;

const char *lrp_ReportFilename = nullptr;
FILE *lrp_ReportFile = nullptr;

clock_t lrp_ProgramStart;
clock_t lrp_ProgramStop;

extern "C" {
void lrp_init(void);
void lrp_report(void);
void lrp_program_start(void);
void lrp_program_stop(void);
void lrp_loop_start(uint32_t id);
void lrp_loop_stop(uint32_t id);
} // extern "C"

void lrp_init(void) {
  const char *report = getenv("LRP_REPORT_FILE");
  if (report)
    lrp_ReportFilename = report;
  else
    lrp_ProfilingEnabled = false;

  if (!lrp_ProfilingEnabled)
    return;

  lrp_ReportFile = fopen(lrp_ReportFilename, "w");

  return;
}

void lrp_report(void) {
  if (!lrp_ProfilingEnabled)
    return;

  double duration =
      1000.0 * (lrp_ProgramStop - lrp_ProgramStart) / CLOCKS_PER_SEC;

  fprintf(lrp_ReportFile, "lrp runtime (ms): %f\n", duration);

  if (lrp_ReportFile)
    fclose(lrp_ReportFile);

  return;
}

void lrp_program_start(void) {
  if (!lrp_ProfilingEnabled)
    return;

  int rc = atexit(lrp_program_stop);

  if (rc) {
    fprintf(lrp_ReportFile, "could not set program exit handler!\n");

    abort();
  }

  fprintf(lrp_ReportFile, "lrp runtime start!\n");
  lrp_ProgramStart = clock();

  return;
}

void lrp_program_stop(void) {
  if (!lrp_ProfilingEnabled)
    return;

  fprintf(lrp_ReportFile, "lrp runtime stop!\n");
  lrp_ProgramStop = clock();

  lrp_report();

  return;
}

void lrp_loop_start(void) {
  if (!lrp_ProfilingEnabled)
    return;

  return;
}

void lrp_loop_stop(void) {
  if (!lrp_ProfilingEnabled)
    return;

  return;
}
