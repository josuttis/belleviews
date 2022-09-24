
#==========================================================
# General Flags
#==========================================================

#GCCFLAGS=-g -Wall -ansi -fhonor-std
#GCCFLAGS=-g -ansi -W -Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wtraditional -pedantic
GCCWARNFLAGS = --pedantic --pedantic-errors -Wall -Wextra -Wnon-virtual-dtor -Wconversion -D_GLIBCXX_DEBUG
FLAGSUNUSED = -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable
GCCFLAGS = -O2 $(GCCWARNFLAGS) $(FLAGSUNUSED)
GCCFLAGSNOWARN = -O2 $(FLAGSUNUSED)
LDFLAGS = -O2

#----------------------------------------------------
# Default: std C++20 with gcc 11.0

PATHDEF = /cygdrive/p/gcc/gcc110
CXX = $(PATHDEF)/bin/g++110
CXXFLAGS = --std=c++20 $(GCCFLAGS)
CXXFLAGSNOWARN = --std=c++20 $(GCCFLAGSNOWARN)
LDFLAGS = -latomic

.cpp:
	$(CXX) $(CXXFLAGSNOWARN) $(INCLUDES) $*.cpp $(LDFLAGS20) -o $*raw.exe
	@echo PATH=\"$(PATH20)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $*  calls  $*raw.exe"
.cxx:
	sed -f $(TTT2CPP) < $< > $*.cpp
	$(CXX) $(CXXFLAGSNOWARN) $(INCLUDES) $*.cpp $(LDFLAGS) -o $*raw.exe
	@echo PATH=\"$(PATHDEF)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	rm $*.cpp
	@echo "- OK:  $*  calls  $*raw.exe"


#==========================================================
# gcc compiler-specific targets
#==========================================================

#----------------------------------------------------
# gcc 4.8 (with C++11)

.SUFFIXES: .48
PATH48 = /cygdrive/p/gcc/gcc48
CXX48 = $(PATH48)/bin/g++48
CXXFLAGS48 = --std=c++11 $(GCCFLAGS)
#LDFLAGS48 = -latomic
LDFLAGS48 =
.cpp.48:
	$(CXX48) $(CXXFLAGS48) $(INCLUDES) $*.cpp $(LDFLAGS48) -o $*48raw.exe
	@echo PATH=\"$(PATH48)/bin:\$$PATH\" ./$*48raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* calls  $*48raw.exe"
.cxx.48:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	$(CXX72) $(CXXFLAGS48) $(INCLUDES) $*.cpp $(LDFLAGS48) -o $*48raw.exe
	@echo PATH=\"$(PATH48)/bin:\$$PATH\" ./$*48raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* calls  $*48raw.exe"
	rm $*.cpp

#----------------------------------------------------
# gcc 6.1 (with C++14):

.SUFFIXES: .61
PATH61 = /cygdrive/p/gcc/gcc61
CXX61 = $(PATH61)/bin/g++61
CXXFLAGS61 = --std=c++14 $(GCCFLAGS)
LDFLAGS61 = -latomic
.cpp.61:
	$(CXX61) $(CXXFLAGS61) $(INCLUDES) $*.cpp $(LDFLAGS61) -o $*61raw.exe
	@echo PATH=\"$(PATH61)/bin:\$$PATH\" ./$*61raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* calls  $*61raw.exe"
.cxx.61:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	$(CXX72) $(CXXFLAGS61) $(INCLUDES) $*.cpp $(LDFLAGS61) -o $*61raw.exe
	@echo PATH=\"$(PATH61)/bin:\$$PATH\" ./$*61raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* calls  $*61raw.exe"
	rm $*.cpp

#----------------------------------------------------
# gcc 7.2 (with C++17):

.SUFFIXES: .72
PATH72 = /cygdrive/p/gcc/gcc72
CXX72 = $(PATH72)/bin/g++72
CXXFLAGS72 = --std=c++17 $(GCCFLAGS)
LDFLAGS72 = -latomic
.cpp.72:
	$(CXX72) $(CXXFLAGS72) $(INCLUDES) $*.cpp $(LDFLAGS72) -o $*72raw.exe
	@echo PATH=\"$(PATH72)/bin:\$$PATH\" ./$*72raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* calls  $*72raw.exe"
.cxx.72:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	$(CXX72) $(CXXFLAGS72) $(INCLUDES) $*.cpp $(LDFLAGS72) -o $*72raw.exe
	@echo PATH=\"$(PATH72)/bin:\$$PATH\" ./$*72raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* calls  $*72raw.exe"
	rm $*.cpp

#----------------------------------------------------
# gcc 8.2 (with C++17):

.SUFFIXES: .82
PATH82 = /cygdrive/p/gcc/gcc82
CXX82 = $(PATH82)/bin/g++82
CXXFLAGS82 = --std=c++17 $(GCCFLAGS)
LDFLAGS82 = -latomic
.cpp.82:
	$(CXX82) $(CXXFLAGS82) $(INCLUDES) $*.cpp $(LDFLAGS82) -o $*82raw.exe
	@echo PATH=\"$(PATH82)/bin:\$$PATH\" ./$*82raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* calls  $*82raw.exe"
.cxx.82:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	$(CXX82) $(CXXFLAGS82) $(INCLUDES) $*.cpp $(LDFLAGS82) -o $*82raw.exe
	@echo PATH=\"$(PATH82)/bin:\$$PATH\" ./$*82raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* calls  $*82raw.exe"
	rm $*.cpp

#----------------------------------------------------
# gcc 9.2 (with C++17):

.SUFFIXES: .92
PATH92 = /cygdrive/p/gcc/gcc92
CXX92 = $(PATH92)/bin/g++92
CXXFLAGS92 = --std=c++17 $(GCCFLAGS)
LDFLAGS92 = -latomic
.cpp.92:
	$(CXX92) $(CXXFLAGS92) $(INCLUDES) $*.cpp $(LDFLAGS92) -o $*92raw.exe
	@echo PATH=\"$(PATH92)/bin:\$$PATH\" ./$*92raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* calls  $*92raw.exe"
.cxx.92:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	$(CXX92) $(CXXFLAGS92) $(INCLUDES) $*.cpp $(LDFLAGS92) -o $*92raw.exe
	@echo PATH=\"$(PATH92)/bin:\$$PATH\" ./$*92raw.exe '$$*' > $*.exe
	@chmod +x $*.exe 
	@echo "- OK:  $* calls  $*92raw.exe"
	rm $*.cpp

#----------------------------------------------------
# gcc 10.1 (with C++20):

.SUFFIXES: .101
PATH101 = /cygdrive/p/gcc/gcc101
CXX101 = $(PATH92)/bin/g++101
CXXFLAGS101 = --std=c++2a $(GCCFLAGS)
LDFLAGS101 = -latomic
.cpp.101:
	$(CXX101) $(CXXFLAGS101) $(INCLUDES) $*.cpp $(LDFLAGS101) -o $*101raw.exe
	@echo PATH=\"$(PATH101)/bin:\$$PATH\" ./$*101raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* calls  $*101raw.exe"
.cxx.101:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	$(CXX101) $(CXXFLAGS101) $(INCLUDES) $*.cpp $(LDFLAGS101) -o $*101raw.exe
	@echo PATH=\"$(PATH101)/bin:\$$PATH\" ./$*101raw.exe '$$*' > $*.exe
	@chmod +x $*.exe 
	@echo "- OK:  $* calls  $*101raw.exe"
	rm $*.cpp

#----------------------------------------------------
# gcc 11.0 (with C++20):

.SUFFIXES: .110
PATH110 = /cygdrive/p/gcc/gcc110
CXX110 = $(PATH92)/bin/g++110
CXXFLAGS110 = --std=c++2a $(GCCFLAGS)
LDFLAGS110 = -latomic
.cpp.110:
	$(CXX110) $(CXXFLAGS110) $(INCLUDES) $*.cpp $(LDFLAGS110) -o $*110raw.exe
	@echo PATH=\"$(PATH110)/bin:\$$PATH\" ./$*110raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* calls  $*110raw.exe"
.cxx.110:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	$(CXX92) $(CXXFLAGS110) $(INCLUDES) $*.cpp $(LDFLAGS110) -o $*110raw.exe
	@echo PATH=\"$(PATH110)/bin:\$$PATH\" ./$*110raw.exe '$$*' > $*.exe
	@chmod +x $*.exe 
	@echo "- OK:  $* calls  $*110raw.exe"
	rm $*.cpp

#----------------------------------------------------
# gcc 12.0 (with C++20):

.SUFFIXES: .12
PATH12 = /cygdrive/p/gcc/gcc12
CXX12 = $(PATH92)/bin/g++12
CXXFLAGS12 = --std=c++2a $(GCCFLAGS)
LDFLAGS12 = -latomic
.cpp.12:
	$(CXX12) $(CXXFLAGS12) $(INCLUDES) $*.cpp $(LDFLAGS12) -o $*12raw.exe
	@echo PATH=\"$(PATH12)/bin:\$$PATH\" ./$*12raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* calls  $*101raw.exe"
.cxx.12:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	$(CXX12) $(CXXFLAGS12) $(INCLUDES) $*.cpp $(LDFLAGS12) -o $*12raw.exe
	@echo PATH=\"$(PATH101)/bin:\$$PATH\" ./$*101raw.exe '$$*' > $*.exe
	@chmod +x $*.exe 
	@echo "- OK:  $* calls  $*101raw.exe"
	rm $*.cpp


#==========================================================
# gcc C++-specific targets
#==========================================================

#----------------------------------------------------
# gcc with C++03 mode:
.SUFFIXES: .03
PATH03 = /cygdrive/p/gcc/gcc48
CXX03 = $(PATH03)/bin/g++48
CXXFLAGS03 = --std=c++03 $(GCCFLAGS)
LDFLAGS03 =
.cpp.03:
	$(CXX03) $(CXXFLAGS03) $(INCLUDES) $*.cpp $(LDFLAGS03) -o $*03raw.exe
	@echo PATH=\"$(PATH03)/bin:\$$PATH\" ./$*03raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* and $*03  call  $*03raw.exe"
.cxx.03:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	$(CXX03) $(CXXFLAGS03) $(INCLUDES) $*.cpp $(LDFLAGS03) -o $*03raw.exe
	@echo PATH=\"$(PATH03)/bin:\$$PATH\" ./$*03raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $* and $*03  call  $*03raw.exe"
	rm $*.cpp

#----------------------------------------------------
# gcc with C++11 mode:
.SUFFIXES: .11
PATH11 = /cygdrive/p/gcc/gcc72
CXX11 = $(PATH11)/bin/g++72
CXXFLAGS11 = --std=c++11 $(GCCFLAGS)
LDFLAGS11 =
.cpp.11:
	$(CXX11) $(CXXFLAGS11) $(INCLUDES) $*.cpp $(LDFLAGS11) -o $*raw.exe
	@echo PATH=\"$(PATH11)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $*  calls  $*raw.exe"
.cxx.11:
	sed -f $(TTT2CPP) < $< > $*.cpp
	$(CXX11) $(CXXFLAGS11) $(INCLUDES) $*.cpp $(LDFLAGS11) -o $*raw.exe
	@echo PATH=\"$(PATH11)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $*  calls  $*raw.exe"
	rm $*.cpp

#----------------------------------------------------
# gcc with C++14 mode:
.SUFFIXES: .14
PATH14 = /cygdrive/p/gcc/gcc72
CXX14 = $(PATH14)/bin/g++72
CXXFLAGS14 = --std=c++14 $(GCCFLAGS)
LDFLAGS14 =
.cpp.14:
	$(CXX14) $(CXXFLAGS14) $(INCLUDES) $*.cpp $(LDFLAGS14) -o $*raw.exe
	@echo PATH=\"$(PATH14)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $*  calls  $*raw.exe"
.cxx.14:
	sed -f $(TTT2CPP) < $< > $*.cpp
	$(CXX14) $(CXXFLAGS14) $(INCLUDES) $*.cpp $(LDFLAGS14) -o $*raw.exe
	@echo PATH=\"$(PATH14)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $*  calls  $*raw.exe"
	rm $*.cpp

#----------------------------------------------------
# gcc with C++17 mode:
.SUFFIXES: .17
PATH17 = /cygdrive/p/gcc/gcc92
CXX17 = $(PATH17)/bin/g++92
CXXFLAGS17 = --std=c++17 $(GCCFLAGS)
LDFLAGS17 = -latomic
.cpp.17:
	$(CXX92) $(CXXFLAGS17) $(INCLUDES) $*.cpp $(LDFLAGS17) -o $*raw.exe
	@echo PATH=\"$(PATH17)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $*  calls  $*raw.exe"
.cxx.17:
	sed -f $(TTT2CPP) < $< > $*.cpp
	$(CXX92) $(CXXFLAGS17) $(INCLUDES) $*.cpp $(LDFLAGS17) -o $*raw.exe
	@echo PATH=\"$(PATH17)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $*  calls  $*raw.exe"
	rm $*.cpp

#----------------------------------------------------
# gcc with C++20 mode with concepts:
.SUFFIXES: .20
PATH20 = /cygdrive/p/gcc/gcc120
CXX20 = $(PATH20)/bin/g++120
CXXFLAGS20 = --std=c++20 -fconcepts -fmodules-ts $(GCCFLAGS)
LDFLAGS20 = -latomic
.cpp.20:
	$(CXX20) $(CXXFLAGS20) $(INCLUDES) $*.cpp $(LDFLAGS20) -o $*raw.exe
	@echo PATH=\"$(PATH20)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	@echo "- OK:  $*  calls  $*raw.exe"
.cxx.20:
	sed -f $(TTT2CPP) < $< > $*.cpp
	$(CXX20) $(CXXFLAGS20) $(INCLUDES) $*.cpp $(LDFLAGS20) -o $*raw.exe
	@echo PATH=\"$(PATH20)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	rm $*.cpp
	@echo "- OK:  $*  calls  $*raw.exe"
.cxxm.20:
	sed -f $(TTT2CPP) < $< > $*.cppm
	$(CXX20) $(CXXFLAGS20) $(INCLUDES) $*.cpp $(LDFLAGS20) -o $*raw.exe
	@echo PATH=\"$(PATH20)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
	@chmod +x $*.exe
	rm $*.cpp
	@echo "- OK:  $*  calls  $*raw.exe"


#==========================================================
# clang
#==========================================================

#----------------------------------------------------
# clang 3.9 for Nico:
.SUFFIXES: .clang
CXXCLANG = clang++
CXXFLAGSCLANG = --std=c++14 $(GCCFLAGS)
LDFLAGSCLANG = -latomic
.cpp.clang:
	$(CXXCLANG) $(CXXFLAGSCLANG) $(INCLUDES) $*.cpp $(LDFLAGSCLANG) -o $*clangraw.exe
	@echo ./$*clangraw.exe '$$*' > $*clang.exe
	@echo ./$*clangraw.exe '$$*' > $*.exe
	@echo "- OK:  $* and $*clang  call  $*clangraw.exe"
.cxx.clang:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	$(CXXCLANG) $(CXXFLAGSCLANG) $(INCLUDES) $*.cpp $(LDFLAGSCLANG) -o $*clangraw.exe
	@echo ./$*clangraw.exe '$$*' > $*clang.exe
	@echo ./$*clangraw.exe '$$*' > $*.exe
	@echo "- OK:  $* and $*clang  call  $*clangraw.exe"

#----------------------------------------------------

# clang C++17
.SUFFIXES: .clang17
CXXCLANG17 = clang++
CXXFLAGSCLANG17 = --std=c++1z $(GCCFLAGS)
LDFLAGSCLANG17 = -latomic
.cpp.clang17:
	$(CXXCLANG17) $(CXXFLAGSCLANG17) $(INCLUDES) $*.cpp $(LDFLAGSCLANG17) -o $*clang17raw.exe
	@echo ./$*clang17raw.exe '$$*' > $*clang.exe
	@echo ./$*clang17raw.exe '$$*' > $*.exe
	@echo "- OK:  $* and $*clang  call  $*clang17raw.exe"
.cxx.clang17:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	$(CXXCLANG17) $(CXXFLAGSCLANG) $(INCLUDES) $*.cpp $(LDFLAGSCLANG) -o $*clang17raw.exe
	@echo ./$*clang17raw.exe '$$*' > $*clang.exe
	@echo ./$*clang17raw.exe '$$*' > $*.exe
	@echo "- OK:  $* and $*clang  call  $*clangraw.exe"


#----------------------------------------------------
# clang

#CXX = xcrun clang++
#CXXFLAGS = -std=c++11 -stdlib=libc++ -Wall
#LDFLAGS = -stdlib=libc++

#----------------------------------------------------
# Visual Studio: use suffix .win

WINFLAGS = /Ox /MT /EHsc /W4 /Zc:strictStrings /wd4189 /wd4101
CXXFLAGSWIN = $(WINFLAS)
.SUFFIXES: .win
.cpp.win:
	cl $(WINFLAGS) $*.cpp /Fe$*

#----------------------------------------------------
# Visual Studio: use suffix .win17

WIN17FLAGS = /std:c++17 /permissive- $(WINFLAGS)
CXXFLAGSWIN17 = $(WIN17FLAGS)
.SUFFIXES: .win17
.cpp.win17:
	cl $(WIN17FLAGS) $*.cpp /Fe$*

#----------------------------------------------------
# Visual Studio: use suffix .win20

WIN20FLAGS = /std:c++20 /permissive- $(WINFLAGS)
CXXFLAGSWIN20 = $(WIN17FLAGS)
.SUFFIXES: .win20
.cxx.win20:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	cl $(WIN20FLAGS) $*.cpp /Fe$*
.cxxm.win20:
	sed -f $(TTT2CPP) < $*.cxxm > $*.cppm
	cl $(WIN20FLAGS) $*.cppm /Fe$*

#----------------------------------------------------
# Visual Studio: use suffix .winL

WINLFLAGS = /std:c++latest /permissive- $(WINFLAGS)
CXXFLAGSWINL = $(WINLFLAGS)
.SUFFIXES: .winL
.cxx.winL:
	sed -f $(TTT2CPP) < $*.cxx > $*.cpp
	cl $(WINLFLAGS) $*.cpp /Fe$*
.cxxm.winL:
	sed -f $(TTT2CPP) < $*.cxxm > $*.cppm
	cl $(WINLFLAGS) $*.cppm /Fe$*


#----------------------------------------------------


TTT2CPP = ../ttt2cpp.sed

help::
	@echo "all:    progs"

all:: headers progs
html:: headers cpp

# .cxx: TeX C/C++ Files (new style)
# .hxx: TeX header files (new style)
# .ott: generated output
# .ctt: TeX C/C++ Files
# .htt: TeX header files
# .itt: stdin
# .ott: generated stdout
# .argtt: command line args
.SUFFIXES: .ctt .htt .cxx .hxx .cxxm .cpp .hpp .itt .ott .argtt .oxx .cppm

.hxx.hpp:
	sed -f $(TTT2CPP) < $< > $*.hpp
.cxx.cpp:
	sed -f $(TTT2CPP) < $< > $*.cpp
.cxxm.cppm:
	sed -f $(TTT2CPP) < $< > $*.cppm
.htt.hpp:
	sed -f $(TTT2CPP) < $< > $*.hpp
.ctt.cpp:
	sed -f $(TTT2CPP) < $< > $*.cpp
#.cpp:
#	$(CXX) $(CXXFLAGS) $(INCLUDES) $*.cpp $(LDFLAGS) -o $*raw.exe
#	@echo PATH=\"$(PATH)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
#	@echo PATH=\"$(PATH)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
#	@echo "- OK:  $*  calls  $*raw.exe"

#FS_INCL=/cygdrive/p/boost/cygwin/include
##FS_LIB=/cygdrive/p/boost/cygwin/lib -lboost_filesystem -lboost_system -static
##FS_LIB=/cygdrive/p/boost/cygwin/lib libboost_filesystem.a libboost_system.a
#FS_LIB=/cygdrive/p/boost/cygwin/lib -Wl,-Bstatic -lboost_system -lboost_filesystem -lboost_system -Wl,-Bdynamic
#.cxx:
#	sed -f $(TTT2CPP) < $< > $*.cpp
#	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(FS_INCL) $*.cpp $(LDFLAGS) -L$(FS_LIB) -o $*raw.exe
#	@echo PATH=\"$(PATHDEF)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
#	@echo PATH=\"$(PATHDEF)/bin:\$$PATH\" ./$*raw.exe '$$*' > $*.exe
#	@echo "- OK:  $*  calls  $*raw.exe"
#	rm $*.cpp


############ clean and delete ##########################
help::
	@echo 'clean:  remove all except generated executables'
	@echo 'delete: remove every generated file'
clean::
	rm -rf *.o *.obj *.exe *~
	rm -rf *.ifc gcm.cache

delete:: clean
	rm -rf *.exe

