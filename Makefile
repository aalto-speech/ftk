
cxxflags = -O2 -march=native -g

##################################################

prog = fe
prog_srcs = $(prog:=.cc)
srcs = factorencoder.cc
objs = $(srcs:.cc=.o)

test_prog = test_fe
test_prog_srcs = $(test_prog:=.cc)
test_srcs = fetest.cc
test_objs = $(test_srcs:.cc=.o)


##################################################

.SUFFIXES:

default: $(prog)

%.o: %.cc
	$(CXX) -c $(cxxflags) $< -o $@

%: %.o $(objs)
	$(CXX) $(cxxflags) $< -o $@ $(objs)

test_prog: $(test_prog_srcs) $(test_objs) $(prog_objs)
	$(CXX) $(cxxflags) $< -o $@ $(test_objs) -lcppunit

test_objs: $(test_srcs)

test_progs: $(objs) $(test_objs)

test: $(test_prog)

.PHONY: clean
clean:
	rm -f $(objs) $(prog) $(test_prog) $(prog_objs) $(test_objs) .depend *~

dep:
	$(CXX) -MM $(cxxflags) $(DEPFLAGS) $(all_srcs) > dep
include dep

