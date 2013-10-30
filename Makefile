
cxxflags = -O3 -march=native -std=gnu++0x -Wall -Wno-sign-compare -Wno-unused-variable -Wno-unused-function
#cxxflags = -O0 -gddb -std=gnu++0x -Wall
#cxxflags = -O0 -g -std=gnu++0x -Wall -Wno-sign-compare -Wno-unused-variable -Wno-unused-function

##################################################

progs = fe substrings strscore g1g g2g g2gr segposts iterate cmsfg ll counts iterate-sents g1g-sents
progs_srcs = $(progs:=.cc)
progs_objs = $(progs:=.o)
srcs = io.cc conf.cc ArpaReader.cc LM.cc StringSet.cc FactorGraph.cc MSFG.cc FactorEncoder.cc Unigrams.cc Bigrams.cc
objs = $(srcs:.cc=.o)

test_progs = test_fe
test_progs_srcs = $(test_progs:=.cc)
test_progs_objs = $(test_progs:=.o)
test_srcs = fetest.cc msfgtest.cc sstest.cc
test_objs = $(test_srcs:.cc=.o)

##################################################

.SUFFIXES:

all: $(progs) $(test_progs)

%.o: %.cc
	$(CXX) -c $(cxxflags) $< -o $@

$(progs): %: %.o $(objs)
	$(CXX) $(cxxflags) $< -o $@ $(objs)

%: %.o $(objs)
	$(CXX) $(cxxflags) $< -o $@ $(objs)

$(test_progs): %: %.o $(objs) $(test_objs)
	$(CXX) $(cxxflags) $< -o $@ $(objs) $(test_objs) -lcppunit

test_objs: $(test_srcs)

test_progs: $(objs) $(test_objs)

test: $(test_progs)

.PHONY: clean
clean:
	rm -f $(objs) $(progs) $(progs_objs) $(test_progs) $(test_progs_objs) $(test_objs) .depend *~

dep:
	$(CXX) -MM $(cxxflags) $(DEPFLAGS) $(all_srcs) > dep
include dep
