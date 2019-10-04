#include <iomanip>

#include "conf.hh"
#include "str.hh"
#include "Unigrams.hh"

using namespace std;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: g1g-sents [OPTION...] CORPUS VOCAB_INIT VOCAB_FINAL\n")
      ('h', "help", "", "", "display help")
      ('c', "candidates=INT", "arg", "25000", "Number of factors to consider for removal per iteration, DEFAULT: 25 000")
      ('r', "removals=INT", "arg", "500", "Number of removals per iteration, DEFAULT: 500")
      ('m', "min-length=INT", "arg", "2", "Minimum length of factors to remove, DEFAULT: 2")
      ('v', "vocab-size=INT", "arg must", "", "Target vocabulary size (stopping criterion)")
      ('s', "stop-list=STRING", "arg", "", "Text file containing subwords that should not be removed")
      ('t', "temp-vocabs=INT", "arg", "0", "Write out intermediate vocabularies for #V mod INT == 0")
      ('f', "forward-backward", "", "", "Use Forward-backward segmentation instead of Viterbi")
      ('8', "utf-8", "", "", "Utf-8 character encoding in use");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    flt_type short_factor_min_lp = -25.0;
    string corpus_fname = config.arguments[0];
    string vocab_fname = config.arguments[1];
    string out_vocab_fname = config.arguments[2];
    unsigned int n_candidates_per_iter = config["candidates"].get_int();
    unsigned int removals_per_iter = config["removals"].get_int();
    unsigned int min_removal_length = config["min-length"].get_int();
    unsigned int target_vocab_size = config["vocab-size"].get_int();
    unsigned int temp_vocab_interval = config["temp-vocabs"].get_int();
    bool enable_forward_backward = config["forward-backward"].specified;
    bool utf8_encoding = config["utf-8"].specified;

    set<string> stoplist;
    if (config["stop-list"].specified) {
        vector<string> stopstrings;
        Unigrams::read_sents(config["stop-list"].get_str(), stopstrings);
        stoplist.insert(stopstrings.begin(), stopstrings.end());
    }

    cerr << setprecision(15) << std::boolalpha;
    cerr << "parameters, training corpus: " << corpus_fname << endl;
    cerr << "parameters, initial vocabulary: " << vocab_fname << endl;
    cerr << "parameters, final vocabulary: " << out_vocab_fname << endl;
    cerr << "parameters, candidates per iteration: " << n_candidates_per_iter << endl;
    cerr << "parameters, minimum length for factors to remove: " << min_removal_length << endl;
    cerr << "parameters, removals per iteration: " << removals_per_iter << endl;
    if (stoplist.size() > 0)
        cerr << "parameters, stoplist with " << stoplist.size() << " subwords" << endl;
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;
    if (temp_vocab_interval > 0)
        cerr << "parameters, write temporary vocabularies whenever #V modulo " << temp_vocab_interval << " == 0" << endl;
    else
        cerr << "parameters, write temp vocabularies: NO" << endl;
    cerr << "parameters, use forward-backward: " << enable_forward_backward << endl;
    cerr << "parameters, utf-8 encoding: " << utf8_encoding << endl;

    int maxlen;
    set<string> short_factors;
    map<string, flt_type> vocab;
    map<string, flt_type> freqs;
    vector<string> sents;

    cerr << "Reading vocabulary " << vocab_fname << endl;
    int retval = Unigrams::read_vocab(vocab_fname, vocab, maxlen, utf8_encoding);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(EXIT_FAILURE);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;
    find_short_factors(vocab, short_factors, min_removal_length, utf8_encoding);

    cerr << "Reading training corpus " << corpus_fname << endl;
    retval = Unigrams::read_sents(corpus_fname, sents);
    if (retval < 0) {
        cerr << "something went wrong reading training corpus" << endl;
        exit(EXIT_FAILURE);
    }
    cerr << "\t" << "number of sentences in corpus: " << sents.size() << endl;

    Unigrams gg;
    if (enable_forward_backward)
        gg.set_segmentation_method(forward_backward);
    else
        gg.set_segmentation_method(viterbi);
    gg.set_utf8(utf8_encoding);

    flt_type cost = gg.resegment_sents(sents, vocab, freqs);
    cerr << "Initial likelihood: " << cost << endl;

    cerr << endl << "Removing factors by likelihood based pruning" << endl;
    int itern = 1;
    while (true) {

        cerr << "iteration " << itern << endl;
        cerr << "collecting candidate factors" << endl;
        set<string> candidates;
        gg.init_candidates(vocab, candidates, n_candidates_per_iter, stoplist, min_removal_length);

        cerr << "ranking candidate factors (" << candidates.size() << ")" << endl;
        vector<pair<string, flt_type> > removal_scores;
        cost = gg.rank_candidates(sents, vocab, candidates, freqs, removal_scores);

        cerr << "initial likelihood before removing factors: " << cost << endl;

        // Remove factors one by one
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

        cost = gg.iterate(sents, vocab, 1);
        assert_factors(vocab, short_factors, short_factor_min_lp);

        cerr << "factors removed in this iteration: " << n_removals << endl;
        cerr << "current vocabulary size: " << vocab.size() << endl;
        cerr << "likelihood after the removals: " << cost << endl;

        itern++;

        if (vocab.size() <= target_vocab_size) {
            cerr << "stopping by min_vocab_size." << endl;
            break;
        }
    }

    Unigrams::write_vocab(out_vocab_fname, vocab);
    exit(EXIT_SUCCESS);
}
