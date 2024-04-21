#ifndef _TWIDDLE_H
#define _TWIDDLE_H

void iter_combos_bitwise(int N, int M, void (*visit)(int, int[]));
void iter_combos(int N, int M, void (*visit)(int, int[], void *), void *);

#endif
