#ifndef __UTILS_H_
#define __UTILS_H_

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))



#define min(a, b) _Generic((a), \
    long: __min_long, \
    int: __min_int, \
    default: __min_int \
)(a, b)

static int __min_int(int a, int b) {
	return (a < b) ? a : b;
}

static long __min_long(long a, long b) {
	return (a < b) ? a : b;
}

// We need to define these separateley, because otherwise we could have issue with 
#define max(a, b) _Generic((a), \
    long: __max_long, \
    int: __max_int, \
    default: __max_int \
)(a, b)

static int __max_int(int a, int b) {
	return (a > b) ? a : b;
}

static long __max_long(long a, long b) {
	return (a > b) ? a : b;
}

#define clamp(v, a, b) min(max(v, a), b)




#endif //__UTILS_H_