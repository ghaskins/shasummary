NAME=shasummary

OPTIMIZE ?= -O3
CFLAGS += -g $(OPTIMIZE) --std=c++11 -DBOOST_FILESYSTEM_VERSION=3 $(EXTRA_CFLAGS)
ARCH=$(shell uname -m)
OS:=$(shell uname)
BINDIR ?= /usr/bin
OBJDIR ?= obj/$(ARCH)
TESTDIR ?= obj/test

ifeq ($(OS),Darwin)
BOOST_POSTFIX=-mt
endif

LIBRARIES += -lboost_program_options$(BOOST_POSTFIX)
LIBRARIES += -lboost_filesystem$(BOOST_POSTFIX)
LIBRARIES += -lboost_system$(BOOST_POSTFIX)
LIBRARIES += -lboost_thread$(BOOST_POSTFIX)

SRCS += $(wildcard *.cc)
HDRS += $(wildcard *.hh)

OBJS = $(patsubst %.cc,$(OBJDIR)/%.o,$(notdir $(SRCS)))

TESTFILE += foo1.dat
TESTFILE += foo2.dat
TESTFILE += foo3.dat
TESTFILE += foo4.dat

TESTOBJS = $(patsubst %.dat,$(TESTDIR)/%.dat,$(TESTFILE))

OUTPUT = $(OBJDIR)/$(NAME)

all: $(OUTPUT)

$(OUTPUT): $(OBJS)

$(OBJS): Makefile $(HDRS)

$(OBJDIR)/%.o: %.cc Makefile
	@echo "Compiling (C++) $< to $@"
	@mkdir -p $(OBJDIR)
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/%:
	@echo "Linking $@"
	@mkdir -p $(OBJDIR)
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ $(filter %.o %.a,$+) $(LIBDIR) $(LIBRARIES)

$(TESTDIR)/%.dat: Makefile
	@echo "Generating test data $@"
	@mkdir -p $(TESTDIR)
	@uuidgen > $@

$(PREFIX)$(BINDIR):
	mkdir -p $@

install: $(OUTPUT) $(PREFIX)$(BINDIR)
	cp $(OUTPUT) $(PREFIX)$(BINDIR)

testdata: $(TESTOBJS)

clean: 
	-rm -rf obj
	-rm *~


