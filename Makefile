
cxxflags = -O2 -march=native -g

##################################################

progs = fe
progs_srcs = $(progs:=.cc)
all_srcs = $(progs_srcs)
objs = $(srcs:.cc=.o)

test_progs = test_fe
test_progs_srcs = $(test_progs:=.cc) $(progs_srcs)
test_srcs = $(test_progs_srcs) fetest.cc
test_objs = $(test_srcs:.cc=.o)


##################################################

.SUFFIXES:

default: $(progs)

%.o: %.cc
	$(CXX) -c $(cxxflags) $< -o $@

%: %.o $(objs)
	$(CXX) $(cxxflags) $< -o $@ $(objs)

test_fe: test_fe.o $(objs)
	$(CXX) $(cxxflags) $< -o $@ $(objs) -lcppunit

progs: $(progs_srcs)

test_objs: $(test_srcs)
test_progs: $(test_progs_src) $(objs)
test: $(test_progs)

.PHONY: clean
clean:
	rm -f $(objs) $(progs) $(test_progs) $(test_objs) .depend *~

dep:
	$(CXX) -MM $(cxxflags) $(DEPFLAGS) $(all_srcs) > dep
include dep

