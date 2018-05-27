CC = clang
CXX = clang++

WARNINGS = -Wall -Wextra -Weffc++ -Wno-unused-function -Wno-unused-variable
INCLUDES = -Isrc -I/usr/include/botan-2
CFLAGS = $(WARNINGS) $(INCLUDES) -DNDEBUG -march=native -fomit-frame-pointer -Ofast -DBORINGSSL_HAS_UINT128=1 -std=gnu11
CXXFLAGS = $(WARNINGS) $(INCLUDES) -DNDEBUG -march=native -fomit-frame-pointer -Ofast -std=gnu++17
LDFLAGS = -lbotan-2 -lboost_program_options -lboost_system -lpthread

OBJECTS = \
	src/curve25519/mycurve25519.c.o \
    src/curve25519/util.c.o \
    src/network/dummy_connection.cpp.o \
    src/network/tcp_connection.cpp.o \
    src/ot/ot_hl17.cpp.o \
    src/util/options.cpp.o \
    src/util/threading.cpp.o \
    src/util/util.cpp.o

.phony: clean

baseOT: bin/baseOT.o libparty.a
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

libparty.a: $(OBJECTS)
	ar qc libparty.a  $(OBJECTS)

%.c.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	find bin/ src/ -iname "*.o" -delete
	rm -f libparty.a
	rm -f baseOT
