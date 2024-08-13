#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "combos.h"

static int combos_6_4[16][4] = {
    {0, 1, 2, 3},
    {0, 1, 2, 4},
    {0, 1, 2, 5},
    {0, 1, 3, 4},
    {0, 1, 3, 5},
    {0, 1, 4, 5},
    {0, 2, 3, 4},
    {0, 2, 3, 5},
    {0, 2, 4, 5},
    {0, 3, 4, 5},
    {1, 2, 3, 4},
    {1, 2, 3, 5},
    {1, 2, 4, 5},
    {1, 3, 4, 5},
    {2, 3, 4, 5},
    {-1, -1, -1, -1},
};

static int combos_6_5[7][5] = {
    {0, 1, 2, 3, 4},
    {0, 1, 2, 3, 5},
    {0, 1, 2, 4, 5},
    {0, 1, 3, 4, 5},
    {0, 2, 3, 4, 5},
    {1, 2, 3, 4, 5},
    {-1, -1, -1, -1, -1},
};

static int combos_5_4[6][4] = {
    {0, 1, 2, 3},
    {0, 1, 2, 4},
    {0, 1, 3, 4},
    {0, 2, 3, 4},
    {1, 2, 3, 4},
    {-1, -1, -1, -1},
};

static int combos_5_3[11][3] = {
    {0, 1, 2},
    {0, 1, 3},
    {0, 1, 4},
    {0, 2, 3},
    {0, 2, 4},
    {0, 3, 4},
    {1, 2, 3},
    {1, 2, 4},
    {1, 3, 4},
    {2, 3, 4},
    {-1, -1, -1},
};

static int combos_5_2[11][2] = {
    {0, 1},
    {0, 2},
    {0, 3},
    {0, 4},
    {1, 2},
    {1, 3},
    {1, 4},
    {2, 3},
    {2, 4},
    {3, 4},
    {-1, -1},
};

static int combos_4_3[5][3] = {
    {0, 1, 2},
    {0, 1, 3},
    {0, 2, 3},
    {1, 2, 3},
    {-1, -1, -1},
};

static int combos_4_2[7][2] = {
    {0, 1},
    {0, 2},
    {0, 3},
    {1, 2},
    {1, 3},
    {2, 3},
    {-1, -1},
};

void iter_combos(int n, int m, void (*visit)(int, int[], void *), void *data) {
    if (n == m) {
        int combo[n];
        for (int i = 0; i < n; i++) {
            combo[i] = i;
        }
        visit(m, combo, data);
        return;
    }

    int *combos;
    if (n == 6 && m == 4) {
        combos = &(combos_6_4[0][0]);
    }
    else if (n == 6 && m == 5) {
        combos = &(combos_6_5[0][0]);
    }
    else if (n == 5 && m == 4) {
        combos = &(combos_5_4[0][0]);
    }
    else if (n == 5 && m == 3) {
        combos = &(combos_5_3[0][0]);
    }
    else if (n == 5 && m == 2) {
        combos = &(combos_5_2[0][0]);
    }
    else if (n == 4 && m == 3) {
        combos = &(combos_4_3[0][0]);
    }
    else if (n == 4 && m == 2) {
        combos = &(combos_4_2[0][0]);
    }
    else {
        fprintf(stderr, "unknown combo: n=%d, m=%d\n", n, m);
        abort();
    }

    /*
    int i = 0;
    combo = combos + (i * m);
    while (combo[0] != -1) {
        printf("combo %d: ", i);
        visit(m, combo, data);
        i++;
        combo = combos + (i * m);
    }
    */

    int *combo;
    for (int i = 0; combo = combos + (i * m), combo[0] != -1; i++) {
        visit(m, combo, data);
    }

}

typedef struct {
    FILE *outf;
} visit_t;

void visit(int m, int indexes[], void *data) {
    visit_t *visit_data = (visit_t *) data;
    FILE *outf = visit_data->outf;

    fprintf(outf, "[");
    int i;
    for (i = 0; i < m - 1; i++) {
        fprintf(outf, "%d, ", indexes[i]);
    }
    fprintf(outf, "%d]\n", indexes[i]);
}

int _main(int argc, char *argv[]) {
    visit_t visit_data = {stdout};
    printf("combos(6, 4):\n");
    iter_combos(6, 4, visit, &visit_data);
    printf("combos(6, 5):\n");
    iter_combos(6, 5, visit, &visit_data);

    printf("combos(4, 4):\n");
    iter_combos(4, 4, visit, &visit_data);
    printf("combos(5, 5):\n");
    iter_combos(5, 5, visit, &visit_data);

    return 0;
}
