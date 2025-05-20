BUILDDIR := build

# 指定交叉编译器路径
CC  := /usr/bin/aarch64-linux-gnu-gcc
CXX := /usr/bin/aarch64-linux-gnu-g++

.PHONY: all clean run rebuild

all:
	cmake -S . -B $(BUILDDIR) \
		-DCMAKE_C_COMPILER="$(CC)" \
		-DCMAKE_CXX_COMPILER="$(CXX)" \
		-DCMAKE_SYSTEM_NAME=Linux \
		-DCMAKE_SYSTEM_PROCESSOR=aarch64 \
		-G "Unix Makefiles"
	cmake --build $(BUILDDIR)

run: all
	./$(BUILDDIR)/application

clean: 
	rm -rf $(BUILDDIR)

rebuild: clean all