#ifndef CONFIG_H   // Include guard start
#define CONFIG_H

#include <cstdint>

// Should all be in seconds
#define CONFIG_PITCH_TIME_MAX 10 // 3 minutes
#define CONFIG_Q_AND_A_TIME_MAX 5 // 1 minute
#define CONFIG_TIME_TO_NEXT_TABLE_MAX 10 // 1 minute

#define CONFIG_FLIP_SCREEN_FOR_TIMER false

// assign 5 - 9 tables max per round
#define CONFIG_NUMBER_OF_TABLES 5

#define MAX_PROJECTS 9

int assignedtables[] = {1, 250, 3, 454, 5, 0, 0, 10, 10, 10};
bool assignedtablescheck[] = { true, false, true, true, false , false, false, false, false, false};
int numberoftables = CONFIG_NUMBER_OF_TABLES;

#define CONFIG_UUID_LENGTH 7


uint8_t assignedtablesUUID[MAX_PROJECTS][CONFIG_UUID_LENGTH] = {
    { 4, 122, 208, 5, 57, 108, 128},
    { 4, 4, 30, 73, 42, 116, 128},
    { 4, 116, 30, 154, 42, 116, 128},
    { 4, 0, 30, 0, 58, 116, 128},
    { 4, 0, 0, 0, 0, 0, 0},
    { 4, 0, 0, 0, 0, 0, 0},
    { 4, 0, 0, 0, 0, 0, 0},
    { 4, 0, 0, 0, 0, 0, 0},
    { 4, 0, 0, 0, 0, 0, 0}
};

#endif // CONFIG_H  // Include guard end