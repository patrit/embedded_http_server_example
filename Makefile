appname := server

CXX := clang++
CC := clang
CXXFLAGS := -std=c++17 -I/opt/ibm/db2/include
CCFLAGS := -std=c17 -I/opt/ibm/db2/include
LDLIBS := -lPocoNet -lPocoUtil -lPocoFoundation -L/opt/ibm/db2/lib64 -ldb2

cfiles := $(shell find . -name "*.c")
cppfiles := $(shell find . -name "*.cpp")
sqxfiles := $(shell find . -name "*.sqx")
objects  := $(patsubst %.c, %.o, $(cfiles)) $(patsubst %.cpp, %.o, $(cppfiles)) $(patsubst %.sqx, %.o, $(sqxfiles))

all: $(appname)

$(appname): $(objects)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(appname) $(objects) $(LDLIBS)

depend: .depend

.depend_c: $(cfiles)
	rm -f ./.depend
	$(CC) $(CCFLAGS) -MM $^>>./.depend_c;

.depend_cpp: $(cppfiles)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend_cpp;

clean:
	rm -f $(objects)

dist-clean: clean
	rm -f *~ .depend_c .depend_cpp

format:
	clang-format -i $(shell find . -name "*.c" -o -name "*.cpp" -o -name "*.h")

_assertdbupdated:
	@echo "Assert that the DB is up to date e.g. flyway"

LEVEL := $(shell git rev-list -n 1 HEAD | head -c 8)
DBRMVER := $(shell git rev-list -n 1 HEAD)

%.C: %.sqx _assertdbupdated
	(./db2_wrapper.sh "db2 prep $< bindfile QUALIFIER DUDE PACKAGE \
	              USING $(shell ./gendbrm.sh $<) \
	              TARGET CPLUSPLUS COLLECTION SAMPLE LEVEL \"$(LEVEL)\" VERSION \"$(DBRMVER)\"" > $@.log; RESULT=$$?; \
	if [ $$RESULT -eq 0 ]; then sed -i -e "s/^ 172,0,65,69,65,/ static_cast<char>(172),0,65,69,65,/g" \
                                       -e "/static const short sqlIsLiteral/d" \
                                       -e "/static const short sqlIsInputHvar/d" $@; fi; \
	if [ $$RESULT -ne 0 ]; then [ -e $@.log ] && cat $@.log; fi; rm -f $@.log; exit $$RESULT;);

include .depend_c .depend_cpp
