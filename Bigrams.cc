#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "io.hh"
#include "Unigrams.hh"
#include "Bigrams.hh"

using namespace std;


flt_type
Bigrams::iterate(const map<string, flt_type> &words,
                 MultiStringFactorGraph &msfg,
                 transitions_t &transitions,
                 bool forward_backward,
                 unsigned int iterations)
{
    flt_type lp=0.0;
    for (unsigned int i=0; i<iterations; i++) {
        map<string, flt_type> unigram_stats;
        transitions_t trans_stats;
        assign_scores(transitions, msfg);
        lp = collect_trans_stats(words, msfg, trans_stats, unigram_stats, forward_backward);
        transitions.swap(trans_stats);
        Bigrams::freqs_to_logprobs(transitions);
    }
    return lp;
}


flt_type
Bigrams::iterate_kn(const map<string, flt_type> &words,
                    MultiStringFactorGraph &msfg,
                    transitions_t &transitions,
                    bool forward_backward,
                    flt_type D,
                    unsigned int iterations)
{
    flt_type lp=0.0;
    for (unsigned int i=0; i<iterations; i++) {
        map<string, flt_type> unigram_stats;
        transitions_t trans_stats;
        assign_scores(transitions, msfg);
        lp = collect_trans_stats(words, msfg, trans_stats, unigram_stats, forward_backward);
        kn_smooth(trans_stats, transitions, D);
    }
    return lp;
}


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
Bigrams::update_trans_stats(const transitions_t &collected_stats,
                            flt_type weight,
                            transitions_t &trans_stats)
{
    for (auto srcit = collected_stats.cbegin(); srcit != collected_stats.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit)
            trans_stats[srcit->first][tgtit->first] += weight * tgtit->second;
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

    flt_type total_lp = 0.0;
    if (fb) {
        vector<flt_type> fw(msfg.nodes.size(), MIN_FLOAT);
        fw[0] = 0.0;
        forward(msfg, fw);
        transitions_t word_stats;
        for (auto it = msfg.string_end_nodes.begin(); it != msfg.string_end_nodes.end(); ++it)
            total_lp += words.at(it->first) * backward(msfg, it->first, fw, word_stats, words.at(it->first));
        update_trans_stats(word_stats, 1.0, trans_stats, unigram_stats);
    }
    else {
        total_lp = viterbi(msfg, words, trans_stats);
        get_unigram_stats(trans_stats, unigram_stats);
        finalize_viterbi_stats(msfg, trans_stats);
    }

    return total_lp;
}


void
Bigrams::get_unigram_stats(const transitions_t &trans_stats,
                           map<string, flt_type> &unigram_stats)
{
    for (auto srcit = trans_stats.begin(); srcit != trans_stats.end(); ++srcit)
        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit)
            unigram_stats[tgtit->first] += tgtit->second;
}


void
Bigrams::finalize_viterbi_stats(const MultiStringFactorGraph &msfg,
                                transitions_t &stats)
{
    for (auto sit = stats.begin(); sit != stats.end(); ++sit) {
        const string &srcstr = sit->first;
        const vector<msfg_node_idx_t> &nodes = msfg.factor_node_map.at(srcstr);
        for (auto nit = nodes.begin(); nit != nodes.end(); ++nit) {
            const MultiStringFactorGraph::Node &src_nd = msfg.nodes[*nit];
            for (auto ait=src_nd.outgoing.begin(); ait != src_nd.outgoing.end(); ++ait) {
                string tgtstr = msfg.nodes[(*ait)->target_node].factor;
                if (stats.find(tgtstr) != stats.end()
                    && sit->second.find(tgtstr) == sit->second.end())
                    sit->second[tgtstr] = exp(FLOOR_LP);
            }
        }
    }
}


void
Bigrams::freqs_to_logprobs(transitions_t &trans_stats,
                           flt_type min_cost)
{
    for (auto srcit = trans_stats.begin(); srcit != trans_stats.end(); ++srcit) {
        flt_type normalizer = 0.0;
        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit)
            normalizer += tgtit->second;
        normalizer = log(normalizer);

        bool renormalize = false;
        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit) {
            tgtit->second = log(tgtit->second) - normalizer;
            if (tgtit->second < min_cost) {
                tgtit->second = min_cost;
                renormalize = true;
            }
        }

        if (renormalize) {
            normalizer = MIN_FLOAT;
            for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit)
                if (normalizer == MIN_FLOAT) normalizer = tgtit->second;
                else normalizer = add_log_domain_probs(normalizer, tgtit->second);
            for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit)
                tgtit->second -= normalizer;
        }
    }
}


void
Bigrams::normalize(transitions_t &trans_stats)
{
    for (auto srcit = trans_stats.begin(); srcit != trans_stats.end(); ++srcit) {
        flt_type normalizer = MIN_FLOAT;
        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit)
            if (normalizer == MIN_FLOAT) normalizer = tgtit->second;
            else normalizer = add_log_domain_probs(normalizer, tgtit->second);
        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit)
            tgtit->second -= normalizer;
     }
}


void
Bigrams::write_transitions(const transitions_t &transitions,
                           const string &filename,
                           bool count_style,
                           int num_decimals)
{
    SimpleFileOutput transfile(filename);

    for (auto srcit = transitions.cbegin(); srcit != transitions.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit)
            if (count_style)
                transfile << srcit->first << " " << tgtit->first << "\t" << tgtit->second << "\n";
            else
                transfile << srcit->first << " " << tgtit->first << "\t" << tgtit->second << "\n";

    transfile.close();
}


int
Bigrams::read_transitions(transitions_t &transitions,
                          const string &filename)
{
    SimpleFileInput transfile(filename);

    string line;
    flt_type count;
    int num_trans = 0;
    while (transfile.getline(line)) {
        stringstream ss(line);
        string src, tgt;
        ss >> src;
        ss >> tgt;
        ss >> count;
        transitions[src][tgt] = count;
        num_trans++;
    }

    return num_trans;
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

    normalize(transitions);
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
        for (unsigned int i=0; i<to_remove.size(); i++) {
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

    for (unsigned int i=0; i<to_remove.size(); i++)
        msfg.remove_arcs(to_remove[i]);

    return to_remove.size();
}


int
Bigrams::remove_least_common(const map<string, flt_type> &unigram_stats,
                             unsigned int num_removals,
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
        for (unsigned int i=0; i<num_removals; i++) {
            size_t found = fgit->first.find(sorted_stats[i].first);
            if (found != std::string::npos) fgit->second->remove_arcs(sorted_stats[i].first);
        }
    }

    return to_remove.size();
}


int
Bigrams::remove_least_common(const map<string, flt_type> &unigram_stats,
                             unsigned int num_removals,
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

    for (unsigned int i=0; i<to_remove.size(); i++)
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
                            string src,
                            string tgt,
                            transitions_t &transitions,
                            transitions_t &changes)
{
    changes.clear();

    flt_type renormalizer = sub_log_domain_probs(0, transitions[src][tgt]);
    flt_type ll_diff = 0.0;
    changes[src][tgt] = transitions[src][tgt];
    transitions[src][tgt] = SMALL_LP;

    for (auto it = transitions[src].begin(); it != transitions[src].end(); ++it) {
        if (it->first != tgt) {
            flt_type count = unigram_stats.at(src) * exp(it->second);
            changes[src][it->first] = it->second;
            ll_diff -= count * it->second;
            it->second -= renormalizer;
            ll_diff += count * it->second;
        }
    }

    return ll_diff;
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
Bigrams::init_candidates_freq(unsigned int n_candidates,
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


int
Bigrams::init_candidates_num_contexts(unsigned int n_candidates,
                                      const transitions_t &transitions,
                                      const map<string, flt_type> &unigram_stats,
                                      map<string, flt_type> &candidates)
{
    vector<pair<string, flt_type> > sorted_stats;
    Unigrams::sort_vocab(unigram_stats, sorted_stats, false);

    map<string, flt_type> ctxt_estimate;
    for (auto srcit = transitions.begin(); srcit != transitions.end(); ++srcit) {
        ctxt_estimate[srcit->first] += srcit->second.size();
        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit)
            ctxt_estimate[tgtit->first] += 1;
    }

    for (auto ssit = sorted_stats.begin(); ssit != sorted_stats.end(); ++ssit)
        ssit->second = ctxt_estimate[ssit->first];
    stable_sort(sorted_stats.begin(), sorted_stats.end(), ascending_sort);

    for (auto it = sorted_stats.begin(); it != sorted_stats.end(); ++it) {
        if (it->first.length() < 2) continue;
        candidates[it->first] = 0.0;
        if (candidates.size() >= n_candidates) break;
    }

    return candidates.size();
}


void
Bigrams::rank_candidate_subwords(const map<string, flt_type> &words,
                                 const MultiStringFactorGraph &msfg,
                                 const map<string, flt_type> &unigram_stats,
                                 transitions_t &transitions,
                                 map<string, flt_type> &candidates,
                                 bool forward_backward,
                                 bool normalize_by_bigram_count)
{
    map<string, set<string> > backpointers;
    Bigrams::get_backpointers(msfg, backpointers, 1);
    transitions_t reverse;
    Bigrams::reverse_transitions(transitions, reverse);

    for (auto it = candidates.begin(); it != candidates.end(); ++it) {
        transitions_t changes;
        set<string> words_to_resegment = backpointers.at(it->first);
        flt_type orig_score = likelihood(words, words_to_resegment, msfg, forward_backward);
        flt_type context_score = Bigrams::disable_string(reverse, it->first,
                                                         unigram_stats, transitions, changes);
        flt_type hypo_score = likelihood(words, words_to_resegment, msfg, forward_backward);
        it->second = hypo_score-orig_score + context_score;
        Bigrams::restore_string(transitions, changes);
        if (normalize_by_bigram_count) {
            int num_bigrams = transitions.at(it->first).size() + reverse.at(it->first).size();
            if (it->second < 0) it->second /= num_bigrams;
            else it->second *= num_bigrams;
        }
    }
}


void
Bigrams::kn_smooth(const transitions_t &counts,
                   transitions_t &kn,
                   double D)
{
    kn.clear();

    map<string, double> ctxt_totals;
    map<string, double> ctxt_count;
    map<string, double> unigram_count;
    double u_total = 0;
    for (auto srcit = counts.begin(); srcit != counts.end(); ++srcit) {
        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit) {
            if (tgtit->second > D) {
                ctxt_totals[srcit->first] += tgtit->second;
                ctxt_count[srcit->first] += D;
                unigram_count[tgtit->first] += D;
                u_total += D;
            }
            else {
                ctxt_totals[srcit->first] += tgtit->second;
                ctxt_count[srcit->first] += tgtit->second;
                unigram_count[tgtit->first] += tgtit->second;
                u_total += tgtit->second;
            }
        }
    }

    for (auto srcit = counts.begin(); srcit != counts.end(); ++srcit) {
        for (auto tgtit = srcit->second.begin(); tgtit != srcit->second.end(); ++tgtit) {
            double term1 = max(tgtit->second-D, 0.0);
            term1 /= ctxt_totals[srcit->first];
            double term2 = ctxt_count[srcit->first] / ctxt_totals[srcit->first];
            term2 *= unigram_count[tgtit->first] / u_total;
            double kn_prob = log(term1+term2);
            if (std::isnan(kn_prob)) kn_prob = -100;
            kn[srcit->first][tgtit->first] = kn_prob;
        }
    }
}

