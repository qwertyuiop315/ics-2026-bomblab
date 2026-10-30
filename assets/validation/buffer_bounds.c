#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "defuser.h"

int main(void) {
    int (*phases[])(uint64_t, char *, size_t) = {
        solve_phase_1, solve_phase_2, solve_phase_3,
        solve_phase_4, solve_phase_5, solve_phase_6
    };
    uint64_t state = UINT64_C(0x20261014);
    unsigned tests = 0;
    for (unsigned sample = 0; sample < 80; ++sample) {
        state = state * UINT64_C(6364136223846793005) + 1;
        for (unsigned phase = 0; phase < 7; ++phase) {
            char reference[256];
            int status = phase < 6 ? phases[phase](state, reference, sizeof(reference))
                : solve_secret(state, ~state, reference, sizeof(reference));
            if (status) return 1;
            size_t required = strlen(reference) + 1;
            for (size_t cap = 0; cap <= 256; ++cap) {
                unsigned char guarded[258];
                memset(guarded, 0xa5, sizeof(guarded));
                char *out = (char *)guarded + 1;
                status = phase < 6 ? phases[phase](state, out, cap)
                    : solve_secret(state, ~state, out, cap);
                if ((status == 0) != (cap >= required)) return 2;
                if (!status && strcmp(out, reference)) return 3;
                if (guarded[0] != 0xa5) return 4;
                for (size_t i = cap + 1; i < sizeof(guarded); ++i)
                    if (guarded[i] != 0xa5) return 5;
                ++tests;
            }
        }
    }
    printf("PASS: %u buffer-boundary cases, cap 0..256, all 7 solvers; ASan/UBSan clean.\n", tests);
    return 0;
}
