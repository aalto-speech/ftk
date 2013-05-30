#include <cmath>
#include <fstream>
#include <sstream>

#include "Bigrams.hh"
#include "FactorEncoder.hh"

using namespace std;


void
Bigrams::update_trans_stats(const transitions_t &collected_stats,
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
Bigrams::collect_trans_stats(const transitions_t &transitions,
                             const map<string, flt_type> &words,
                             map<string, FactorGraph*> &fg_words,
                             transitions_t &trans_stats,
                             map<string, flt_type> &trans_normalizers,
                             map<string, flt_type> &unigram_stats,
                             bool fb)
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
Bigrams::collect_trans_stats(const transitions_t &transitions,
                             const map<string, flt_type> &words,
                             MultiStringFactorGraph &msfg,
                             transitions_t &trans_stats,
                             map<string, flt_type> &trans_normalizers,
                             map<string, flt_type> &unigram_stats,
                             bool fb)
{
    trans_stats.clear();
    trans_normalizers.clear();
    unigram_stats.clear();

    vector<flt_type> fw(msfg.nodes.size(), -std::numeric_limits<flt_type>::max());
    fw[0] = 0.0;

    forward(transitions, msfg, fw);
    for (auto it = msfg.string_end_nodes.begin(); it != msfg.string_end_nodes.end(); ++it) {
        transitions_t word_stats;
        backward(msfg, it->first, fw, word_stats);
        update_trans_stats(word_stats, words.at(it->first), trans_stats, trans_normalizers, unigram_stats);
    }
}


void
Bigrams::collect_trans_stats(const map<string, flt_type> &vocab,
                             const map<string, flt_type> &words,
                             MultiStringFactorGraph &msfg,
                             transitions_t &trans_stats,
                             map<string, flt_type> &trans_normalizers,
                             map<string, flt_type> &unigram_stats,
                             bool fb)
{
    trans_stats.clear();
    trans_normalizers.clear();
    unigram_stats.clear();

    vector<flt_type> fw(msfg.nodes.size(), -std::numeric_limits<flt_type>::max());
    fw[0] = 0.0;

    forward(vocab, msfg, fw);
    for (auto it = msfg.string_end_nodes.begin(); it != msfg.string_end_nodes.end(); ++it) {
        transitions_t word_stats;
        backward(msfg, it->first, fw, word_stats);
        update_trans_stats(word_stats, words.at(it->first), trans_stats, trans_normalizers, unigram_stats);
    }
}


void
Bigrams::normalize(transitions_t &trans_stats,
                   map<string, flt_type> &trans_normalizers,
                   flt_type min_cost)
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
Bigrams::write_transitions(const transitions_t &transitions,
                           const string &filename)
{
    ofstream transfile(filename);
    if (!transfile) return;

    for (auto srcit = transitions.cbegin(); srcit != transitions.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit)
            transfile << srcit->first << " " << tgtit->first << " " << tgtit->second << endl;

    transfile.close();
}


int
Bigrams::read_transitions(transitions_t &transitions,
                          const string &filename)
{
    ifstream transfile(filename);
    if (!transfile) return -1;

    string line;
    flt_type count;
    int num_trans = 0;
    while (getline(transfile, line)) {
        stringstream ss(line);
        string src, tgt;
        ss >> src;
        ss >> tgt;
        ss >> count;
        transitions[src][tgt] = count;
        num_trans++;
    }
    transfile.close();

    return num_trans;
}


flt_type
Bigrams::bigram_cost(const transitions_t &transitions,
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
Bigrams::cutoff(const map<string, flt_type> &unigram_stats,
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
Bigrams::cutoff(const map<string, flt_type> &unigram_stats,
                flt_type cutoff,
                transitions_t &transitions,
                MultiStringFactorGraph &msfg)
{
    vector<string> to_remove;
    for (auto it = unigram_stats.begin(); it != unigram_stats.end(); ++it)
        if (it->second < cutoff) to_remove.push_back(it->first);

    for (int i=0; i<to_remove.size(); i++)
        transitions.erase(to_remove[i]);

    for (auto srcit = transitions.begin(); srcit != transitions.end(); ++srcit)
        for (int i=0; i<to_remove.size(); i++)
            srcit->second.erase(to_remove[i]);

    for (int i=0; i<to_remove.size(); i++)
        msfg.remove_arcs(to_remove[i]);

    return to_remove.size();
}


int
Bigrams::remove_least_common(const map<string, flt_type> &unigram_stats,
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
Bigrams::remove_least_common(const map<string, flt_type> &unigram_stats,
                             int num_removals,
                             transitions_t &transitions,
                             MultiStringFactorGraph &msfg)
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

    for (int i=0; i<num_removals; i++)
        msfg.remove_arcs(sorted_stats[i].first);

    return start_size-transitions.size();
}


int
Bigrams::transition_count(const transitions_t &transitions)
{
    int count = 0;
    for (auto srcit = transitions.cbegin(); srcit != transitions.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit)
            count++;
    return count;
}

void
Bigrams::trans_to_vocab(const transitions_t &transitions,
                        map<string, flt_type> &vocab)
{
    vocab.clear();
    for (auto srcit = transitions.cbegin(); srcit != transitions.cend(); ++srcit)
        vocab[srcit->first] = 0.0;
}

