#include <stdbool.h>
#include <stdio.h>

#include "pid.h"

static int failures = 0;

static void check(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

static bool same_state(const Pid *a, const Pid *b)
{
    return a->kp == b->kp && a->ki == b->ki && a->kd == b->kd &&
           a->integral == b->integral && a->integral_limit == b->integral_limit &&
           a->output_limit == b->output_limit &&
           a->prev_measurement == b->prev_measurement && a->first == b->first;
}

int main(void)
{
    const float invalid_dt[] = {0.0f, -0.01f};
    Pid pid;
    Pid saved;
    Pid reference;

    for (unsigned history = 0; history < 2u; history++) {
        Pid_Init(&pid, 2.0f, 1.0f, 0.5f, 100.0f, 1000.0f);
        if (history != 0u) {
            (void)Pid_Update(&pid, 10.0f, 2.0f, 0.5f);
        }
        saved = pid;
        for (unsigned i = 0; i < sizeof(invalid_dt) / sizeof(invalid_dt[0]); i++) {
            check(Pid_Update(&pid, 100.0f, 50.0f, invalid_dt[i]) == 0.0f,
                  "nonpositive dt returns zero");
            check(same_state(&pid, &saved), "nonpositive dt preserves every PID field");
        }
        reference = saved;
        {
            float expected = Pid_Update(&reference, 10.0f, 3.0f, 0.5f);
            float actual = Pid_Update(&pid, 10.0f, 3.0f, 0.5f);
            check(actual == expected && same_state(&pid, &reference),
                  "valid dt resumes from unchanged history");
        }
    }

    if (failures != 0) {
        return 1;
    }
    printf("pid_invalid_dt: all checks passed\n");
    return 0;
}
