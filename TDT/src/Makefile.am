bin_PROGRAMS=tdt_server

tdt_server_SOURCES= \
	TDT.cpp \
	TdtServantI.cpp \
	server.cpp

tdt_server_CXXFLAGS= \
	-I../common

tdt_server_LDADD= \
	-lIce \
	-lIceUtil \
	-lgsoap++ \
	-L../common -ltdtcommon

TDT.cpp: TDT.ice
	slice2cpp $<

clean-local:
	rm -f TDT.cpp TDT.h

#for test
TESTS=test

check_PROGRAMS=test

test_SOURCES= \
	test.cpp \
	TDT.cpp 

test_LDADD= \
	-lIce \
	-lIceUtil
