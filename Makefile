INC_DIR = ./
CXX		=g++
LD		=g++
CXXFLAGS	=-O2 -ggdb -std=gnu++0x -Wall
LDFLAGS		=-L/cvmfs/cms.cern.ch/slc6_amd64_gcc481/cms/cmssw/CMSSW_7_2_1/external/slc6_amd64_gcc481/lib/ -lz -lm
SOFLAGS		=-fPIC -shared 
SHELL		=bash
###
SrcSuf        = cpp
HeadSuf       = hpp
ObjSuf        = o
DepSuf        = d
DllSuf        = so
StatSuf       = a

## Packages	=controller testDataTypeServer testDataTypeClient testDataType testXMLLib testUInt
## Objects		=Daemon EventBuilder Handler Logger Profiler  Configurator ControlManager ConnectionManager Utility HwManager AsyncUtils BoardConfig

CppTestFiles=$(wildcard test/*.$(SrcSuf))
Packages=$(patsubst test/%.$(SrcSuf),%,$(CppTestFiles) )

CppSrcFiles=$(wildcard src/*.$(SrcSuf))
Objects=$(patsubst src/%.$(SrcSuf),%,$(CppSrcFiles))

LibName		=H4DQM

### ----- OPTIONS ABOVE ----- ####

#InfoLine = \033[01\;31m compiling $(1) \033[00m
InfoLine = compiling $(1)

BASEDIR=$(shell pwd)
BINDIR=$(BASEDIR)/bin
SRCDIR = $(BASEDIR)/src
HDIR = $(BASEDIR)/interface

### Main Target, first
.PHONY: all
all: info $(Packages) | $(BINDIR)

CXXFLAGS	+=`root-config --cflags`
LDFLAGS 	+=`root-config --libs`

BINOBJ	=$(patsubst %,$(BINDIR)/%.$(ObjSuf),$(Objects) )
SRCFILES=$(patsubst %,$(SRCDIR)/%.$(SrcSuf),$(Objects) )
HFILES	=$(patsubst %,$(HDIR)/%.$(HeadSuf),$(Objects) )
StatLib		=$(BINDIR)/H4DQM.$(StatSuf)
SoLib		=$(BINDIR)/H4DQM.$(DllSuf)

.PRECIOUS:*.ObjSuf *.DepSuf *.DllSuf

Deps=$(patsubst %,$(BINDIR)/%.$(DepSuf),$(Objects) $(Packages) )

############### EXPLICIT RULES ###############

$(BINDIR):
	mkdir -p $(BINDIR)

info:
	@echo "--------------------------"
	@echo "Compile on $(shell hostname)"
	@echo "Packages are: $(Packages)"
	@echo "Objects are: $(Objects)"
	@echo "--------------------------"
	@echo "DEBUG:"

$(StatLib): $(BINOBJ)
	ar rcs $@ $(BINOBJ)
.PHONY: soLib
soLib: $(SoLib)

$(SoLib): $(StatLib)
	$(LD) $(LDFLAGS) $(SOFLAGS) -o $@ $^

.PHONY: $(Packages) 
$(Packages): % : $(BINDIR)/% | $(BINDIR)
	@echo $(call InfoLine , $@ )

#$(BINDIR)/$(Packages): $(BINDIR)/% : $(BASEDIR)/test/%.$(SrcSuf) $(StatLib) | $(BINDIR)
$(addprefix $(BINDIR)/,$(Packages)): $(BINDIR)/% : $(BASEDIR)/test/%.$(SrcSuf) $(StatLib) | $(BINDIR)
	@echo $(call InfoLine , $@ )
	$(CXX) -o $@ $< $(StatLib) -I$(INC_DIR) -I$(HDIR) $(CXXFLAGS) $(LDFLAGS)

#make this function of $(Packages)
#.PHONY: controller
#controller: bin/controller
#bin/controller: test/controller.cpp $(BINOBJ) $(StatLib)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(BINOBJ) $< 


.PHONY: clean
clean:
	-rm -v bin/controller
	-rm -v bin/readBinary
	-rm -v bin/*.$(ObjSuf)
	-rm -v bin/*.$(DllSuf)
	-rm -v bin/*.$(StatSuf)


############### IMPLICIT RULES ###############



#.o
%.$(ObjSuf): $(BINDIR)/%.$(ObjSuf)

#.o
$(BINDIR)/%.$(ObjSuf): $(SRCDIR)/%.$(SrcSuf) $(HDIR)/%.$(HeadSuf)
	@echo $(call InfoLine , $@ )
	$(CXX)  -c -o $@ $(SRCDIR)/$*.$(SrcSuf) -I$(INC_DIR) -I$(HDIR) $(CXXFLAGS) $(LDFLAGS)

#.d
$(BINDIR)/%.$(DepSuf): $(SRCDIR)/%.$(SrcSuf) $(HDIR)/%.$(HeadSuf)
	@echo $(call InfoLine , $@ )
	$(CXX)   -M -o $@ $(SRCDIR)/$*.$(SrcSuf) -I$(INC_DIR) -I$(HDIR) $(CXXFLAGS) $(LDFLAGS)
	sed -i'' "s|^.*:|& Makefile $(BINDIR)/&|g" $@

#-include $(Deps)
#	%.d: %.c
#		$(SHELL) -ec '$(CC) -M \
#			$(CPPFLAGS) $< | \
#			sed '\''s/$*.o/& $@/g'\'' \
#			> $@'
#	
