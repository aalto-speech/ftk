#ifndef PROJECT_DEFS
#define PROJECT_DEFS

#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

typedef double flt_type;
typedef unsigned short int fg_node_idx_t;
typedef unsigned int msfg_node_idx_t;
typedef unsigned short int factor_pos_t;
typedef unsigned char factor_len_t;

typedef std::map<std::string, std::map<std::string, flt_type> > transitions_t;

#define FLOOR_LP -20.0
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

static void get_character_positions_utf8(const std::string &word,
                                         std::vector<unsigned int> &positions)
{
    positions.clear();
    positions.push_back(0);
    unsigned int charpos=0;

    while (charpos<word.length()) {
        unsigned int utfseqlen=0;
        if (!(word[charpos] & 128)) utfseqlen = 1;
        else if ((word[charpos] & 240) == 240) utfseqlen = 4;
        else if ((word[charpos] & 224) == 224) utfseqlen = 3;
        else utfseqlen = 2;
        charpos += utfseqlen;
        positions.push_back(charpos);
    }
}

static void get_character_positions(const std::string &word,
                                    std::vector<unsigned int> &positions,
                                    bool utf8=false)
{
    if (utf8)
        get_character_positions_utf8(word, positions);
    else
        get_character_positions_onebyte(word, positions);
}

static void find_short_factors(const std::map<std::string, flt_type> &vocab,
                               std::set<std::string> &short_factors,
                               unsigned int min_removal_length,
                               bool utf8)
{
    for (auto it = vocab.cbegin(); it != vocab.end(); ++it) {
        std::vector<unsigned int> char_positions;
        get_character_positions(it->first, char_positions, utf8);
        if (char_positions.size()-1 < min_removal_length)
            short_factors.insert(it->first);
    }
}

static void assert_factors(std::map<std::string, flt_type> &vocab,
                                 const std::set<std::string> &factors,
                                 flt_type min_lp)
{
    for (auto it = factors.cbegin(); it != factors.cend(); ++it)
        if (vocab.find(*it) == vocab.end())
            vocab[*it] = min_lp;
}


static unsigned int get_factor_length_utf8(const std::string &factor)
{
    unsigned int length = 0;

    unsigned int charpos=0;
    while (charpos<factor.length()) {
        unsigned int utfseqlen=0;
        if (!(factor[charpos] & 128)) utfseqlen = 1;
        else if ((factor[charpos] & 240) == 240) utfseqlen = 4;
        else if ((factor[charpos] & 224) == 224) utfseqlen = 3;
        else utfseqlen = 2;
        charpos += utfseqlen;
        length++;
    }

    return length;
}


static unsigned int get_factor_length(const std::string &factor, bool utf8)
{
    if (utf8) return get_factor_length_utf8(factor);
    else return factor.length();
}


#endif /* PROJECT_DEFS */

