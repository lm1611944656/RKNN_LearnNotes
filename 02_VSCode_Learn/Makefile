BUILDDIR := build

# 指定你要使用的编译器路径
CC  := E:/Tools/C_CPP_env/mingw_w64/install/mingw64/bin/gcc.exe
CXX := E:/Tools/C_CPP_env/mingw_w64/install/mingw64/bin/g++.exe

.PHONY: all clean run rebuild

# 指定生成器
all:
	cmake -S . -B $(BUILDDIR) \
		-DCMAKE_C_COMPILER="$(CC)" \
		-DCMAKE_CXX_COMPILER="$(CXX)" \
		-G "MinGW Makefiles"				
	cmake --build $(BUILDDIR)

run: all
	./$(BUILDDIR)/application

clean:
	rmdir /S /Q $(BUILDDIR)/*

rebuild: clean all