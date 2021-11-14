#pragma once

#define WRAP(a, b) ((b) + ((a)%(b))) % (b)
#define ARRAYLEN(x) (sizeof(x) / sizeof(x[0]))
