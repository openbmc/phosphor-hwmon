LIBS += libphosphor-hwmon.so
libphosphor-hwmon.so_OBJS += argument.o
libphosphor-hwmon.so_OBJS += directory.o
libphosphor-hwmon.so_OBJS += sensorset.o

EXES += phosphor-hwmon-readd
phosphor-hwmon-readd_OBJS += readd.o
phosphor-hwmon-readd_LIBS += phosphor-hwmon

#TODO: Issue#1 - Add the write-daemon for fan, pwm control.
#EXES += phosphor-hwmon-writed
#phosphor-hwmon-writed_OBJS += writed.o
#phosphor-hwmon-writed_LIBS += phosphor-hwmon

#### -----------------------------------------------------------------------####
#                                                                              #
##                       Compilare Regulas Sequi                              ##
#                                                                              #
#### -----------------------------------------------------------------------####

CXXFLAGS ?= -O3 -g -pipe
CXXFLAGS += --std=gnu++14 -Wall -Werror -flto -fPIC

define __BUILD_EXE
$1 : $$($1_OBJS) $$(LIBS)
		$$(LINK.cpp) -o $$@ $$^

#include $$($1_OBJS:.o=.d)
endef

$(foreach exe,$(EXES),$(eval $(call __BUILD_EXE,$(exe))))

define __BUILD_LIB
$1 : $$($1_OBJS)
		$$(LINK.cpp) -shared -o $$@ $$^

#include $$($1_OBJS:.o=.d)
endef

$(foreach lib,$(LIBS),$(eval $(call __BUILD_LIB,$(lib))))

.PHONY: clean
clean:
		$(RM) $(foreach exe,$(EXES),$(exe) $($(exe)_OBJS)) \
			  $(foreach lib,$(LIBS),$(lib) $($(lib)_OBJS))

DESTDIR ?= /
BINDIR ?= /usr/bin
LIBDIR ?= /usr/lib

.PHONY: install
install:
		install -m 0755 -d $(DESTDIR)$(BINDIR)
		install -m 0755 $(EXES) $(DESTDIR)$(BINDIR)
		install -m 0755 -d $(DESTDIR)$(LIBDIR)
		install -m 0755 $(LIBS) $(DESTDIR)$(LIBDIR)

.DEFAULT_GOAL: all
.PHONY: all
all: $(EXES) $(LIBS)
