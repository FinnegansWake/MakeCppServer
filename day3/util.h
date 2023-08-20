#ifndef UTIL_H
#define UTIL_H
#include <cstdio>
#include <cstdlib>
void errif(bool condition, const char* error_msg);

static const int BUFFER_SIZE = 1024;
static const int MAX_EVENTS = 1024;
#endif