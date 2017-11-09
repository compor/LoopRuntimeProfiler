//
//
//

#include <map>
// using std::map

#include <set>
// using std::set

#include <chrono>
// using std::chrono

#include <ratio>
// using std::milli

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

using MSecs = std::chrono::duration<double, std::milli>;
using TP = std::chrono::time_point<std::chrono::system_clock>;

TP lrp_ProgramStart;
TP lrp_ProgramStop;

long unsigned lrp_MaxDepth = 0;
long unsigned lrp_CurrentDepth = 0;

std::map<uint32_t, std::set<long unsigned>> lrp_LoopDepths;

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

  auto duration = lrp_ProgramStop - lrp_ProgramStart;

  fprintf(lrp_ReportFile, "lrp runtime (ms): %lf\n", MSecs(duration).count());
  fprintf(lrp_ReportFile, "max depth %lu\n", lrp_MaxDepth);

  for (const auto &e : lrp_LoopDepths) {
    fprintf(lrp_ReportFile, "%u ", e.first);

    for(const auto &k : e.second)
      fprintf(lrp_ReportFile, "%lu ", k);

    fprintf(lrp_ReportFile, "\n");
  }

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
  lrp_ProgramStart = std::chrono::system_clock::now();

  return;
}

void lrp_program_stop(void) {
  if (!lrp_ProfilingEnabled)
    return;

  fprintf(lrp_ReportFile, "lrp runtime stop!\n");
  lrp_ProgramStop = std::chrono::system_clock::now();

  lrp_report();

  return;
}

void lrp_loop_start(uint32_t id) {
  if (!lrp_ProfilingEnabled)
    return;

  if (lrp_LoopDepths.find(id) == lrp_LoopDepths.end())
    lrp_LoopDepths[id] = {lrp_CurrentDepth};
  else
    lrp_LoopDepths[id].insert(lrp_CurrentDepth);

  ++lrp_CurrentDepth;

  if (lrp_CurrentDepth > lrp_MaxDepth)
    lrp_MaxDepth = lrp_CurrentDepth;

  return;
}

void lrp_loop_stop(uint32_t id) {
  if (!lrp_ProfilingEnabled)
    return;

  --lrp_CurrentDepth;

  return;
}
