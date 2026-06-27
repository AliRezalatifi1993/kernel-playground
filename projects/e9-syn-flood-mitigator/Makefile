TARGET = xdp_syn_flood
BPF_OBJ = $(TARGET).o
BPF_SRC = $(TARGET).bpf.c

UNAME_M := $(shell uname -m)

ifeq ($(UNAME_M),x86_64)
    ARCH_INCLUDE = /usr/include/x86_64-linux-gnu
else ifeq ($(UNAME_M),aarch64)
    ARCH_INCLUDE = /usr/include/aarch64-linux-gnu
else
    ARCH_INCLUDE = /usr/include/$(UNAME_M)-linux-gnu
endif

CLANG ?= clang

all: $(BPF_OBJ)

$(BPF_OBJ): $(BPF_SRC)
	$(CLANG) -O2 -g -Wall -target bpf -I$(ARCH_INCLUDE) -c $(BPF_SRC) -o $(BPF_OBJ)

clean:
	rm -f $(BPF_OBJ)
