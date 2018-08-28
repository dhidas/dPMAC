CC = g++
LD = g++
CFLAGS = -Wall -O3
LIBS = -lreadline
INCLUDE = -Iinclude
OBJS  = $(patsubst src/%.cc,lib/%.o,$(wildcard src/*.cc))
EXECS = $(patsubst exe/%.cc,bin/%,$(wildcard exe/*.cc))
EXEOBJS  = $(patsubst exe/%.cc,lib/%.o,$(wildcard exe/*.cc))


all: $(OBJS) $(EXEOBJS) $(EXECS)

install:
	make all
	mkdir -pv ~/bin
	cp bin/* ~/bin

lib/%.o : src/%.cc
	$(CC) -Wall $(CFLAGS) $(INCLUDE) -c $< -o $@

lib/%.o : exe/%.cc
	$(CC) -Wall $(CFLAGS) $(INCLUDE) -c $< -o $@


bin/% : $(OBJS) lib/%.o
	$(LD) $(LIBS) $(OBJS) lib/$*.o -o bin/$*





clean:
	rm -f $(EXECS) lib/*.o

