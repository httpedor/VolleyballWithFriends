ifeq ($(OS),Windows_NT)
    RM = del /Q /F
	AND = &
	COPY = xcopy /s /y
else
    RM = rm -rf
	AND = ;
	COPY = cp -a
endif

SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := build
ASSETS_DIR := assets
LIB_DIR := lib
OUT_DIR := bin
CC = gcc
CFLAGS = -Wall -Iinclude -L$(LIB_DIR) -lSDL2 -lSDL2_ttf

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: main

debug: CFLAGS += -g
debug: main

main: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/main.exe $(AND) $(COPY) $(ASSETS_DIR) $(OUT_DIR) $(AND) $(COPY) $(LIB_DIR) $(OUT_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ASSETS_DIR)/%:
	$(COPY) $@ $(OUT_DIR)
$(LIB_DIR)/%:
	$(COPY) $@ $(OUT_DIR)

clean:
	$(RM) $(BUILD_DIR)\*