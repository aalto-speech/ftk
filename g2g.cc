#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <popt.h>

#include "defs.hh"
#include "StringSet.hh"
#include "FactorEncoder.hh"
#include "GreedyUnigrams.hh"

using namespace std;


void
assert_single_chars(map<string, flt_type> &vocab,
                    const map<string, flt_type> &chars,
                    flt_type val)
{
    for (auto it = chars.cbegin(); it != chars.cend(); ++it)
        if (vocab.find(it->first) == vocab.end())
            vocab[it->first] = val;
}


void
collect_trans_stats(const map<pair<string,string>, flt_type> &transitions,
                    const map<string, flt_type> &words,
                    map<string, FactorGraph*> &fg_words,
                    map<pair<string,string>, flt_type> &trans_stats,
                    map<string, flt_type> &trans_normalizers,
                    map<string, flt_type> &unigram_stats)
{
    trans_stats.clear();
    trans_normalizers.clear();
    unigram_stats.clear();

    for (auto it = fg_words.begin(); it != fg_words.end(); ++it) {
        map<pair<string,string>, flt_type> stats;
        forward_backward(transitions, *it->second, stats);
        for (auto statit = stats.cbegin(); statit != stats.cend(); ++statit) {
            trans_stats[statit->first] += words.at(it->first) * statit->second;
            trans_normalizers[statit->first.first] += words.at(it->first) * statit->second;
            unigram_stats[statit->first.second] += words.at(it->first) * statit->second;
        }
    }
}


void
normalize(map<pair<string,string>, flt_type> &trans_stats,
          map<string, flt_type> &trans_normalizers,
          flt_type min_cost = -200.0)
{
    auto iter = trans_stats.begin();
    while (iter != trans_stats.end()) {
        iter->second /= trans_normalizers[iter->first.first];
        iter->second = log(iter->second);
        if (iter->second < min_cost || std::isnan(iter->second))
            trans_stats.erase(iter++);
        else
            ++iter;
    }
}


void
write_transitions(const map<pair<string,string>, flt_type> &transitions,
                  const string &filename)
{
    ofstream transfile(filename);
    if (!transfile) return;

    for (auto it = transitions.begin(); it != transitions.end(); ++it)
        transfile << it->first.first << " " << it->first.second << " " << it->second << endl;

    transfile.close();
}


flt_type
bigram_cost(const map<pair<string,string>, flt_type> &transitions,
            const map<pair<string,string>, flt_type> &trans_stats)
{
    flt_type total = 0.0;
    flt_type tmp = 0.0;
    for (auto iter = transitions.cbegin(); iter != transitions.cend(); ++iter) {
        tmp = iter->second * trans_stats.at(iter->first);
        if (!std::isnan(tmp)) total += tmp;
    }
    return total;
}


int
vocab_size(const map<pair<string,string>, flt_type> &transitions)
{
    map<string, flt_type> vocab;
    for (auto iter = transitions.cbegin(); iter != transitions.cend(); ++iter)
        vocab[iter->first.first] = 0.0;
    return vocab.size();
}


int
cutoff(const map<string, flt_type> unigram_stats,
       flt_type cutoff,
       map<pair<string,string>, flt_type> &transitions,
       map<string, FactorGraph*> fg_words)
{
    map<string, flt_type> to_remove;
    for (auto it = unigram_stats.begin(); it != unigram_stats.end(); ++it)
        if (it->second < cutoff) to_remove[it->first] = it->second;

    for (auto it = to_remove.begin(); it != to_remove.end(); ++it) {
        for (auto trans_it = transitions.begin(); trans_it != transitions.end();) {
            if (trans_it->first.first == it->first || trans_it->first.second == it->first)
                transitions.erase(trans_it++);
            else
                ++trans_it;
        }

        for (auto fgit = fg_words.begin(); fgit != fg_words.end(); ++fgit) {
            size_t found = fgit->first.find(it->first);
            if (found != std::string::npos) fgit->second->remove_arcs(it->first);
        }
    }

    return to_remove.size();
}


int main(int argc, char* argv[]) {

    int iter_amount = 10;
    float cutoff_value = 0.0;
    flt_type one_char_min_lp = -25.0;
    string vocab_fname;
    string wordlist_fname;
    string transition_fname;
    string wordseg_fname;

    // Popt documentation:
    // http://linux.die.net/man/3/popt
    // http://privatemisc.blogspot.fi/2012/12/popt-basic-example.html
    poptContext pc;
    struct poptOption po[] = {
        {"iter", 'i', POPT_ARG_INT, &iter_amount, 11001, NULL, "How many iterations"},
        {"cutoff", 'u', POPT_ARG_FLOAT, &cutoff_value, 11001, NULL, "Cutoff value for each iteration"},
        POPT_AUTOHELP
        {NULL}
    };

    pc = poptGetContext(NULL, argc, (const char **)argv, po, 0);
    poptSetOtherOptionHelp(pc, "[INITIAL VOCABULARY] [WORDLIST] [TRANSITIONS] [WORD_SEGS]");

    int val;
    while ((val = poptGetNextOpt(pc)) >= 0)
        continue;

    // poptGetNextOpt returns -1 when the final argument has been parsed
    // otherwise an error occured
    if (val != -1) {
        switch (val) {
        case POPT_ERROR_NOARG:
            cerr << "Argument missing for an option" << endl;
            exit(1);
        case POPT_ERROR_BADOPT:
            cerr << "Option's argument could not be parsed" << endl;
            exit(1);
        case POPT_ERROR_BADNUMBER:
        case POPT_ERROR_OVERFLOW:
            cerr << "Option could not be converted to number" << endl;
            exit(1);
        default:
            cerr << "Unknown error in option processing" << endl;
            exit(1);
        }
    }

    // Handle ARG part of command line
    if (poptPeekArg(pc) != NULL)
        vocab_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Initial vocabulary file not set" << endl;
        exit(1);
    }

    if (poptPeekArg(pc) != NULL)
        wordlist_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Wordlist file not set" << endl;
        exit(1);
    }

    if (poptPeekArg(pc) != NULL)
        transition_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Transition file not set" << endl;
        exit(1);
    }

    if (poptPeekArg(pc) != NULL)
        wordseg_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Word segmentation file not set" << endl;
        exit(1);
    }

    cerr << "parameters, initial vocabulary: " << vocab_fname << endl;
    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, transitions: " << transition_fname << endl;
    cerr << "parameters, word segmentations: " << wordseg_fname << endl;
    cerr << "parameters, cutoff: " << setprecision(15) << cutoff_value << endl;
    cerr << "parameters, iterations: " << iter_amount << endl;

    int maxlen, word_maxlen;
    map<string, flt_type> all_chars;
    map<string, flt_type> vocab;
    map<string, flt_type> freqs;
    map<string, flt_type> words;

    cerr << "Reading vocabulary " << vocab_fname << endl;
    int retval = read_vocab(vocab_fname, vocab, maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(0);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;
    for (auto it = vocab.cbegin(); it != vocab.end(); ++it)
        if (it->first.length() == 1)
            all_chars[it->first] = 0.0;

    cerr << "Reading word list " << wordlist_fname << endl;
    retval = read_vocab(wordlist_fname, words, word_maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading word list" << endl;
        exit(0);
    }
    cerr << "\t" << "wordlist size: " << words.size() << endl;
    cerr << "\t" << "maximum word length: " << word_maxlen << endl;

    GreedyUnigrams gg;
    gg.set_segmentation_method(forward_backward);

    cerr << "Initial cutoff" << endl;
    gg.resegment_words(words, vocab, freqs);
    flt_type densum = gg.get_sum(freqs);
    flt_type cost = gg.get_cost(freqs, densum);
    cerr << "unigram cost without word end symbols: " << cost << endl;

    gg.cutoff(freqs, (flt_type)cutoff_value);
    cerr << "\tcutoff: " << cutoff_value << "\t" << "vocabulary size: " << freqs.size() << endl;
    vocab = freqs;
    densum = gg.get_sum(vocab);
    gg.freqs_to_logprobs(vocab, densum);
    assert_single_chars(vocab, all_chars, one_char_min_lp);

    StringSet<flt_type> ss_vocab(vocab);
    map<string, FactorGraph*> fg_words;
    string start_end_symbol("*");
    map<pair<string,string>, flt_type> transitions;
    map<pair<string,string>, flt_type> trans_stats;
    map<string, flt_type> trans_normalizers;
    map<string, flt_type> unigram_stats;
    vocab[start_end_symbol] = 0.0;

    // Initial segmentation using unigram model
    for (auto it = words.cbegin(); it != words.cend(); ++it) {
        FactorGraph *fg = new FactorGraph(it->first, start_end_symbol, ss_vocab);
        fg_words[it->first] = fg;
        map<pair<string,string>, flt_type> stats;
        forward_backward(vocab, *fg, stats);
        for (auto statit = stats.cbegin(); statit != stats.cend(); ++statit) {
            trans_stats[statit->first] += it->second * statit->second;
            trans_normalizers[statit->first.first] += it->second * statit->second;
            unigram_stats[statit->first.second] += it->second * statit->second;
        }
    }

    // Unigram cost with word end markers
    densum = gg.get_sum(unigram_stats);
    cost = gg.get_cost(unigram_stats, densum);
    cerr << "unigram cost with word end symbols: " << cost << endl;

    // Initial bigram cost
    transitions = trans_stats;
    normalize(transitions, trans_normalizers);
    cerr << "bigram cost: " << bigram_cost(transitions, trans_stats) << endl;
    cerr << "\tamount of transitions: " << transitions.size() << endl;
    cerr << "\tvocab size: " << unigram_stats.size() << endl;
    int co_removed = cutoff(unigram_stats, cutoff_value, transitions, fg_words);
    cerr << "\tremoved by cutoff: " << co_removed << endl;

    // Re-estimate using bigram stats
    for (int i=0; i<iter_amount; i++) {
        collect_trans_stats(transitions, words, fg_words, trans_stats, trans_normalizers, unigram_stats);
        transitions = trans_stats;
        normalize(transitions, trans_normalizers);
        cerr << "bigram cost: " << bigram_cost(transitions, trans_stats) << endl;
        cerr << "\tamount of transitions: " << transitions.size() << endl;
        cerr << "\tvocab size: " << unigram_stats.size() << endl;
        co_removed = cutoff(unigram_stats, cutoff_value, transitions, fg_words);
        cerr << "\tremoved by cutoff: " << co_removed << endl;
    }

    // Write transitions
    write_transitions(transitions, transition_fname);
    write_transitions(trans_stats, "trans_stats.out");
    write_vocab("unigram.out", unigram_stats);

    // Viterbi segmentations
    ofstream wsegfile(wordseg_fname);
    if (!wsegfile) exit(0);
    for (auto it = fg_words.begin(); it != fg_words.cend(); ++it) {
        std::vector<std::string> best_path;
        viterbi(transitions, *it->second, best_path);
        for (int i=0; i<best_path.size(); i++) {
            wsegfile << best_path[i];
            if (i<best_path.size()-1) wsegfile << " ";
        }
        wsegfile << endl;
    }
    wsegfile.close();

    // Clean up
    for (auto it = fg_words.begin(); it != fg_words.cend(); ++it)
        delete it->second;

    exit(1);
}

