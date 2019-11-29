//
//
//

#include <map>
// using std::map

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
// using uint64_t

bool lrp_ProfilingEnabled = true;

const char *lrp_ReportFilename = nullptr;
FILE *lrp_ReportFile = nullptr;

using TripCountTy = uint64_t;

std::set<uint32_t> lrp_Normal;
std::set<uint32_t> lrp_Degenerate;
std::map<uint32_t, TripCountTy> lrp_LoopTripCounts;

extern "C" {
void lrp_init(void);
void lrp_report(void);
void lrp_program_start(void);
void lrp_program_stop(void);
void lrp_loop_start(uint32_t id);
void lrp_loop_stop(uint32_t id);
void lrp_loop_latch(uint32_t id);
} // extern "C"

void lrp_init(void) {
  const char *report = getenv("LRP_REPORT_FILE");
  if (report) {
    lrp_ReportFilename = report;
  } else {
    lrp_ProfilingEnabled = false;
  }

  if (!lrp_ProfilingEnabled) {
    return;
  }

  lrp_ReportFile = fopen(lrp_ReportFilename, "w");
}

void lrp_report(void) {
  if (!lrp_ProfilingEnabled) {
    return;
  }

  for (const auto &e : lrp_Degenerate) {
    fprintf(lrp_ReportFile, "%u ", e);

    fprintf(lrp_ReportFile, "\n");
  }

  if (lrp_ReportFile) {
    fclose(lrp_ReportFile);
  }
}

void lrp_program_start(void) {
  if (!lrp_ProfilingEnabled) {
    return;
  }

  int rc = atexit(lrp_program_stop);

  if (rc) {
    fprintf(lrp_ReportFile, "could not set program exit handler!\n");

    abort();
  }

  fprintf(lrp_ReportFile, "lrp runtime start!\n");
}

void lrp_program_stop(void) {
  if (!lrp_ProfilingEnabled) {
    return;
  }

  fprintf(lrp_ReportFile, "lrp runtime stop!\n");

  lrp_report();
}

void lrp_loop_start(uint32_t id) {
  if (!lrp_ProfilingEnabled) {
    return;
  }

  lrp_LoopTripCounts[id] = 0;

  if (!lrp_Normal.count(id)) {
    lrp_Degenerate.insert(id);
  }
}

void lrp_loop_stop(uint32_t id) {
  if (!lrp_ProfilingEnabled) {
    return;
  }

  if (!lrp_Normal.count(id) && lrp_LoopTripCounts[id] > 2) {
    lrp_Normal.insert(id);
    lrp_Degenerate.erase(id);
  }

  lrp_LoopTripCounts[id] = 0;
}

void lrp_loop_latch(uint32_t id) {
  if (!lrp_ProfilingEnabled) {
    return;
  }

  lrp_LoopTripCounts[id]++;
}
