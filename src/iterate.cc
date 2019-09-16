#include <ctime>

#include "conf.hh"
#include "Unigrams.hh"

using namespace std;


void floor_values(map<string, flt_type> &vocab,
                  flt_type floor_val)
{
    for (auto it = vocab.begin(); it != vocab.end(); ++it)
        if (it->second < floor_val) it->second = floor_val;
}


int main(int argc, char* argv[]) {

    flt_type one_char_min_lp = -25.0;

    conf::Config config;
    config("usage: iterate [OPTION...] WORDLIST IN_VOCAB OUT_VOCAB\n")
      ('h', "help", "", "", "display help")
      ('i', "iterations=INT", "arg", "5", "Number of iterations")
      ('f', "forward-backward", "", "", "Use Forward-backward segmentation instead of Viterbi")
      ('c', "text-corpus", "", "", "Corpus is running text, possibly with <s>, </s>, <w> markers")
      ('8', "utf-8", "", "", "Utf-8 character encoding in use");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    int num_iterations = config["iterations"].get_int();
    bool enable_forward_backward = config["forward-backward"].specified;
    bool running_text = config["text-corpus"].specified;
    bool utf8_encoding = config["utf-8"].specified;
    string wordlist_fname = config.arguments[0];
    string vocab_in_fname = config.arguments[1];
    string vocab_out_fname = config.arguments[2];

    cerr << std::boolalpha;
    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, initial vocabulary: " << vocab_in_fname << endl;
    cerr << "parameters, final vocabulary: " << vocab_out_fname << endl;
    cerr << "parameters, use forward-backward: " << enable_forward_backward << endl;
    cerr << "parameters, number of iterations: " << num_iterations << endl;
    cerr << "parameters, utf-8 encoding: " << utf8_encoding << endl;
    cerr << "parameters, running text: " << running_text << endl;

    int maxlen, word_maxlen;
    set<string> all_chars;
    map<string, flt_type> vocab;
    map<string, flt_type> freqs;
    map<string, flt_type> words;

    cerr << "Reading vocabulary " << vocab_in_fname << endl;
    int retval = Unigrams::read_vocab(vocab_in_fname, vocab, maxlen, utf8_encoding);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(0);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;
    find_short_factors(vocab, all_chars, 2, utf8_encoding);

    set<string> special_words;
    if (!running_text) {
        cerr << "Reading word list " << wordlist_fname << endl;
        retval = Unigrams::read_vocab(wordlist_fname, words, word_maxlen, utf8_encoding);
        if (retval < 0) {
            cerr << "something went wrong reading word list" << endl;
            exit(1);
        }
    }

    else {
        cerr << "Reading corpus " << wordlist_fname << endl;
        retval = Unigrams::read_corpus(wordlist_fname, words, word_maxlen, utf8_encoding);
        if (retval < 0) {
            cerr << "something went wrong reading text corpus" << endl;
            exit(1);
        }
        // Check for possible <s>, </s>, <w> etc.
        for (auto wit=words.begin(); wit != words.end(); ++wit)
            if (wit->first.find("<") != string::npos
                && wit->first.find(">") != string::npos)
                special_words.insert(wit->first);
    }

    cerr << "\t" << "wordlist size: " << words.size() << endl;
    cerr << "\t" << "maximum word length: " << word_maxlen << endl;

    Unigrams gg;
    if (enable_forward_backward)
        gg.set_segmentation_method(forward_backward);
    else
        gg.set_segmentation_method(viterbi);
    gg.set_utf8(utf8_encoding);

    cerr << "iterating.." << endl;
    time_t rawtime;
    time ( &rawtime );
    cerr << "start time: " << ctime (&rawtime) << endl;
    for (int i=0; i<num_iterations; i++) {
        flt_type cost = gg.resegment_words(words, vocab, freqs, special_words);
        cerr << "likelihood: " << cost << endl;
        vocab = freqs;
        Unigrams::freqs_to_logprobs(vocab);
        floor_values(vocab, SMALL_LP);
        assert_factors(vocab, all_chars, one_char_min_lp);
    }
    time ( &rawtime );
    cerr << "end time: " << ctime (&rawtime) << endl;

    Unigrams::write_vocab(vocab_out_fname, vocab);

    exit(0);
}
