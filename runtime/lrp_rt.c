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

static clock_t lrp_program_start_timestamp;
static clock_t lrp_program_stop_timestamp;

static void lrp_report(void) {
  double duration = 1000.0 *
                    (lrp_program_stop_timestamp - lrp_program_start_timestamp) /
                    CLOCKS_PER_SEC;

  fprintf(stderr, "lrp runtime (ms): %f\n", duration);

  return;
}

static void lrp_program_stop(void) {
  fprintf(stderr, "lrp runtime stop!\n");
  lrp_program_stop_timestamp = clock();

  lrp_report();

  return;
}

static void lrp_program_start(void) {
  int rc = atexit(lrp_program_stop);

  if (!rc) {
    fprintf(stderr, "could not set program exit handler!\n");

    abort();
  }

  fprintf(stderr, "lrp runtime start!\n");
  lrp_program_start_timestamp = clock();

  return;
}

static void lrp_loop_start(void) { return; }

static void lrp_loop_stop(void) { return; }
