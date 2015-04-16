cxxflags = -O3 -march=native -std=gnu++0x -Wall -Wno-unused-function -Wno-write-strings
#cxxflags = -O0 -g -std=gnu++0x -Wall -Wno-unused-function -Wno-write-strings

##################################################

progs = segtext substrings strscore g1g g2gr g2g g2gkn segposts iterate iterate12 cmsfg llh counts iterate-sents g1g-sents
progs_srcs = $(progs:=.cc)
progs_objs = $(progs:=.o)
srcs = io.cc conf.cc Ngram.cc StringSet.cc FactorGraph.cc MSFG.cc EM.cc Unigrams.cc Bigrams.cc
objs = $(srcs:.cc=.o)

test_progs = runtests
test_progs_srcs = $(test_progs:=.cc)
test_progs_objs = $(test_progs:=.o)
test_srcs = emtest.cc msfgtest.cc sstest.cc
test_objs = $(test_srcs:.cc=.o)

##################################################

.SUFFIXES:

all: $(progs) $(test_progs)

%.o: %.cc
	$(CXX) -c $(cxxflags) $< -o $@

$(progs): %: %.o $(objs)
	$(CXX) $(cxxflags) $< -o $@ $(objs) -lz

%: %.o $(objs)
	$(CXX) $(cxxflags) $< -o $@ $(objs)

$(test_progs): %: %.o $(objs) $(test_objs)
	$(CXX) $(cxxflags) $< -o $@ $(objs) $(test_objs) -lcppunit -lz

test_objs: $(test_srcs)

test_progs: $(objs) $(test_objs)

test: $(test_progs)

.PHONY: clean
clean:
	rm -f $(objs) $(progs) $(progs_objs) $(test_progs) $(test_progs_objs) $(test_objs) .depend *~

dep:
	$(CXX) -MM $(cxxflags) $(DEPFLAGS) $(all_srcs) > dep
include dep
