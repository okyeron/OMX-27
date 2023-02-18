#pragma once

#define WRAP(a, b) ((b) + ((a)%(b))) % (b)
#define ARRAYLEN(x) (sizeof(x) / sizeof(x[0]))
#define SGN(x) ((x) < 0 ? -1 : 1)
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLAMP(a,min,max) (MAX(MIN(a,max),min))