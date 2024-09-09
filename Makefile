SRC_PATH = ./src
SRCS = $(wildcard $(SRC_PATH)/*.c)

BIN_PATH = ./bin
BINS = $(subst $(SRC_PATH), $(BIN_PATH), $(SRCS:%.c=%))
$(shell mkdir -p $(BIN_PATH))

AUX = $(SRC_PATH)/common.h

CC = riscv64-linux-gnu-gcc-13
override CFLAGS += -static -march=rv64gcv
LIBS += -lm

.PHONY: all clean install
all: $(notdir $(BINS))

%: $(SRC_PATH)/%.c $(AUX)
	$(CC) $(LDFLAGS) -o $(BIN_PATH)/$@ $(CFLAGS) $< $(LIBS)

clean:
	rm -f $(BINS)
