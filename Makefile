VERSION=0.0.5
#-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
OPT_DEBUG=  -std=c++0x -Wall -Wextra -Wlong-long -Wunused -O0 -ggdb -pedantic -pg
OPT_RELEASE=-std=c++0x -Wall -Wextra -Wlong-long -Wunused -O777 -s
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
CXX=g++
LN=g++
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
CFLAGS_x32DBG="-m32 -ggdb -O0   -std=c++0x -Wall -Wextra -Wlong-long -Wunused -pg";
CFLAGS_x32REL="-m32       -O777 -std=c++0x -Wall -Wextra -Wlong-long -Wunused";
CFLAGS_x64DBG="-m64 -ggdb -O0   -std=c++0x -Wall -Wextra -Wlong-long -Wunused -pg";
CFLAGS_x64REL="-m64       -O777 -std=c++0x -Wall -Wextra -Wlong-long -Wunused";
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
LFLAGS_x32DBG="-m32 -lrt -lpthread -lpq -ggdb";
LFLAGS_x32REL="-m32 -lrt -lpthread -lpq -s";
LFLAGS_x64DBG="-m64 -lrt -lpthread -lpq -ggdb";
LFLAGS_x64REL="-m64 -lrt -lpthread -lpq -s";
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
help:
	@echo "use make [x32 | x64 | x32dbg | x64dbg | clean]";
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
x32:    patch_spliter.cpp Makefile
	@if [ ! -e bin ]; then    (mkdir bin;)     fi
	@$(CXX) patch_spliter.cpp -c -o bin/patch_spliter.o -D'ARCH="$(@)"' -D'VERSION="$(VERSION)"' $(OPT_RELEASE) -m32
	@$(LN) bin/patch_spliter.o -o bin/patch_spliter-$(@)-$(VERSION) -D'ARCH="$(@)"' -D'VERSION="$(VERSION)"' $(OPT_RELEASE) -m32
	@ln -sf patch_spliter-$(@)-$(VERSION) bin/patch_spliter
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
x32dbg: patch_spliter.cpp Makefile
	@if [ ! -e bin ]; then    (mkdir bin;)     fi
	@$(CXX) patch_spliter.cpp -c -o bin/patch_spliter.o -D'ARCH="$(@)"' -D'VERSION="$(VERSION)"' $(OPT_DEBUG) -m32
	@$(LN) bin/patch_spliter.o -o bin/patch_spliter-$(@)-$(VERSION) -D'ARCH="$(@)"' -D'VERSION="$(VERSION)"' $(OPT_DEBUG) -m32
	@ln -sf patch_spliter-$(@)-$(VERSION) bin/patch_spliter
	@objdump -Dslx bin/patch_spliter-$(@)-$(VERSION) > bin/patch_spliter-$(@)-$(VERSION).dump;
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
x64:    patch_spliter.cpp Makefile
	@if [ ! -e bin ]; then    (mkdir bin;)     fi
	@$(CXX) patch_spliter.cpp -c -o bin/patch_spliter.o -D'ARCH="$(@)"' -D'VERSION="$(VERSION)"' $(OPT_RELEASE) -m64
	@$(LN) bin/patch_spliter.o -o bin/patch_spliter-$(@)-$(VERSION) -D'ARCH="$(@)"' -D'VERSION="$(VERSION)"' $(OPT_RELEASE) -m64
	@ln -sf patch_spliter-$(@)-$(VERSION) bin/patch_spliter
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
x64dbg: patch_spliter.cpp Makefile
	@if [ ! -e bin ]; then    (mkdir bin;)     fi
	@$(CXX) patch_spliter.cpp -c -o bin/patch_spliter.o -D'ARCH="$(@)"' -D'VERSION="$(VERSION)"' $(OPT_DEBUG) -m64
	@$(LN) bin/patch_spliter.o -o bin/patch_spliter-$(@)-$(VERSION) -D'ARCH="$(@)"' -D'VERSION="$(VERSION)"' $(OPT_DEBUG) -m64
	@ln -sf patch_spliter-$(@)-$(VERSION) bin/patch_spliter
	@objdump -Dslx bin/patch_spliter-$(@)-$(VERSION) > bin/patch_spliter-$(@)-$(VERSION).dump;
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
clean:
	@if [ -e bin ]; then    rm -rf bin;    fi
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
