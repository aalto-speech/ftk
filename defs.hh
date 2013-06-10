#ifndef PROJECT_DEFS
#define PROJECT_DEFS

#include <limits>
#include <map>
#include <string>

typedef double flt_type;
typedef unsigned short int fg_node_idx_t;
typedef unsigned int msfg_node_idx_t;
typedef unsigned char factor_pos_t;
typedef unsigned char factor_len_t;

typedef std::map<std::string, std::map<std::string, flt_type> > transitions_t;

static flt_type SMALL_LP = -100.0;
static flt_type MIN_FLOAT = -std::numeric_limits<flt_type>::max();

static int NUM_THREADS = 4;

#endif /* PROJECT_DEFS */
