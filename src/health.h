// From https://github.com/pebble-examples/simple-health-example
// MIT Licence Copyright (c) 2016 Pebble Examples
// https://github.com/pebble-examples/simple-health-example/blob/master/LICENSE

#pragma once

#include <pebble.h>

//#include "main_window.h"

void health_init();

bool health_is_available();

int health_get_metric_sum(HealthMetric metric);
