/*
 * Enumerate all combinations of m elements from a set of n elements.
 * For each combination, call visit(m, indexes, data) where
 * indexes is an m-length array of indexes into an array of n.
 *
 * For example, combos(6, 4, visit, data) might result in a series
 * of 15 calls like
 *
 *   visit(4, {0, 1, 2, 3}, data);
 *   visit(4, {0, 1, 2, 4}, data);
 *   visit(4, {0, 1, 2, 5}, data);
 *   visit(4, {0, 1, 3, 4}, data);
 *   visit(4, {0, 1, 3, 5}, data);
 *      ...
 *   visit(4, {2, 3, 4, 5}, data);
 */
void iter_combos(int n, int m, void (*visit)(int, int[], void *), void *data);
