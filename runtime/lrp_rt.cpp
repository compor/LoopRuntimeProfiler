//
//
//

#include <map>
// using std::map

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

#include <assert.h>
// using assert

bool lrp_ProfilingEnabled = true;

const char *lrp_ReportFilename = nullptr;
FILE *lrp_ReportFile = nullptr;

clock_t lrp_ProgramStart;
clock_t lrp_ProgramStop;

long int lrp_TestDepth = -1;
long int lrp_CurrentDepth = -1;

struct TimingEntry {
  TimingEntry() : m_NumSections(0), m_TotalDuration(0), m_LastEntered(0) {}

  uint32_t m_NumSections;
  clock_t m_TotalDuration;
  clock_t m_LastEntered;
};

std::map<uint32_t, TimingEntry> LoopTimingEntries;

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

  const char *d = getenv("LRP_TEST_DEPTH");
  if (d)
    lrp_TestDepth = atol(d);

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

void lrp_program_stop(void) {
  if (!lrp_ProfilingEnabled)
    return;

  fprintf(lrp_ReportFile, "lrp runtime stop!\n");
  lrp_ProgramStop = clock();

  lrp_report();

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

void lrp_loop_start(uint32_t id) {
  if (!lrp_ProfilingEnabled)
    return;

  ++lrp_CurrentDepth;
  if (lrp_CurrentDepth != lrp_TestDepth)
    return;

  auto found = LoopTimingEntries.find(id);

  if (found == LoopTimingEntries.end())
    found = LoopTimingEntries.emplace(id, TimingEntry{}).first;

  // this assertion disallows nesting
  assert(found->second.m_LastEntered &&
         "Timing for section has already been started!");

  found->second.m_LastEntered = clock();

  return;
}

void lrp_loop_stop(uint32_t id) {
  if (!lrp_ProfilingEnabled)
    return;

  if (lrp_CurrentDepth == lrp_TestDepth) {
    auto found = LoopTimingEntries.find(id);

    assert(found != LoopTimingEntries.end() &&
           "Timing of section has not been started!");

    found->second.m_NumSections++;
    found->second.m_TotalDuration +=
        1000.0 * (clock() - found->second.m_LastEntered) / CLOCKS_PER_SEC;
    found->second.m_LastEntered = 0;
  }

  --lrp_CurrentDepth;

  return;
}
