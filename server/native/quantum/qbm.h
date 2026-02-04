#ifndef HARMONY_QUANTUM_QBM_H
#define HARMONY_QUANTUM_QBM_H

#include <stdint.h>
#include <stddef.h>

// A "Quantum Bit" representing a probability distribution
// p0: Probability of being 0 (Negative/Neutral)
// p1: Probability of being 1 (Positive/Active)
typedef struct {
    float p0;
    float p1;
} qbit_t;

// Quantum Binary Matrix
// Rows: Entities (e.g., PIDs)
// Cols: Features (e.g., CPU Usage, I/O Wait, Error Rate)
typedef struct {
    int rows;
    int cols;
    qbit_t* data; // Flat array: index = row * cols + col
} qmatrix_t;

// API
void qbm_init(qmatrix_t* qm, int rows, int cols);
void qbm_free(qmatrix_t* qm);
void qbm_reset_row(qmatrix_t* qm, int row);

// Observation: Collapse the wavefunction to get a binary decision
// Returns 1 if p1 > random threshold, else 0
int qbm_observe(qmatrix_t* qm, int row, int col);

// Update: Adjust weights based on feedback
// feedback > 0: Strengthen p1
// feedback < 0: Strengthen p0
void qbm_update(qmatrix_t* qm, int row, int col, float feedback);

// Accessor
qbit_t* qbm_get(qmatrix_t* qm, int row, int col);

#endif // HARMONY_QUANTUM_QBM_H
