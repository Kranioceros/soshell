# tool macros
CC := gcc # FILL: the compiler
CCFLAG := # FILL: compile flags
LINKFLAG := -lfl -lreadline
DBGFLAG := -g
CCOBJFLAG := $(CCFLAG) -c

# path macros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug

# compile macros
TARGET_NAME := sosh # FILL: target name
TARGET := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DEBUG := $(DBG_PATH)/$(TARGET_NAME)
MAIN_SRC := main.c # FILL: src file contains `main()`

# src files & obj files
SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
OBJ := $(OBJ) $(OBJ_PATH)/lex.yy.o $(OBJ_PATH)/sosh.tab.o
OBJ_DEBUG := $(addprefix $(DBG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
OBJ_DEBUG := $(OBJ_DEBUG) $(OBJ_PATH)/lex.yy.o $(OBJ_PATH)/sosh.tab.o

# clean files list
DISTCLEAN_LIST := $(OBJ) \
                  $(OBJ_DEBUG)
CLEAN_LIST := $(TARGET) \
			  $(TARGET_DEBUG) \
			  $(DISTCLEAN_LIST)

# default rule
default: all

# rules for creating the parser
$(OBJ_PATH)/lex.yy.o: parser/lex.yy.c
	gcc -c -o $(OBJ_PATH)/lex.yy.o parser/lex.yy.c 

$(OBJ_PATH)/sosh.tab.o: parser/sosh.tab.c
	gcc -c -o $(OBJ_PATH)/sosh.tab.o parser/sosh.tab.c

parser/lex.yy.c: parser/sosh.l parser/sosh.tab.h
	flex -o parser/lex.yy.c parser/sosh.l

parser/sosh.tab.c: parser/sosh.y src/AST.h
	bison -o parser/sosh.tab.c --defines=parser/sosh.tab.h parser/sosh.y

parser/sosh.tab.h: parser/sosh.y src/AST.h
	bison -o parser/sosh.tab.c --defines=parser/sosh.tab.h parser/sosh.y

# non-phony targets
$(TARGET): $(OBJ)
	$(CC) $(CCFLAG) $(LINKFLAG) -o $@ $?

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CCOBJFLAG) -o $@ $<

$(DBG_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CCOBJFLAG) $(DBGFLAG) -o $@ $<

$(TARGET_DEBUG): $(OBJ_DEBUG)
	$(CC) $(CCFLAG) $(DBGFLAG) $(LINKFLAG) $? -o $@

# phony rules
.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: $(TARGET_DEBUG)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)
	rm -f parser/*.o
	rm -f parser/*.c
	rm -f parser/*.h

.PHONY: distclean
distclean:
	@echo CLEAN $(DISTCLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)