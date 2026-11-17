#include "rng.h"

void rng_get(unsigned *out){
    RNG_SHORTS = 1;
    RNG_TASKS_START = 1;
    while(RNG_EVENTS_VALRDY == 0) { }
    RNG_EVENTS_VALRDY = 0;
    *out = RNG_VALUE;
}