all: parser

OBJS = parser.o  \
       codegen.o \
       main.o    \
       tokens.o  \
       corefn.o  \
	   native.o  \

LLVMCONFIG = llvm-config-3.9
CPPFLAGS = `$(LLVMCONFIG) --cppflags` -std=c++11
LDFLAGS = `$(LLVMCONFIG) --ldflags` -lpthread -ldl -lz -lncurses -rdynamic -I/usr/include/jsoncpp -ljsoncpp
LIBS = `$(LLVMCONFIG) --libs`

clean:
	$(RM) -rf parser.cpp parser.hpp parser tokens.cpp $(OBJS)

parser.cpp: parser.y
	bison -d -o $@ $^
	
parser.hpp: parser.cpp

tokens.cpp: tokens.l parser.hpp
	flex -o $@ $^

%.o: %.cpp
	g++ -c $(CPPFLAGS) -o $@ $< $(LDFLAGS)


parser: $(OBJS)
	g++ -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

test: parser example.txt
	cat example.txt | ./parser

