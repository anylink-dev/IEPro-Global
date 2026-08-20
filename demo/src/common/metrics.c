#include "metrics.h"

#include "gpio_util.h"

#include <stdio.h>
#include <time.h>

int metrics_build_json(char *buf, size_t len)
{
    return snprintf(buf, len,
                    "{\"timestamp\":%ld,"
                    "\"di1\":%d,\"dip1\":%d,\"dip2\":%d}",
                    (long)time(NULL),
                    gpio_read_di(),
                    gpio_read_dip1(),
                    gpio_read_dip2());
}
