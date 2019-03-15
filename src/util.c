#include "bootpack.h"

int min(int a, int b) { return a > b ? b : a; }

int max(int a, int b) { return a < b ? b : a; }

int clamp(int x, int a, int b) { return min(max(x, a), b); }
