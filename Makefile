-include Makefile.local

cxxflags += -std=gnu++0x

##################################################

progs = segtext\
	substrings\
	strscore\
	1g-threshold\
	1g-prune\
	2g-prune\
	2g-prune-kn\
	2g-prune-simple\
	segposts\
	iterate\
	iterate12\
	cmsfg\
	llh\
	counts\
	iterate-sents\
	g1g-sents
progs_srcs = $(addsuffix .cc,$(addprefix src/,$(progs)))
progs_objs = $(addsuffix .o,$(addprefix src/,$(progs)))

srcs = util/io.cc\
	util/conf.cc\
	util/Ngram.cc\
	src/StringSet.cc\
	src/FactorGraph.cc\
	src/MSFG.cc\
	src/EM.cc\
	src/Unigrams.cc\
	src/Bigrams.cc
objs = $(srcs:.cc=.o)

ifndef NO_UNIT_TESTS
test_progs = runtests
test_progs_srcs = $(test_progs:=.cc)
test_progs_objs = $(test_progs:=.o)
test_srcs = test/sstest.cc\
	test/emtest.cc\
	test/msfgtest.cc
test_objs = $(test_srcs:.cc=.o)
endif

##################################################

.SUFFIXES:

all: $(progs) $(test_progs)

%.o: %.cc
	$(CXX) -c $(cxxflags) $< -o $@ -I./util

$(progs): $(progs_objs) $(objs)
	$(CXX) $(cxxflags) -o $@ src/$@.cc $(objs) -lz -I./util -I./src

%: %.o $(objs)
	$(CXX) $(cxxflags) $< -o $@ $(objs)

$(test_progs): $(test_objs)
	$(CXX) $(cxxflags) -o $@ test/$@.cc $(objs) $(test_objs)\
	 -lboost_unit_test_framework -lz -I./util -I./src

$(test_objs): %.o: %.cc $(objs)
	$(CXX) -c $(cxxflags) $< -o $@ -I./util -I./src

.PHONY: clean
clean:
	rm -f $(objs) $(progs_objs) $(test_objs)\
	 $(progs) $(test_progs) .depend *~ *.exe

dep:
	$(CXX) -MM $(cxxflags) $(DEPFLAGS) $(all_srcs) > dep
include dep
