VERSION=0.0.2
#-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
OPT_DEBUG=  -std=c++0x -Wall -Wextra -Wlong-long -Wunused -O0 -ggdb -pedantic
OPT_RELEASE=-std=c++0x -Wall -Wextra -Wlong-long -Wunused -O777 -s

help:
	@echo "use make [x32 | x64 | x32dbg | x64dbg | clean]";

x32:    patch_spliter.cpp Makefile
	@if [ ! -e bin ]; then    (mkdir bin;)     fi
	@g++ patch_spliter.cpp -o bin/patch_spliter-$(@)-$(VERSION) -D'ARCH="$(@)"' -D'VERSION="$(VERSION)"' $(OPT_RELEASE) -m32
	@ln -sf patch_spliter-$(@)-$(VERSION) bin/patch_spliter

x32dbg: patch_spliter.cpp Makefile
	@if [ ! -e bin ]; then    (mkdir bin;)     fi
	@g++ patch_spliter.cpp -o bin/patch_spliter-$(@)-$(VERSION) -D'ARCH="$(@)"' -D'VERSION="$(VERSION)"' $(OPT_DEBUG) -m32
	@ln -sf patch_spliter-$(@)-$(VERSION) bin/patch_spliter
	@objdump -Dslx bin/patch_spliter-$(@)-$(VERSION) > bin/patch_spliter-$(@)-$(VERSION).dump;

x64: patch_spliter.cpp Makefile
	@if [ ! -e bin ]; then    (mkdir bin;)     fi
	@g++ patch_spliter.cpp -o bin/patch_spliter-$(@)-$(VERSION) -D'ARCH="$(@)"' -D'VERSION="$(VERSION)"' $(OPT_RELEASE) -m64
	@ln -sf patch_spliter-$(@)-$(VERSION) bin/patch_spliter

x64dbg: patch_spliter.cpp Makefile
	@if [ ! -e bin ]; then    (mkdir bin;)     fi
	@g++ patch_spliter.cpp -o bin/patch_spliter-$(@)-$(VERSION) -D'ARCH="$(@)"' -D'VERSION="$(VERSION)"' $(OPT_DEBUG) -m64
	@ln -sf patch_spliter-$(@)-$(VERSION) bin/patch_spliter
	@objdump -Dslx bin/patch_spliter-$(@)-$(VERSION) > bin/patch_spliter-$(@)-$(VERSION).dump;

clean:
	@if [ -e bin ]; then    rm -rf bin;    fi
