//
//
//

#include <time.h>
// using clock
// using CLOCKS_PER_SEC

#include <stdlib.h>
// using atexit
// using abort

#include <stdio.h>
// using fprintf

#include <map>
// using std::map

#include <stdint.h>
// using uint32_t

#include <assert.h>
// using assert

struct TimingEntry {
  TimingEntry() : m_NumSections(0), m_TotalDuration(0), m_LastEntered(0) {}

  uint32_t m_NumSections;
  clock_t m_TotalDuration;
  clock_t m_LastEntered;
};

std::map<uint32_t, TimingEntry> LoopTimingEntries;

extern "C" {

clock_t lrp_program_start_timestamp;
clock_t lrp_program_stop_timestamp;

long int lrp_test_depth = -1;
long int lrp_current_depth = -1;

void lrp_report(void) {
  double duration = 1000.0 *
                    (lrp_program_stop_timestamp - lrp_program_start_timestamp) /
                    CLOCKS_PER_SEC;

  fprintf(stderr, "lrp runtime (ms): %f\n", duration);

  return;
}

void lrp_program_stop(void) {
  fprintf(stderr, "lrp runtime stop!\n");
  lrp_program_stop_timestamp = clock();

  lrp_report();

  return;
}

void lrp_program_start(void) {
  int rc = atexit(lrp_program_stop);

  if (rc) {
    fprintf(stderr, "could not set program exit handler!\n");

    abort();
  }

  fprintf(stderr, "lrp runtime start!\n");
  lrp_program_start_timestamp = clock();

  return;
}

void lrp_loop_start(uint32_t id) {
  ++lrp_current_depth;
  if (lrp_current_depth != lrp_test_depth)
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
  if (lrp_current_depth == lrp_test_depth) {
    auto found = LoopTimingEntries.find(id);

    assert(found != LoopTimingEntries.end() &&
           "Timing of section has not been started!");

    found->second.m_NumSections++;
    found->second.m_TotalDuration +=
        1000.0 * (clock() - found->second.m_LastEntered) / CLOCKS_PER_SEC;
    found->second.m_LastEntered = 0;
  }

  --lrp_current_depth;

  return;
}

} // extern "C"
