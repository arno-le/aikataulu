#pragma once

#define KEY_TIME1 1
#define KEY_TIME2 3
#define KEY_CODE1 11
#define KEY_CODE2 12

#define KEY_LINE1 0
#define KEY_LINE2 2
#define KEY_TIME_TO_DEPARTURE 4
#define KEY_STOP1 5
#define KEY_STOP2 6

#define KEY_TWENTY_FOUR_HOUR_FORMAT 7
#define KEY_REFRESH_RATE 8

#define KEY_JS_READY 9
#define KEY_INTERNET 10


void reload_with_timeout();
void timeout_timer_handler(void *context);

