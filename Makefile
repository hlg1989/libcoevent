
# gcc compiler
MAKE = make
CC  = gcc
CPP = g++
LD  = ld
AR  = ar

# target
BIN_DIR = ./bin
TARGET_SO_FILE_NAME = libcoevent.so
TARGET_A_FILE_NAME = libcoevent.a
TARGET_SO = $(BIN_DIR)/$(TARGET_SO_FILE_NAME)
TARGET_A = $(BIN_DIR)/$(TARGET_A_FILE_NAME)

# directories
LIBCO_DIR = ./libco_from_git
LIBCO_BIN = $(LIBCO_DIR)/lib
LIBCO_A_FILE_NAME = libcolib.a
LIBCO_HEADER_FILE_NAME = co_routine.h
LIBCO_HEADER = $(LIBCO_DIR)/$(LIBCO_HEADER_FILE_NAME)
LIBCO_TARGET = $(LIBCO_BIN)/$(LIBCO_A_FILE_NAME)
LIBCO_GIT_URL = https://github.com/Tencent/libco.git

# install parameters
LIBCOEVENT_LIB_PATH_CONF_DIR = /etc/ld.so.conf.d/
LIBCOEVENT_LIB_DIR_NAME = libcoevent.conf
libdir ?= ./Makefile
LIBCOEVENT_LIB_PATH = $(libdir)
LIBCOEVENT_LIB_HEADER_PATH = /usr/include


# flagsst 
CFLAGS += -Wall -g -fPIC -lpthread -I./include -I./src -I./$(LIBCO_DIR) -levent
CPPFLAGS += $(CFLAGS)
LDFLAGS += -lpthread -lm -lrt

# source files
C_SRCS = $(wildcard ./src/*.c)
CPP_SRCS = $(wildcard ./src/*.cpp)
ASM_SRCS = $(wildcard ./src/*.S)

C_OBJS = $(C_SRCS:.c=.c.o)
CPP_OBJS = $(CPP_SRCS:.cpp=.cpp.o)
ASM_OBJS = $(ASM_SRCS:.S=.S.o)

NULL ?=#
ifneq ($(strip $(CPP_OBJS)), $(NULL))
FINAL_CC = $(CPP)
else
FINAL_CC = $(CC)
CPPFLAGS = $(CFLAGS)
endif

export FINAL_CC
export NULL
export CPPFLAGS
export CFLAGS
export CC
export CPP
export LD

# default target
.PHONY:all
all: $(LIBCO_TARGET) $(TARGET_SO) $(TARGET_A)
	@echo "	<< libcoevent made >>"

# install
.PHONY:install
install:
	@if [ ! -d $(LIBCOEVENT_LIB_PATH) ]; then\
		echo "Please specify parameter: libdir";\
		exit 1;\
	fi
	@echo "# libcoevent default configuration" > $(LIBCOEVENT_LIB_PATH_CONF_DIR)/$(LIBCOEVENT_LIB_DIR_NAME)
	@echo "$(LIBCOEVENT_LIB_PATH)" >> $(LIBCOEVENT_LIB_PATH_CONF_DIR)/$(LIBCOEVENT_LIB_DIR_NAME)
	install -c $(TARGET_SO) $(LIBCOEVENT_LIB_PATH)
	install -c $(TARGET_A) $(LIBCOEVENT_LIB_PATH)
	install -c $(LIBCO_TARGET) $(LIBCOEVENT_LIB_PATH)
	@ls include | xargs -I [] install -m 0664 include/[] $(LIBCOEVENT_LIB_HEADER_PATH)
	@install -m 0664 $(LIBCO_HEADER) $(LIBCOEVENT_LIB_HEADER_PATH)
	@echo "<< libcoevent installed >>"

# uninstall
.PHONY:uninstall
uninstall:
	@if [ ! -d $(LIBCOEVENT_LIB_PATH) ]; then\
		echo "Please specify parameter: libdir";\
		exit 1;\
	fi
	-rm -f $(LIBCOEVENT_LIB_PATH)/$(TARGET_SO_FILE_NAME)
	-rm -f $(LIBCOEVENT_LIB_PATH)/$(TARGET_A_FILE_NAME)
	-rm -f $(LIBCOEVENT_LIB_PATH_CONF_DIR)/$(LIBCOEVENT_LIB_DIR_NAME)
	-rm -f $(LIBCOEVENT_LIB_PATH_CONF_DIR)/$(LIBCO_A_FILE_NAME)
	-@ls include | xargs -I [] rm -f $(LIBCOEVENT_LIB_HEADER_PATH)/[]
	-@rm -f $(LIBCOEVENT_LIB_HEADER_PATH)/$(LIBCO_HEADER_FILE_NAME)
	@echo "<< libcoevent uninstalled >>"

# libcoevent
$(TARGET_SO): $(BIN_DIR) $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS)
#	@echo ".so deps = "$^
	$(FINAL_CC) -o $@ $(CPP_OBJS) $(CPPFLAGS) -shared

$(TARGET_A): $(BIN_DIR) $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS)
#	@echo ".a deps = "$^
	$(AR) r $@ $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS)

$(BIN_DIR):
	mkdir $(BIN_DIR)

# libco
$(LIBCO_DIR):
	git clone 'https://github.com/Tencent/libco.git' $(LIBCO_DIR)

$(LIBCO_TARGET): $(LIBCO_DIR)
	if [ ! -f $(LIBCO_TARGET) ]; then\
		make -C $(LIBCO_DIR);\
	fi

# automatic compiler
-include $(C_OBJS:.o=.d)
-include $(CPP_OBJS:.o=.d)

%.cpp.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $*.cpp -o $*.cpp.o
	@$(CPP) -MM $(CPPFLAGS) $*.cpp > $*.cpp.d
	@mv -f $*.cpp.d $*.cpp.d.tmp
	@sed -e 's|.*:|$*.cpp.o:|' < $*.cpp.d.tmp > $*.cpp.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.cpp.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.cpp.d
	@rm -f $*.cpp.d.tmp

%.c.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.c.o
	@$(CC) -MM $(CFLAGS) $*.c > $*.c.d
	@mv -f $*.c.d $*.c.d.tmp
	@sed -e 's|.*:|$*.c.o:|' < $*.c.d.tmp > $*.c.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.c.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.c.d
	@rm -f $*.c.d.tmp

%.S.o: %.S
	$(CC) -c $*.S $*.S.o

#$(PROG_NAME): $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS)
#	@echo "$(LD) -r -o $@.o *.o"
#	@$(LD) -r -o $@.o $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS)
#	$(FINAL_CC) $@.o $(STATIC_LIBS) -o $@ $(LDFLAGS)
#	chmod $(TARMODE) $@

.PHONY: clean
clean:
#	@rm -f $(C_OBJS) $(CPP_OBJS) $(PROG_NAME) clist.txt cpplist.txt *.d *.d.* *.o
	-@if [ -d $(LIBCO_DIR) ]; then\
		make clean -C $(LIBCO_DIR);\
	fi
	-@find -name '*.o' | xargs -I [] rm [] >> /dev/null
	-@find -name '*.d' | xargs -I [] rm [] >> /dev/null
#	-@find -name '*.so' | xargs -I [] rm [] >> /dev/null
	-@rm -f $(TARGET_SO) $(TARGET_A)
	@echo "	<< $(shell basename `pwd`) cleaned >>"

.PHONY: distclean
distclean: clean
	-@if [ -d $(LIBCO_DIR) ]; then\
		rm -rf $(LIBCO_DIR);\
	fi

.PHONY: test
test:
	@echo 'Now build test programs'
	@echo LIBCOEVENT_LIB_PATH=$(LIBCOEVENT_LIB_PATH)