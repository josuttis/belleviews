default: modall.win

include Makefile.h
	
###########################################
# generic rules for special module suffixes
###########################################

bel: belle
belle:: testall.20 testdrop.20 testtake.20 testdropwhile.20 testfilter.20 testtransform.20
belle:: testsub.20
belle:: testdropwhile.20

bel.win: belle.win
belle.win:: testall.winL testdrop.winL testtake.winL testdropwhile.winL testfilter.winL testtransform.winL
belle.win:: testsub.winL
belle.win:: testdropwhile.winL

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

