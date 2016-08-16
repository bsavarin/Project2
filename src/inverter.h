// Provided courtesy of Max Baeumle (@maxbaeumle) on Slack pebbledev message, dated 20/12/2015, 5:44 pm
// https://slack-files.com/T04R13A5S-F0H2RGFSA-c7d5bee7df


#include <pebble.h>

struct InverterLayer;
typedef struct InverterLayer InverterLayer;

InverterLayer* inverter_layer_create(GRect frame);
void inverter_layer_destroy(InverterLayer* inverter_layer);
Layer* inverter_layer_get_layer(InverterLayer *inverter_layer);