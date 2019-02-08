GPP = g++ -m32 -std=c++11 -Ilib -Isrc -fno-stack-protector -nodefaultlibs
GCC = gcc -m32 -Ilib -Isrc -fno-stack-protector -nodefaultlibs
LINK = $(GCC)
PP_OUTFILE = "./NativeFallback.so"

COMPILE_FLAGS = -c -O3 -fPIC -w -DLINUX -pthread -fno-operator-names

NativeFallback = -D NativeFallback $(COMPILE_FLAGS)

all: NativeFallback

clean:
	-rm -f *~ *.o *.so
  
static: GPP = g++ -m32 -std=c++11 -Ilib -Isrc -fno-stack-protector -static-libgcc -static-libstdc++
static: GCC = gcc -m32 -Ilib -Isrc -fno-stack-protector -static-libgcc -static-libstdc++
static: LINK = $(GPP)
static: all

NativeFallback: clean
	$(GPP) $(NativeFallback) ./lib/sdk/*.cpp
	$(GCC) $(NativeFallback) ./lib/subhook/*.c
	$(GPP) $(NativeFallback) ./src/*.cpp
	$(LINK) -fshort-wchar -pthread -shared -o $(PP_OUTFILE) *.o
