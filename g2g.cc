#include <algorithm>
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
update_trans_stats(const transitions_t &collected_stats,
                   flt_type weight,
                   transitions_t &trans_stats,
                   map<string, flt_type> &trans_normalizers,
                   map<string, flt_type> &unigram_stats)
{
    for (auto srcit = collected_stats.cbegin(); srcit != collected_stats.cend(); ++srcit) {
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit) {
            trans_stats[srcit->first][tgtit->first] += weight * tgtit->second;
            trans_normalizers[srcit->first] += weight * tgtit->second;
            unigram_stats[tgtit->first] += weight * tgtit->second;
        }
    }
}


void
collect_trans_stats(const transitions_t &transitions,
                    const map<string, flt_type> &words,
                    map<string, FactorGraph*> &fg_words,
                    transitions_t &trans_stats,
                    map<string, flt_type> &trans_normalizers,
                    map<string, flt_type> &unigram_stats,
                    bool fb=true)
{
    trans_stats.clear();
    trans_normalizers.clear();
    unigram_stats.clear();

    for (auto it = fg_words.begin(); it != fg_words.end(); ++it) {
        transitions_t word_stats;
        if (fb)
            forward_backward(transitions, *it->second, word_stats);
        else
            viterbi(transitions, *it->second, word_stats);
        update_trans_stats(word_stats, words.at(it->first), trans_stats, trans_normalizers, unigram_stats);
    }
}


void
normalize(transitions_t &trans_stats,
          map<string, flt_type> &trans_normalizers,
          flt_type min_cost = -200.0)
{
    for (auto srcit = trans_stats.begin(); srcit != trans_stats.end(); ++srcit) {
        auto tgtit = srcit->second.begin();
        while (tgtit != srcit->second.end()) {
            tgtit->second /= trans_normalizers[srcit->first];
            tgtit->second = log(tgtit->second);
            if (tgtit->second < min_cost || std::isnan(tgtit->second))
                srcit->second.erase(tgtit++);
            else
                ++tgtit;
        }
    }
}


void
write_transitions(const transitions_t &transitions,
                  const string &filename)
{
    ofstream transfile(filename);
    if (!transfile) return;

    for (auto srcit = transitions.cbegin(); srcit != transitions.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit)
            transfile << srcit->first << " " << tgtit->first << " " << tgtit->second << endl;

    transfile.close();
}


flt_type
bigram_cost(const transitions_t &transitions,
            const transitions_t &trans_stats)
{
    flt_type total = 0.0;
    flt_type tmp = 0.0;

    for (auto srcit = transitions.cbegin(); srcit != transitions.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit) {
            tmp = tgtit->second * trans_stats.at(srcit->first).at(tgtit->first);
            if (!std::isnan(tmp)) total += tmp;
        }

    return total;
}


int
cutoff(const map<string, flt_type> &unigram_stats,
       flt_type cutoff,
       transitions_t &transitions,
       map<string, FactorGraph*> &fg_words)
{
    vector<string> to_remove;
    for (auto it = unigram_stats.begin(); it != unigram_stats.end(); ++it)
        if (it->second < cutoff) to_remove.push_back(it->first);

    for (int i=0; i<to_remove.size(); i++)
        transitions.erase(to_remove[i]);

    for (auto srcit = transitions.begin(); srcit != transitions.end(); ++srcit)
        for (int i=0; i<to_remove.size(); i++)
            srcit->second.erase(to_remove[i]);

    for (auto fgit = fg_words.begin(); fgit != fg_words.end(); ++fgit) {
        for (int i=0; i<to_remove.size(); i++) {
            size_t found = fgit->first.find(to_remove[i]);
            if (found != std::string::npos) fgit->second->remove_arcs(to_remove[i]);
        }
    }

    return to_remove.size();
}


int
remove_least_common(const map<string, flt_type> &unigram_stats,
                    int num_removals,
                    transitions_t &transitions,
                    map<string, FactorGraph*> &fg_words)
{
    int start_size = transitions.size();
    vector<pair<string, flt_type> > sorted_stats;
    sort_vocab(unigram_stats, sorted_stats, false);

    for (int i=0; i<num_removals; i++)
        transitions.erase(sorted_stats[i].first);

    for (auto srcit = transitions.begin(); srcit != transitions.end(); ++srcit)
        for (int i=0; i<num_removals; i++)
            srcit->second.erase(sorted_stats[i].first);

    for (auto srcit = transitions.begin(); srcit != transitions.end();)
        if (srcit->second.size() == 0) transitions.erase(srcit++);
        else ++srcit;

    for (auto fgit = fg_words.begin(); fgit != fg_words.end(); ++fgit) {
        for (int i=0; i<num_removals; i++) {
            size_t found = fgit->first.find(sorted_stats[i].first);
            if (found != std::string::npos) fgit->second->remove_arcs(sorted_stats[i].first);
        }
    }

    return start_size-transitions.size();
}


int
transition_count(const transitions_t &transitions)
{
    int count = 0;
    for (auto srcit = transitions.cbegin(); srcit != transitions.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit)
            count++;
    return count;
}


int main(int argc, char* argv[]) {

    int iter_amount = 10;
    int least_common = 0;
    int target_vocab_size = 30000;
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
        {"least-common", 'l', POPT_ARG_INT, &least_common, 11001, NULL, "Remove least common strings"},
        {"vocab_size", 'g', POPT_ARG_INT, &target_vocab_size, 11007, NULL, "Target vocabulary size (stopping criterion)"},
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
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;

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
    transitions_t transitions;
    transitions_t trans_stats;
    map<string, flt_type> trans_normalizers;
    map<string, flt_type> unigram_stats;
    vocab[start_end_symbol] = 0.0;

    // Initial segmentation using unigram model
    for (auto it = words.cbegin(); it != words.cend(); ++it) {
        FactorGraph *fg = new FactorGraph(it->first, start_end_symbol, ss_vocab);
        fg_words[it->first] = fg;
        transitions_t word_stats;
        forward_backward(vocab, *fg, word_stats);
        update_trans_stats(word_stats, it->second, trans_stats, trans_normalizers, unigram_stats);
    }

    // Unigram cost with word end markers
    densum = gg.get_sum(unigram_stats);
    cost = gg.get_cost(unigram_stats, densum);
    cerr << "unigram cost with word end symbols: " << cost << endl;

    // Initial bigram cost
    transitions = trans_stats;
    normalize(transitions, trans_normalizers);
    cerr << "bigram cost: " << bigram_cost(transitions, trans_stats) << endl;
    cerr << "\tamount of transitions: " << transition_count(transitions) << endl;
    cerr << "\tvocab size: " << unigram_stats.size() << endl;
    int co_removed = cutoff(unigram_stats, cutoff_value, transitions, fg_words);
    cerr << "\tremoved by cutoff: " << co_removed << endl;

    // Re-estimate using bigram stats
    int vocab_size = unigram_stats.size();
    for (int i=0; i<iter_amount; i++) {
        if (vocab_size < 100000)
            collect_trans_stats(transitions, words, fg_words, trans_stats, trans_normalizers, unigram_stats, false);
        else
            collect_trans_stats(transitions, words, fg_words, trans_stats, trans_normalizers, unigram_stats, true);
        transitions = trans_stats;
        normalize(transitions, trans_normalizers);
        vocab_size = unigram_stats.size();
        cerr << "bigram cost: " << bigram_cost(transitions, trans_stats) << endl;
        cerr << "\tamount of transitions: " << transition_count(transitions) << endl;
        cerr << "\tvocab size: " << vocab_size << endl;

        // Write temp transitions
        ostringstream transitions_temp;
        transitions_temp << "transitions.iter" << i+1;
        write_transitions(transitions, transitions_temp.str());

        int curr_least_common = least_common + (vocab_size % 1000);
        int lc_removed = remove_least_common(unigram_stats, curr_least_common, transitions, fg_words);
        cerr << "\tremoved " << lc_removed << " least common subwords" << endl;
        vocab_size = transitions.size();

        if  (vocab_size < target_vocab_size) break;
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
