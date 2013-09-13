#ifndef PROJECT_DEFS
#define PROJECT_DEFS

#include <cmath>
#include <limits>
#include <map>
#include <string>

typedef double flt_type;
typedef unsigned short int fg_node_idx_t;
typedef unsigned int msfg_node_idx_t;
typedef unsigned short int factor_pos_t;
typedef unsigned char factor_len_t;

typedef std::map<std::string, std::map<std::string, flt_type> > transitions_t;

static flt_type FLOOR_LP = -15.0;
static flt_type SMALL_LP = -100.0;
static flt_type MIN_FLOAT = -std::numeric_limits<flt_type>::max();
static unsigned int MAX_LINE_LEN = 8192;
static std::string start_end_symbol("*");

// Return log(X+Y) where a=log(X) b=log(Y)
static flt_type add_log_domain_probs(flt_type a, flt_type b) {
    flt_type delta = a - b;
    if (delta > 0) {
      b = a;
      delta = -delta;
    }
    return b + log1p(exp(delta));
}

// Return log(X-Y) where a=log(X) b=log(Y)
static flt_type sub_log_domain_probs(flt_type a, flt_type b) {
    flt_type delta = b - a;
    if (delta > 0) {
      fprintf(stderr, "invalid call to sub_log_domain_probs, a should be bigger than b (a=%f,b=%f)\n",a,b);
      exit(1);
    }
    return a + log1p(-exp(delta));
}

static void trim(std::string &str, char to_trim)
{
    std::string::size_type pos = str.find_last_not_of(to_trim);
    if(pos != std::string::npos) {
        str.erase(pos + 1);
        pos = str.find_first_not_of(to_trim);
        if(pos != std::string::npos) str.erase(0, pos);
    }
    else str.erase(str.begin(), str.end());
}


#endif /* PROJECT_DEFS */

