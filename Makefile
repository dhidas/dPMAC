CC = g++
LD = g++
CFLAGS = -Wall -O3 -std=c++14
LIBS = -stdlib=libc++
INCLUDE = -Iinclude
OBJS  = $(patsubst src/%.cc,lib/%.o,$(wildcard src/*.cc))
EXECS = $(patsubst exe/%.cc,bin/%,$(wildcard exe/*.cc))
EXEOBJS  = $(patsubst exe/%.cc,lib/%.o,$(wildcard exe/*.cc))


all: $(OBJS) $(EXEOBJS) $(EXECS)

lib/%.o : src/%.cc
	$(CC) -Wall $(CFLAGS) $(INCLUDE) -c $< -o $@

lib/%.o : exe/%.cc
	$(CC) -Wall $(CFLAGS) $(INCLUDE) -c $< -o $@


bin/% : $(OBJS) lib/%.o
	$(LD) $(LIBS) $(OBJS) lib/$*.o -o bin/$*





clean:
	rm -f $(EXECS) lib/*.o

