//
//
//

#include <map>
// using std::map

#include <stack>
// using std::stack

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

#include <assert.h>
// using assert

bool lrp_ProfilingEnabled = true;

const char *lrp_ReportFilename = nullptr;
FILE *lrp_ReportFile = nullptr;

using MSecs = std::chrono::duration<double, std::milli>;
using TP = std::chrono::time_point<std::chrono::system_clock>;

TP lrp_ProgramStart;
TP lrp_ProgramStop;

struct TimingEntry {
  TimingEntry() {}

  MSecs m_TotalDuration;
  TP m_LastEntered;
};

std::map<uint32_t, TimingEntry> LoopTimingEntries;
using CallStackTy = std::stack<uint32_t>;

CallStackTy lrp_CallStack;

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

  lrp_CallStack.push(0);

  lrp_ReportFile = fopen(lrp_ReportFilename, "w");

  return;
}

void lrp_report(void) {
  if (!lrp_ProfilingEnabled)
    return;

  auto duration = lrp_ProgramStop - lrp_ProgramStart;
  fprintf(lrp_ReportFile, "lrp runtime (ms): %lf\n", MSecs(duration).count());

  for (const auto &e : LoopTimingEntries) {
    double p = e.second.m_TotalDuration / duration * 100.0;

    fprintf(lrp_ReportFile, "%u %lf %lf\n", e.first,
            MSecs(e.second.m_TotalDuration).count(), p);
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

  if (lrp_CallStack.top() != id) {
    auto found = LoopTimingEntries.find(id);

    if (found == LoopTimingEntries.end())
      found = LoopTimingEntries.emplace(id, TimingEntry{}).first;

    // this assertion disallows nesting
    assert(!found->second.m_LastEntered.time_since_epoch().count() &&
           "Timing for section has already been started!");

    found->second.m_LastEntered = std::chrono::system_clock::now();
  }

  lrp_CallStack.push(id);

  return;
}

void lrp_loop_stop(uint32_t id) {
  if (!lrp_ProfilingEnabled)
    return;

  lrp_CallStack.pop();

  if (lrp_CallStack.top() != id) {
    auto found = LoopTimingEntries.find(id);

    assert(found != LoopTimingEntries.end() &&
           "Timing of section has not been started!");

    found->second.m_TotalDuration +=
        std::chrono::system_clock::now() - found->second.m_LastEntered;
    found->second.m_LastEntered = decltype(found->second.m_LastEntered){};
  }

  return;
}
