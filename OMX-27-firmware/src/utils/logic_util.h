#pragma once

#ifndef WRAP
#define WRAP(a, b) ((b) + ((a) % (b))) % (b)
#endif

#ifndef ARRAYLEN
#define ARRAYLEN(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef SGN
#define SGN(x) ((x) < 0 ? -1 : 1)
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(a, min, max) (MAX(MIN(a, max), min))
#endif
