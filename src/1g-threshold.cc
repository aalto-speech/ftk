#include <iomanip>
#include <algorithm>

#include "conf.hh"
#include "str.hh"
#include "Unigrams.hh"

using namespace std;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: 1g-threshold [OPTION...] WORDLIST VOCAB_INIT VOCAB_FINAL\n")
      ('h', "help", "", "", "display help")
      ('i', "threshold-increment=FLOAT", "arg", "1.0", "Threshold increment for each iteration, DEFAULT: 1.0")
      ('m', "min-length=INT", "arg", "2", "Minimum length of subwords to remove, DEFAULT: 2")
      ('v', "vocab-size=INT", "arg must", "", "Target vocabulary size")
      ('s', "stop-list=STRING", "arg", "", "Text file containing subwords that should not be removed")
      ('f', "forward-backward", "", "", "Use Forward-backward segmentation instead of Viterbi")
      ('8', "utf-8", "", "", "Utf-8 character encoding in use");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    flt_type subword_min_lp = -25.0;
    string wordlist_fname = config.arguments[0];
    string vocab_fname = config.arguments[1];
    string out_vocab_fname = config.arguments[2];
    float threshold_increment = config["threshold-increment"].get_float();
    unsigned int min_removal_length = config["min-length"].get_int();
    unsigned int target_vocab_size = config["vocab-size"].get_int();
    bool enable_forward_backward = config["forward-backward"].specified;
    bool utf8_encoding = config["utf-8"].specified;

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
    cerr << "parameters, threshold increment: " << threshold_increment << endl;
    cerr << "parameters, minimum length for subwords to remove: " << min_removal_length << endl;
    if (stoplist.size() > 0)
        cerr << "parameters, stoplist with " << stoplist.size() << " subwords" << endl;
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;
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
        exit(EXIT_FAILURE);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;
    find_short_factors(vocab, short_subwords, min_removal_length, utf8_encoding);

    set<string> stop_union;
    set_union(stoplist.begin(), stoplist.end(),
              short_subwords.begin(), short_subwords.end(),
              inserter(stop_union, stop_union.begin()));
    stoplist = stop_union;

    cerr << "Reading word list " << wordlist_fname << endl;
    retval = Unigrams::read_vocab(wordlist_fname, words, word_maxlen, utf8_encoding);
    if (retval < 0) {
        cerr << "something went wrong reading word list" << endl;
        exit(EXIT_FAILURE);
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

    flt_type threshold_value = 0.0;
    while (target_vocab_size > 0 && vocab.size() > target_vocab_size) {
        threshold_value += threshold_increment;
        gg.cutoff(freqs, threshold_value, stoplist, min_removal_length);
        cerr << "\tthreshold: " << threshold_value << "\t" << "vocabulary size: " << freqs.size() << endl;
        vocab = freqs;
        Unigrams::freqs_to_logprobs(vocab);
        assert_factors(vocab, stoplist, subword_min_lp);
        cost = gg.resegment_words(words, vocab, freqs);
        cerr << "likelihood: " << cost << endl;
    }

    Unigrams::write_vocab(out_vocab_fname, vocab);
    exit(EXIT_SUCCESS);
}
