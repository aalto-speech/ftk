#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "io.hh"
#include "Unigrams.hh"
#include "Bigrams.hh"

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
Bigrams::collect_trans_stats(transitions_t &transitions,
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


flt_type
Bigrams::collect_trans_stats(const map<string, flt_type> &words,
                             MultiStringFactorGraph &msfg,
                             transitions_t &trans_stats,
                             map<string, flt_type> &unigram_stats,
                             bool fb)
{
    trans_stats.clear();
    unigram_stats.clear();

    vector<flt_type> fw(msfg.nodes.size(), MIN_FLOAT);
    fw[0] = 0.0;

    forward(msfg, fw);
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

        bool renormalization_needed = false;
        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit) {
            tgtit->second = log(tgtit->second) - normalizer;
            if (tgtit->second < min_cost) {
                tgtit->second = min_cost;
                renormalization_needed = true;
            }
        }

        if (renormalization_needed) {
            normalizer = MIN_FLOAT;
            for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit)
                if (normalizer == MIN_FLOAT) normalizer = tgtit->second;
                else normalizer = add_log_domain_probs(normalizer, tgtit->second);
            for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit)
                tgtit->second -= normalizer;
        }
    }
}


// NOTE: assumes that tgt has the same elements as in src
void
Bigrams::copy_transitions(transitions_t &src,
                          transitions_t &tgt)
{
    for (auto trsrcit = tgt.begin(); trsrcit != tgt.end(); ++trsrcit)
        for (auto trtgtit = trsrcit->second.begin(); trtgtit != trsrcit->second.end(); ++trtgtit)
            trtgtit->second = src[trsrcit->first][trtgtit->first];
}


void
Bigrams::write_transitions(const transitions_t &transitions,
                           const string &filename,
                           bool count_style)
{
    io::Stream transfile(filename, "w");

    for (auto srcit = transitions.cbegin(); srcit != transitions.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit)
            if (count_style)
                fprintf(transfile.file, "%s %s\t%f\n", srcit->first.c_str(), tgtit->first.c_str(), tgtit->second);
            else
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

    for (auto srcit = transitions.begin(); srcit != transitions.end(); ++srcit) {
        flt_type normalizer = MIN_FLOAT;
        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit)
            if (normalizer == MIN_FLOAT) normalizer = tgtit->second;
            else normalizer = add_log_domain_probs(normalizer, tgtit->second);
        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit)
            tgtit->second -= normalizer;
    }
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
    Unigrams::sort_vocab(unigram_stats, sorted_stats, false);
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
    Unigrams::sort_vocab(unigram_stats, sorted_stats, false);
    vector<string> to_remove;
    for (auto it = sorted_stats.begin(); it != sorted_stats.end(); ++it) {
        if (it->first.length() < 3) continue;
        to_remove.push_back(it->first);
        if (to_remove.size() >= num_removals) break;
    }

    remove_transitions(to_remove, transitions);

    for (int i=0; i<to_remove.size(); i++)
        msfg.remove_arcs(to_remove[i]);

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


flt_type
Bigrams::score(const transitions_t &transitions,
               std::vector<std::string> &path)
{
    flt_type ll = 0.0;

    for (int i=0; i<path.size()-1; i++) {
        try {
            ll += transitions.at(path[i]).at(path[i+1]);
        }
        catch (std::out_of_range &oor) {
            ll += SMALL_LP;
        }
    }

    return ll;
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


flt_type
Bigrams::disable_string(const transitions_t &reverse_transitions,
                        const string &text,
                        const map<string, flt_type> &unigram_stats,
                        transitions_t &transitions,
                        transitions_t &changes)
{
    changes.clear();
    flt_type total_ll_diff = 0.0;

    for (auto it = transitions[text].begin(); it != transitions[text].begin(); ++it) {
        changes[text][it->first] = it->second;
        it->second = SMALL_LP;
    }

    for (auto contit = reverse_transitions.at(text).begin(); contit != reverse_transitions.at(text).end(); ++contit) {

        changes[contit->first][text] = transitions[contit->first][text];
        flt_type renormalizer = sub_log_domain_probs(0, transitions[contit->first][text]);
        flt_type ll_diff = 0.0;

        for (auto it = transitions[contit->first].begin(); it != transitions[contit->first].end(); ++it) {
            if (it->first != text) {
                flt_type count = unigram_stats.at(contit->first) * exp(it->second);
                changes[contit->first][it->first] = it->second;
                ll_diff -= count * it->second;
                it->second -= renormalizer;
                ll_diff += count * it->second;
            }
        }

        changes[contit->first][text] = transitions[contit->first][text];
        transitions[contit->first][text] = SMALL_LP;

        total_ll_diff += ll_diff;
    }

    return total_ll_diff;
}


flt_type
Bigrams::disable_transition(const map<string, flt_type> &unigram_stats,
                            const transitions_t &to_remove,
                            transitions_t &transitions,
                            transitions_t &changes)
{
    /*
    changes.clear();

    auto srcit = to_remove.cbegin();
    string src(srcit->first());
    auto tgtit = srcit->second->begin();
    string tgt = *(tgtit->first());


    changes[src][tgt] = transitions[src][tgt];
    flt_type renormalizer = sub_log_domain_probs(0, transitions[src[tgt]]);
    flt_type ll_diff = 0.0;
    transitions[src][tgt] = SMALL_LP;

    for (auto it = transitions[src].begin(); it != transitions[src].end(); ++it) {
        if (it->first != text) {
            flt_type count = unigram_stats.at(src) * exp(it->second);
            changes[src][it->first] = it->second;
            ll_diff -= count * it->second;
            it->second -= renormalizer;
            ll_diff += count * it->second;
        }
    }

    return ll_diff;
    */
}


void
Bigrams::restore_string(transitions_t &transitions,
                        const transitions_t &changes)
{
    for (auto srcit = changes.cbegin(); srcit != changes.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit)
            transitions[srcit->first][tgtit->first] = tgtit->second;
}


void
Bigrams::get_backpointers(const MultiStringFactorGraph &msfg,
                          map<string, set<string> > &backpointers,
                          unsigned int minlen)
{
    backpointers.clear();
    for (auto it = msfg.string_end_nodes.begin(); it != msfg.string_end_nodes.end(); ++it) {
        set<string> factors;
        msfg.collect_factors(it->first, factors);
        for (auto fit = factors.begin(); fit != factors.end(); ++fit) {
            if ((*fit).length() < minlen) continue;
            backpointers[*fit].insert(it->first);
        }
    }
}


int
Bigrams::init_removal_candidates(int n_candidates,
                                 const map<string, flt_type> &unigram_stats,
                                 map<string, flt_type> &candidates)
{
    vector<pair<string, flt_type> > sorted_stats;
    Unigrams::sort_vocab(unigram_stats, sorted_stats, false);

    for (auto it = sorted_stats.begin(); it != sorted_stats.end(); ++it) {
        if (it->first.length() < 2) continue;
        candidates[it->first] = 0.0;
        if (candidates.size() >= n_candidates) break;
    }

    return candidates.size();
}


void
Bigrams::rank_removal_candidates(const map<string, flt_type> &words,
                                 const MultiStringFactorGraph &msfg,
                                 const map<string, flt_type> &unigram_stats,
                                 transitions_t &transitions,
                                 map<std::string, flt_type> &candidates)
{
    map<string, set<string> > backpointers;
    Bigrams::get_backpointers(msfg, backpointers, 1);
    transitions_t reverse;
    Bigrams::reverse_transitions(transitions, reverse);

    for (auto it = candidates.begin(); it != candidates.end(); ++it) {
        transitions_t changes;
        set<string> words_to_resegment = backpointers.at(it->first);
        flt_type orig_score = likelihood(words, words_to_resegment, msfg);
        flt_type context_score = Bigrams::disable_string(reverse, it->first,
                                                         unigram_stats, transitions, changes);
        flt_type hypo_score = likelihood(words, words_to_resegment, msfg);
        it->second = hypo_score-orig_score + context_score;
        Bigrams::restore_string(transitions, changes);
    }
}
