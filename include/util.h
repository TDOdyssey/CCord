#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

long current_time_ms();

// A utility function to reverse a string
void reverse(char str[], int length);

// Implementation of itoa()
char* itoa(int num, char* str, int base);

#endif