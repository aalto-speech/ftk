#include <cmath>
#include <fstream>
#include <sstream>
#include <thread>

#include "io.hh"
#include "Bigrams.hh"
#include "FactorEncoder.hh"

using namespace std;


void
Bigrams::update_trans_stats(const transitions_t &collected_stats,
                            flt_type weight,
                            transitions_t &trans_stats,
                            map<string, flt_type> &unigram_stats)
{
    for (auto srcit = collected_stats.cbegin(); srcit != collected_stats.cend(); ++srcit) {
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit) {
            trans_stats[srcit->first][tgtit->first] += weight * tgtit->second;
            unigram_stats[tgtit->first] += weight * tgtit->second;
        }
    }
}


void
Bigrams::collect_trans_stats(const transitions_t &transitions,
                             const map<string, flt_type> &words,
                             map<string, FactorGraph*> &fg_words,
                             transitions_t &trans_stats,
                             map<string, flt_type> &unigram_stats,
                             bool fb)
{
    trans_stats.clear();
    unigram_stats.clear();

    for (auto it = fg_words.begin(); it != fg_words.end(); ++it) {
        transitions_t word_stats;
        if (fb)
            forward_backward(transitions, *it->second, word_stats);
        else
            viterbi(transitions, *it->second, word_stats);
        update_trans_stats(word_stats, words.at(it->first), trans_stats, unigram_stats);
    }
}


void
Bigrams::threaded_backward(const MultiStringFactorGraph *msfg,
                           const std::vector<flt_type> *fw,
                           const map<string, flt_type> *words,
                           transitions_t *trans_stats,
                           int thread_index,
                           int thread_count,
                           flt_type *total_lp)
{
    *total_lp = 0.0;

    auto it = msfg->string_end_nodes.begin();
    for (int i=0; i<thread_index; i++)
        it++;

    while (true) {
        flt_type lp = backward(*msfg, it->first, *fw, *trans_stats, words->at(it->first));
        (*total_lp) += words->at(it->first) * lp;
        for (int i=0; i<thread_count; i++) {
            it++;
            if (it == msfg->string_end_nodes.end()) break;
        }
        if (it == msfg->string_end_nodes.end()) break;
    }
}


flt_type
Bigrams::collect_trans_stats(const transitions_t &transitions,
                             const map<string, flt_type> &words,
                             MultiStringFactorGraph &msfg,
                             transitions_t &trans_stats,
                             map<string, flt_type> &unigram_stats,
                             bool fb,
                             bool threaded)
{
    trans_stats.clear();
    unigram_stats.clear();

    vector<flt_type> fw(msfg.nodes.size(), MIN_FLOAT);
    fw[0] = 0.0;

    forward(transitions, msfg, fw);

    flt_type total_lp = 0.0;

    if (!threaded) {
        for (auto it = msfg.string_end_nodes.begin(); it != msfg.string_end_nodes.end(); ++it) {
            transitions_t word_stats;
            flt_type lp = backward(msfg, it->first, fw, word_stats);
            total_lp += words.at(it->first) * lp;
            update_trans_stats(word_stats, words.at(it->first), trans_stats, unigram_stats);
        }
        return total_lp;
    }

    vector<transitions_t> thread_stats(NUM_THREADS);
    vector<flt_type> total_lps(NUM_THREADS);
    thread thrs[NUM_THREADS];

    for (int i=0; i<NUM_THREADS; i++)
        thrs[i] = thread(threaded_backward, &msfg, &fw, &words, &(thread_stats[i]), i, NUM_THREADS, &(total_lps[i]));

    for (int i=0; i<NUM_THREADS; i++) {
        thrs[i].join();
        total_lp += total_lps[i];
        update_trans_stats(thread_stats[i], 1.0, trans_stats, unigram_stats);
    }

    return total_lp;
}


flt_type
Bigrams::collect_trans_stats(const map<string, flt_type> &vocab,
                             const map<string, flt_type> &words,
                             MultiStringFactorGraph &msfg,
                             transitions_t &trans_stats,
                             map<string, flt_type> &unigram_stats,
                             bool fb)
{
    trans_stats.clear();
    unigram_stats.clear();

    vector<flt_type> fw(msfg.nodes.size(), MIN_FLOAT);
    fw[0] = 0.0;

    forward(vocab, msfg, fw);
    flt_type total_lp = 0.0;

    transitions_t word_stats;
    for (auto it = msfg.string_end_nodes.begin(); it != msfg.string_end_nodes.end(); ++it) {
        total_lp += words.at(it->first) * backward(msfg, it->first, fw, word_stats, words.at(it->first));
    }
    update_trans_stats(word_stats, 1.0, trans_stats, unigram_stats);

    return total_lp;
}


void
Bigrams::normalize(transitions_t &trans_stats,
                   flt_type min_cost)
{
    for (auto srcit = trans_stats.begin(); srcit != trans_stats.end(); ++srcit) {

        flt_type normalizer = 0.0;
        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit)
            normalizer += tgtit->second;
        normalizer = log(normalizer);

        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit) {

            tgtit->second = log(tgtit->second) - normalizer;
            if (!std::isfinite(tgtit->second)) {
                cerr << "transition " << srcit->first << " " << tgtit->first << " value " << tgtit->second << endl;
                exit(0);
            }
            if (tgtit->second < min_cost) tgtit->second = min_cost;
        }
    }
}


void
Bigrams::write_transitions(const transitions_t &transitions,
                           const string &filename)
{
    io::Stream transfile(filename, "w");

    for (auto srcit = transitions.cbegin(); srcit != transitions.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit)
            fprintf(transfile.file, "%s %s %f\n", srcit->first.c_str(), tgtit->first.c_str(), tgtit->second);

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

    for (auto srcit = trans_stats.cbegin(); srcit != trans_stats.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit) {
            if (std::isfinite(tgtit->second))
                tmp = tgtit->second * transitions.at(srcit->first).at(tgtit->first);
            if (std::isfinite(tmp)) total += tmp;
            if (!std::isfinite(tmp)) cout << srcit->first << " " << tgtit->first << ": "
                    << tgtit->second << " " << transitions.at(srcit->first).at(tgtit->first) << endl;
        }

    return total;
}


void
Bigrams::remove_transitions(vector<string> &to_remove,
                            transitions_t &transitions)
{
    for (auto it = to_remove.begin(); it != to_remove.end(); ++it)
        transitions.erase(*it);

    for (auto srcit = transitions.begin(); srcit != transitions.end(); ++srcit)
        for (auto it = to_remove.begin(); it != to_remove.end(); ++it)
            srcit->second.erase(*it);

    for (auto srcit = transitions.begin(); srcit != transitions.end();)
        if (srcit->second.size() == 0) transitions.erase(srcit++);
        else ++srcit;
}


int
Bigrams::cutoff(const map<string, flt_type> &unigram_stats,
                flt_type cutoff,
                transitions_t &transitions,
                map<string, FactorGraph*> &fg_words)
{
    vector<string> to_remove;
    for (auto it = unigram_stats.begin(); it != unigram_stats.end(); ++it)
        if (it->second < cutoff && it->first.length() > 2) to_remove.push_back(it->first);

    remove_transitions(to_remove, transitions);

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
        if (it->second < cutoff && it->first.length() > 2) to_remove.push_back(it->first);

    remove_transitions(to_remove, transitions);

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
    vector<pair<string, flt_type> > sorted_stats;
    sort_vocab(unigram_stats, sorted_stats, false);
    vector<string> to_remove;
    for (auto it = sorted_stats.begin(); it != sorted_stats.end(); ++it) {
        if (it->first.length() < 3) continue;
        to_remove.push_back(it->first);
        if (to_remove.size() >= num_removals) break;
    }

    remove_transitions(to_remove, transitions);

    for (auto fgit = fg_words.begin(); fgit != fg_words.end(); ++fgit) {
        for (int i=0; i<num_removals; i++) {
            size_t found = fgit->first.find(sorted_stats[i].first);
            if (found != std::string::npos) fgit->second->remove_arcs(sorted_stats[i].first);
        }
    }

    return to_remove.size();
}


int
Bigrams::remove_least_common(const map<string, flt_type> &unigram_stats,
                             int num_removals,
                             transitions_t &transitions,
                             MultiStringFactorGraph &msfg)
{
    vector<pair<string, flt_type> > sorted_stats;
    sort_vocab(unigram_stats, sorted_stats, false);
    vector<string> to_remove;
    for (auto it = sorted_stats.begin(); it != sorted_stats.end(); ++it) {
        if (it->first.length() < 3) continue;
        to_remove.push_back(it->first);
        if (to_remove.size() >= num_removals) break;
    }

    remove_transitions(to_remove, transitions);

    for (int i=0; i<to_remove.size(); i++)
        msfg.remove_arcs(to_remove[i]);
    msfg.prune_unreachable();

    return to_remove.size();
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


void
Bigrams::reverse_transitions(const transitions_t &transitions,
                             transitions_t &reverse_transitions)
{
    reverse_transitions.clear();
    for (auto srcit = transitions.cbegin(); srcit != transitions.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit)
            reverse_transitions[tgtit->first][srcit->first] = tgtit->second;
}


void
Bigrams::remove_string(const transitions_t &reverse_transitions,
                       const std::string &text,
                       transitions_t &transitions)
{
    for (auto it = transitions[text].begin(); it != transitions[text].begin(); ++it)
        it->second = SMALL_LP;

    // FIXME: is there a better way to calculate the normalizer
    for (auto contit = reverse_transitions.at(text).begin(); contit != reverse_transitions.at(text).end(); ++contit) {
        flt_type normalizer = MIN_FLOAT;
        for (auto it = transitions[contit->first].begin(); it != transitions[contit->first].end(); ++it) {
            if (it->first == text) { it->second = SMALL_LP; continue; }
            if (normalizer == MIN_FLOAT) normalizer = it->second;
            else normalizer = add_log_domain_probs(normalizer, it->second);
        }
        for (auto it = transitions[contit->first].begin(); it != transitions[contit->first].end(); ++it)
            it->second -= normalizer;
    }
}

