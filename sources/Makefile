default: modall.win

include ../Makefile.h
	
###########################################
# generic rules for special module suffixes
###########################################

.SUFFIXES: .cppm .cppp .obj .ixx
.ixx.obj:
	@echo ""
	cl $(CXXFLAGSWINL) /TP /c /interface $*.ixx $(IFCDIRFLAGS)
	@echo "=== $*.obj done"
	@echo ""
.cppm.obj:
	@echo ""
	cl $(CXXFLAGSWINL) /TP /c /interface $*.cppm $(IFCDIRFLAGS)
	@echo "=== $*.obj done"
	@echo ""
.cppp.obj:
	@echo ""
	cl $(CXXFLAGSWINL) /TP /c /internalPartition $*.cppp $(IFCDIRFLAGS)
	@echo "=== $*.obj done"
	@echo ""
.cpp.obj:
	@echo ""
	cl $(CXXFLAGSWINL) /c $*.cpp $(IFCDIRFLAGS)
	@echo "=== $*.obj done"
	@echo ""

###########################################
# targets for Visual C++ and GCC
###########################################

modall.win: modall_part.cppm modall_ifpart.cppm modall_if.cppm modall_impl.cpp modall_test.cpp
	@echo ""
	@echo "=== COMPILE & LINK:"
	../clmod.py $(CXXFLAGSWINL) modall_part.cppm modall_ifpart.cppm modall_if.cppm modall_impl.cpp modall_test.cpp $(LDFLAGSWIN) /Femodall.exe
	#rm -f *.obj *.ifc
	@echo "- OK:  modall.exe done"

modall.gcc: modall_part.cppm modall_ifpart.cppm modall_if.cppm modall_impl.cpp modall_test.cpp
	@echo ""
	@echo "=== COMPILE & LINK:"
	$(CXX20) $(CXXFLAGS20) -xc++ modall_part.cppm -xc++ modall_ifpart.cppm -xc++ modall_if.cppm modall_impl.cpp modall_test.cpp $(LDFLAGS20) -o modall.exe
	#rm -rf *.o gcm.cache
	@echo "- OK:  modall.exe done"

