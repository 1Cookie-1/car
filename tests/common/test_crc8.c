#include <stdint.h>
#include <stdio.h>

#include "crc8.h"

int main(void)
{
    static const uint8_t check_data[] = "123456789";
    uint8_t crc = Crc8_Compute(check_data, 9u);

    printf("Crc8_Compute(\"123456789\", 9) = 0x%02X\n", crc);

    if (crc != 0xF4u) {
        fprintf(stderr, "FAIL: expected 0xF4, got 0x%02X\n", crc);
        return 1;
    }

    printf("crc8: all checks passed\n");
    return 0;
}
