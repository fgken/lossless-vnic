#include <stdio.h>

#define output(fmt, ...) printf("%s : " fmt "\n", __FUNCTION__, ##__VA_ARGS__)
