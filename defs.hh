#ifndef PROJECT_DEFS
#define PROJECT_DEFS

#include <cmath>
#include <limits>
#include <map>
#include <string>
#include <vector>

typedef double flt_type;
typedef unsigned short int fg_node_idx_t;
typedef unsigned int msfg_node_idx_t;
typedef unsigned short int factor_pos_t;
typedef unsigned char factor_len_t;

typedef std::map<std::string, std::map<std::string, flt_type> > transitions_t;

#define FLOOR_LP -15.0
#define SMALL_LP -100.0
#define MIN_FLOAT -std::numeric_limits<flt_type>::max()
#define MAX_LINE_LEN 8192
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

static void get_character_positions_onebyte(std::string word,
                                            std::vector<unsigned int> &positions)
{
    positions.clear();
    for (unsigned int i=0; i<word.length()+1; i++)
        positions.push_back(i);
}

static void get_character_positions_utf8(std::string word,
                                         std::vector<unsigned int> &positions)
{
    positions.clear();
    positions.push_back(0);
    unsigned int charpos=0;

    while (charpos<word.length()) {
        unsigned int utfseqlen=0;
        if (!(word[charpos] & 128)) utfseqlen = 1;
        else if (word[charpos] & 192) utfseqlen = 2;
        else if (word[charpos] & 224) utfseqlen = 3;
        else utfseqlen = 4;
        charpos += utfseqlen;
        positions.push_back(charpos);
    }
}

static void get_character_positions(std::string word,
                                    std::vector<unsigned int> &positions,
                                    bool utf8=false)
{
    if (utf8)
        get_character_positions_utf8(word, positions);
    else
        get_character_positions_onebyte(word, positions);
}

#endif /* PROJECT_DEFS */

