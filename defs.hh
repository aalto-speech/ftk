#ifndef PROJECT_DEFS
#define PROJECT_DEFS

#include <limits>

typedef double flt_type;
typedef unsigned short int fg_node_idx_t;
typedef unsigned int msfg_node_idx_t;
typedef char factor_pos_t;
typedef char factor_len_t;

static flt_type SMALL_LP = -100.0;
static flt_type MIN_FLOAT = -std::numeric_limits<flt_type>::max();

static int NUM_THREADS = 8;

#endif /* PROJECT_DEFS */
