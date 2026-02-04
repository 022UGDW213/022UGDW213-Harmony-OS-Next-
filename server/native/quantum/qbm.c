#include "qbm.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Simple pseudo-random for simulation
static float rand_float() {
    return (float)rand() / (float)RAND_MAX;
}

void qbm_init(qmatrix_t* qm, int rows, int cols) {
    qm->rows = rows;
    qm->cols = cols;
    qm->data = (qbit_t*)malloc(rows * cols * sizeof(qbit_t));
    
    // Initialize to superposition (neutral state)
    for (int i = 0; i < rows * cols; i++) {
        qm->data[i].p0 = 0.5f;
        qm->data[i].p1 = 0.5f;
    }
}

void qbm_free(qmatrix_t* qm) {
    if (qm->data) {
        free(qm->data);
        qm->data = NULL;
    }
}

void qbm_reset_row(qmatrix_t* qm, int row) {
    if (row < 0 || row >= qm->rows) return;
    for (int c = 0; c < qm->cols; c++) {
        int idx = row * qm->cols + c;
        qm->data[idx].p0 = 0.5f;
        qm->data[idx].p1 = 0.5f;
    }
}

qbit_t* qbm_get(qmatrix_t* qm, int row, int col) {
    if (row < 0 || row >= qm->rows || col < 0 || col >= qm->cols) return NULL;
    return &qm->data[row * qm->cols + col];
}

int qbm_observe(qmatrix_t* qm, int row, int col) {
    qbit_t* qb = qbm_get(qm, row, col);
    if (!qb) return 0;
    
    // Collapse
    float threshold = rand_float();
    return (threshold < qb->p1) ? 1 : 0;
}

void qbm_update(qmatrix_t* qm, int row, int col, float feedback) {
    qbit_t* qb = qbm_get(qm, row, col);
    if (!qb) return;
    
    // Simple learning rate
    float rate = 0.1f;
    
    if (feedback > 0) {
        // Boost p1
        qb->p1 += rate * feedback;
        if (qb->p1 > 1.0f) qb->p1 = 1.0f;
        qb->p0 = 1.0f - qb->p1;
    } else {
        // Boost p0 (Interpret negative feedback as "bad", so increase p0)
        // Or "decrease p1"
        qb->p1 += rate * feedback; // feedback is negative here
        if (qb->p1 < 0.0f) qb->p1 = 0.0f;
        qb->p0 = 1.0f - qb->p1;
    }
}
