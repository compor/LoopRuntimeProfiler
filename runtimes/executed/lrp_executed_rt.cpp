//
//
//

#include <set>
// using std::set

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

std::set<uint32_t> lrp_LoopsExecuted;

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

  for (const auto &e : lrp_LoopsExecuted)
    fprintf(lrp_ReportFile, "%u\n", e);

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

  return;
}

void lrp_program_stop(void) {
  if (!lrp_ProfilingEnabled)
    return;

  fprintf(lrp_ReportFile, "lrp runtime stop!\n");

  lrp_report();

  return;
}

void lrp_loop_start(uint32_t id) { return; }

void lrp_loop_stop(uint32_t id) { return; }

void lrp_loop_body(uint32_t id) {
  if (!lrp_ProfilingEnabled)
    return;

  lrp_LoopsExecuted.insert(id);

  return;
}

