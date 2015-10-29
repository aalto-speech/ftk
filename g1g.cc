#include <iomanip>

#include "conf.hh"
#include "str.hh"
#include "Unigrams.hh"

using namespace std;


void parse_limits(string limitstr, map<unsigned int, unsigned int> &limits) {
    vector<string> fields;
    str::split_with_quotes(&limitstr, " ,", true, &fields);
    if (fields.size() % 2 != 0) throw string("Problem parsing the removal limits");
    for (unsigned int i=0; i<fields.size(); i=i+2)
        limits[stoi(fields[i])] = stoi(fields[i+1]);
}


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: g1g [OPTION...] WORDLIST VOCAB_INIT VOCAB_FINAL\n")
      ('h', "help", "", "", "display help")
      ('u', "cutoff-target=INT", "arg", "200000", "Iterate cutoff while this vocabulary size is reached, DEFAULT: 200 000")
      ('i', "cutoff-increment=FLOAT", "arg", "1.0", "Cutoff increment for each iteration, DEFAULT: 1.0")
      ('c', "candidates=INT", "arg", "50000", "Number of subwords to consider for removal per iteration, DEFAULT: 50 000")
      ('r', "removals=INT", "arg", "500", "Number of removals per iteration, DEFAULT: 500")
      ('l', "removal-limits=STRING", "arg", "", "Extra limits for subword removal LIMIT1,REMOVALS1,LIMIT2,REMOVALS2,..")
      ('m', "min-length=INT", "arg", "2", "Minimum length of subwords to remove, DEFAULT: 2")
      ('v', "vocab-size=INT", "arg must", "", "Target vocabulary size (stopping criterion)")
      ('s', "stop-list=STRING", "arg", "", "Text file containing subwords that should not be removed")
      ('t', "temp-vocabs=INT", "arg", "0", "Write out intermediate vocabularies for #V mod INT == 0")
      ('f', "forward-backward", "", "", "Use Forward-backward segmentation instead of Viterbi")
      ('8', "utf-8", "", "", "Utf-8 character encoding in use");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    flt_type short_subword_min_lp = -25.0;
    string wordlist_fname = config.arguments[0];
    string vocab_fname = config.arguments[1];
    string out_vocab_fname = config.arguments[2];
    float cutoff_target = config["cutoff-target"].get_float();
    float cutoff_increment = config["cutoff-increment"].get_float();
    unsigned int n_candidates_per_iter = config["candidates"].get_int();
    unsigned int removals_per_iter = config["removals"].get_int();
    unsigned int min_removal_length = config["min-length"].get_int();
    unsigned int target_vocab_size = config["vocab-size"].get_int();
    unsigned int temp_vocab_interval = config["temp-vocabs"].get_int();
    bool enable_forward_backward = config["forward-backward"].specified;
    bool utf8_encoding = config["utf-8"].specified;

    map<unsigned int, unsigned int> removal_limits;
    if (config["removal-limits"].specified)
        parse_limits(config["removal-limits"].get_str(), removal_limits);

    set<string> stoplist;
    if (config["stop-list"].specified) {
        vector<string> stopstrings;
        Unigrams::read_sents(config["stop-list"].get_str(), stopstrings);
        stoplist.insert(stopstrings.begin(), stopstrings.end());
    }

    cerr << setprecision(15) << std::boolalpha;
    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, initial vocabulary: " << vocab_fname << endl;
    cerr << "parameters, final vocabulary: " << out_vocab_fname << endl;
    cerr << "parameters, cutoff target vocabulary size: " << cutoff_target << endl;
    cerr << "parameters, cutoff increment: " << cutoff_increment << endl;
    cerr << "parameters, candidates per iteration: " << n_candidates_per_iter << endl;
    cerr << "parameters, minimum length for subwords to remove: " << min_removal_length << endl;
    cerr << "parameters, removals per iteration: " << removals_per_iter << endl;
    if (removal_limits.size() > 0) {
        int limitc = 1;
        for (auto it = removal_limits.rbegin(); it != removal_limits.rend(); ++it) {
            cerr << "parameters, removal limit " << limitc << ": "
            << it->second << " removals per iteration below vocabulary size " << it->first << endl;
            limitc++;
        }
    }
    if (stoplist.size() > 0)
        cerr << "parameters, stoplist with " << stoplist.size() << " subwords" << endl;
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;
    if (temp_vocab_interval > 0)
        cerr << "parameters, write temporary vocabularies whenever #V modulo " << temp_vocab_interval << " == 0" << endl;
    else
        cerr << "parameters, write temp vocabularies: NO" << endl;
    cerr << "parameters, use forward-backward: " << enable_forward_backward << endl;
    cerr << "parameters, utf-8 encoding: " << utf8_encoding << endl;

    int maxlen, word_maxlen;
    set<string> short_subwords;
    map<string, flt_type> vocab;
    map<string, flt_type> freqs;
    map<string, flt_type> words;

    cerr << "Reading vocabulary " << vocab_fname << endl;
    int retval = Unigrams::read_vocab(vocab_fname, vocab, maxlen, utf8_encoding);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(0);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;
    find_short_factors(vocab, short_subwords, min_removal_length, utf8_encoding);

    cerr << "Reading word list " << wordlist_fname << endl;
    retval = Unigrams::read_vocab(wordlist_fname, words, word_maxlen, utf8_encoding);
    if (retval < 0) {
        cerr << "something went wrong reading word list" << endl;
        exit(0);
    }
    cerr << "\t" << "wordlist size: " << words.size() << endl;
    cerr << "\t" << "maximum word length: " << word_maxlen << endl;

    Unigrams gg;
    if (enable_forward_backward)
        gg.set_segmentation_method(forward_backward);
    else
        gg.set_segmentation_method(viterbi);
    gg.set_utf8(utf8_encoding);

    flt_type cost = gg.resegment_words(words, vocab, freqs);
    cerr << endl << "Initial likelihood: " << cost << endl;

    flt_type cutoff_value = 0.0;
    if (cutoff_target > 0.0) cerr << endl << "Initial cutoff" << endl;
    while (cutoff_target > 0.0 && vocab.size() > cutoff_target) {
        cutoff_value += cutoff_increment;
        gg.cutoff(freqs, cutoff_value, stoplist, min_removal_length);
        cerr << "\tcutoff: " << cutoff_value << "\t" << "vocabulary size: " << freqs.size() << endl;
        vocab = freqs;
        Unigrams::freqs_to_logprobs(vocab);
        assert_short_factors(vocab, short_subwords, short_subword_min_lp);
        cost = gg.resegment_words(words, vocab, freqs);
        cerr << "likelihood: " << cost << endl;
    }

    cerr << endl << "Removing subwords by likelihood based pruning" << endl;
    int itern = 1;
    while (true) {

        cerr << "iteration " << itern << endl;

        for (auto it = removal_limits.rbegin(); it != removal_limits.rend(); it++)
            if (vocab.size() <= it->first) removals_per_iter = it->second;

        cerr << "collecting candidate subwords" << endl;
        set<string> candidates;
        gg.init_candidates_by_usage(words, vocab, candidates, n_candidates_per_iter/3, stoplist, min_removal_length);
        gg.init_candidates_by_random(vocab, candidates, (n_candidates_per_iter-candidates.size())/4, stoplist, min_removal_length);
        gg.init_candidates(vocab, candidates, n_candidates_per_iter, stoplist, min_removal_length);

        cerr << "ranking candidate subwords (" << candidates.size() << ")" << endl;
        vector<pair<string, flt_type> > removal_scores;
        cost = gg.rank_candidates(words, vocab, candidates, freqs, removal_scores);

        cerr << "initial likelihood before removing subwords: " << cost << endl;

        // Remove subwords one by one
        unsigned int n_removals = 0;
        for (unsigned int i=0; i<removal_scores.size(); i++) {

            if (removal_scores[i].first.length() == 1) continue;
            // Score most probably went to zero already
            if (vocab.find(removal_scores[i].first) == vocab.end()) continue;
            if (freqs.find(removal_scores[i].first) == freqs.end()) continue;

            vocab.erase(removal_scores[i].first);
            freqs.erase(removal_scores[i].first);
            n_removals++;

            if (temp_vocab_interval > 0 && freqs.size() % temp_vocab_interval == 0) {
                vocab = freqs;
                Unigrams::freqs_to_logprobs(vocab);
                ostringstream vocabfname;
                vocabfname << "iteration_" << itern << "_" << vocab.size() << ".vocab";
                Unigrams::write_vocab(vocabfname.str(), vocab);
            }

            if (n_removals >= removals_per_iter) break;
            if (vocab.size() <= target_vocab_size) break;
        }

        cost = gg.iterate(words, vocab, 1);
        assert_short_factors(vocab, short_subwords, short_subword_min_lp);

        cerr << "subwords removed in this iteration: " << n_removals << endl;
        cerr << "current vocabulary size: " << vocab.size() << endl;
        cerr << "likelihood after the removals: " << cost << endl;

        itern++;

        if (vocab.size() <= target_vocab_size) {
            cerr << "stopping by min_vocab_size." << endl;
            break;
        }
    }

    Unigrams::write_vocab(out_vocab_fname, vocab);
    exit(0);
}
