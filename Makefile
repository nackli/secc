# 可执行文件名
-include include/config/auto.conf
-include include/config/auto.conf.cmd
#include include/config/auto.conf

PROJECT_NAME = test
SHELL_CMD = shell
THIRDPARTY_PATH=$(PROJECT_PATH)/thirdparty
##################################### 项目路径 #####################################
PROJECT_PATH ?= ${shell pwd}
OBJ_DIR := $(PROJECT_PATH)/build

##################################### 设置编译器，默认使用GCC #####################################
CC ?= gcc
CC = arm-linux-gnueabihf-gcc
 
##################################### 所需头文件的路径 #####################################
CFLAGS = -I$(PROJECT_PATH)/base/ -I$(PROJECT_PATH)/
CFLAGS += -I$(THIRDPARTY_PATH)/zlog/inc -I$(THIRDPARTY_PATH)/plc/inc -I$(THIRDPARTY_PATH)/v2g/inc
LDFLAGS ?= -L$(THIRDPARTY_PATH)/zlog/lib -L$(THIRDPARTY_PATH)/plc/lib

##################################### 编译和链接参数 #####################################
CFLAGS ?= -O3 -g 
CXXFLAGS ?= -O3 -g -Wall -Wshadow -Wundef -Wcomment
LIBS ?= -lrt -lstdc++ -lzlog -lpthread -lslac

##################################### 收集需要编译的源文件 #####################################
CXX_SRCS = $(wildcard ./*.cpp ./base/*.cpp)
C_SRCS = $(wildcard $(THIRDPARTY_PATH)/v2g/src/*.c)
#CXX_SOURCES = $(foreach dir,$(CSRCS), $(wildcard $(dir)/*.cpp))

##################################### 包括其他MK文件 #####################################
#include $(PROJECT_PATH)/lib/lib.mk

##################################### 收集需要编译的源文件 #####################################
CXX_OBJCTS = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(CXX_SRCS)))
C_OBJCTS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(C_SRCS)))
#OBJCTS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(CSRCS)))

 
CREATEDIR = mkdir
#vpath %.cpp $(SOURSE_DIR)
$(OBJ_DIR)/%.o: $(THIRDPARTY_PATH)/v2g//src/%.c
	@$(CC) $(CCFLAGS) $(CFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: ./base/%.cpp
	@$(CC) $(CXXFLAGS) $(CXXFLAGS) $(CFLAGS) -c $< -o $@	
$(OBJ_DIR)/%.o: %.cpp
	@$(CC) $(CXXFLAGS) $(CXXFLAGS) $(CFLAGS) -c $< -o $@


all: CreateBuildDir BuildTarget
	
CreateBuildDir:
	@mkdir -p $(OBJ_DIR)

BuildTarget: $(C_OBJCTS) $(CXX_OBJCTS)
	@$(CC) -o $(OBJ_DIR)/$(PROJECT_NAME) $(C_OBJCTS) $(CXX_OBJCTS) $(LDFLAGS) $(LIBS)
#BuildTarget:$(OBJCTS)
#	@$(CC) -o $(OBJ_DIR)/$(PROJECT_NAME) $(CXX_OBJCTS) $(LDFLAGS) $(LIBS)

.PHONY: clean

clean:
	@rm -rf $(OBJ_DIR)

HMI_CONFIG = config
%.config:
	$(shell echo "aaa")


