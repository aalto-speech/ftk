
cxxflags = -O3 -march=native

##################################################

progs = fe
progs_srcs = $(progs:=.cc)
objs = $(srcs:.cc=.o)
all_srcs = $(progs_srcs)

##################################################

.SUFFIXES:

default: $(progs)

%.o: %.cc
	$(CXX) -c $(cxxflags) $< -o $@

%: %.o $(objs)
	$(CXX) $(cxxflags) $< -o $@ $(objs)

progs: $(progs_srcs)

.PHONY: clean
clean:
	rm -f $(objs) $(progs) .depend *~

dep:
	$(CXX) -MM $(cxxflags) $(DEPFLAGS) $(all_srcs) > dep
include dep

