#include "util.h"
void errif(bool condition, const char* error_msg) {
    if (condition) {
        perror(error_msg);
        exit(EXIT_FAILURE);
    }
}