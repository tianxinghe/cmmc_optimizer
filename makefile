ifeq ($(TASK),)
$(error Please specify TASK=task1, task2, or task3)
endif

# ========== 目录定义 ==========
SRC_DIR        := ./src/$(TASK)
BUILD_DIR      := ./build/$(TASK)
GENERATE_DIR   := $(BUILD_DIR)/generate
OBJ_DIR        := $(BUILD_DIR)/obj

ALL_SRCS := $(shell find $(SRC_DIR) -name "*.c" -or -name "*.cpp")

IR_PARSE_DIR   := $(SRC_DIR)/IR_parse
IR_LEXICAL_L   := $(shell find $(IR_PARSE_DIR) -name "*.l")
IR_SYNTAX_Y    := $(shell find $(IR_PARSE_DIR) -name "*.y")
IR_LEX_YY_C    := $(GENERATE_DIR)/IR_lex.yy.c
IR_SYNTAX_TAB_C := $(GENERATE_DIR)/IR_syntax.tab.c
IR_SYNTAX_TAB_H := $(GENERATE_DIR)/IR_syntax.tab.h

ALL_SRCS += $(IR_LEX_YY_C) $(IR_SYNTAX_TAB_C)

OBJS := $(addprefix $(OBJ_DIR)/, $(addsuffix .o, \
        $(basename $(patsubst $(SRC_DIR)/%, %, $(filter $(SRC_DIR)/%, $(ALL_SRCS))))))

GEN_OBJS := $(addprefix $(OBJ_DIR)/, $(addsuffix .o, \
            $(basename $(notdir $(IR_LEX_YY_C) $(IR_SYNTAX_TAB_C)))))
OBJS += $(GEN_OBJS)

PARSER := ./parser_$(TASK)

INC_PATH := . \
            ./include \
            $(SRC_DIR)/include \
            $(SRC_DIR)/IR_optimize/include \
            $(IR_PARSE_DIR)/include \
            $(GENERATE_DIR)
INCLUDES := $(addprefix -I, $(INC_PATH))

LEX      := flex
YACC     := bison
CC       := gcc
CXX      := g++
LD       := gcc

YACC_FLAGS    += -d -v
COMMON_CFLAGS += -MMD -c -Wall $(INCLUDES) -O2
CFLAGS        += $(COMMON_CFLAGS)
CXX_FLAGS     += $(COMMON_CFLAGS) -std=c++17
LDFLAGS       += -lfl

# 普通 .c 文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "+ CC" $(notdir $<)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -o $@ $<

# 普通 .cpp 文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "+ CXX" $(notdir $<)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXX_FLAGS) -o $@ $<

# 生成 bison 的 .c 和 .h（同时生成）
$(IR_SYNTAX_TAB_C) $(IR_SYNTAX_TAB_H): $(IR_SYNTAX_Y)
	@echo "+ YACC" $(notdir $<)
	@mkdir -p $(dir $@)
	@$(YACC) $(YACC_FLAGS) -o $(IR_SYNTAX_TAB_C) $<

# 生成 flex 的 .c（依赖 bison 头文件）
$(IR_LEX_YY_C): $(IR_LEXICAL_L) $(IR_SYNTAX_TAB_H)
	@echo "+ LEX" $(notdir $<)
	@mkdir -p $(dir $@)
	@$(LEX) -o $@ $<

# 编译 flex 生成的 .c
$(OBJ_DIR)/IR_lex.yy.o: $(IR_LEX_YY_C) $(IR_SYNTAX_TAB_H)
	@echo "+ CC" $(notdir $<)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -o $@ $<

# 编译 bison 生成的 .c
$(OBJ_DIR)/IR_syntax.tab.o: $(IR_SYNTAX_TAB_C) $(IR_SYNTAX_TAB_H)
	@echo "+ CC" $(notdir $<)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -o $@ $<

# 链接
$(PARSER): $(OBJS)
	@echo "+ LD" $(notdir $@)
	@$(LD) -o $@ $^ $(LDFLAGS)

-include $(OBJS:.o=.d)

.PHONY: all clean run gdb

all: $(PARSER)

clean:
	rm -rf $(BUILD_DIR) $(PARSER)

run: $(PARSER)
	$(PARSER) $(ARG)

gdb: $(PARSER)
	gdb $(PARSER)

.DEFAULT_GOAL := all