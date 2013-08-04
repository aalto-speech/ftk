#ifndef PROJECT_DEFS
#define PROJECT_DEFS

#include <cmath>
#include <limits>
#include <map>
#include <string>

typedef double flt_type;
typedef unsigned short int fg_node_idx_t;
typedef unsigned int msfg_node_idx_t;
typedef unsigned char factor_pos_t;
typedef unsigned char factor_len_t;

typedef std::map<std::string, std::map<std::string, flt_type> > transitions_t;

static flt_type SMALL_LP = -25.0;
static flt_type MIN_FLOAT = -std::numeric_limits<flt_type>::max();

// Return log(X+Y) where a=log(X) b=log(Y)
static flt_type add_log_domain_probs(flt_type a, flt_type b) {

    if (b>a) {
        flt_type tmp = b;
        b = a;
        a = tmp;
    }

    return a + log(1 + exp(b - a));
}

// Return log(X-Y) where a=log(X) b=log(Y)
static flt_type sub_log_domain_probs(flt_type a, flt_type b) {

    if (b>a) {
        flt_type tmp = b;
        b = a;
        a = tmp;
    }

    return a + log(1 - exp(b - a));
}

#endif /* PROJECT_DEFS */
