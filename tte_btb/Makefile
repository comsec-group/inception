CC = clang
SRCS = $(wildcard *.c)
INCPATH = -I./
BIN = ijmp_ijmp jcc_ijmp ret_ijmp ooo_ijmp btc-djmp_ijmp btc-ijmp_ijmp
OBJS = $(filter-out $(BIN:=.o), $(SRCS:.c=.o))
OBJS = memtools.o
CFLAGS = -O2 -Wall -Wno-language-extension-token -Wno-unused-function -pedantic -DARCH=$(shell ./arch.sh)
DEP_FILE = .depend

.PHONY = all clean

all: depend $(BIN)

$(BIN: % = %.o): %: %.o

$(BIN): $(OBJS)
	@echo LD $@
	@$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

.c.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) $(INCPATH) $<

clean:
	@echo CLEAN
	@$(RM) $(OBJS)
	@$(RM) $(DEP_FILE)
	@$(RM) $(BIN)
	@$(RM) $(foreach f,$(BIN),$f.o)

depend: $(DEP_FILE)
	@touch $(DEP_FILE)

$(DEP_FILE):
	@echo DEP $@
	@-$(CC) -E -MM $(CFLAGS) $(INCPATH) $(SRCS) >> $(DEP_FILE)

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(DEP_FILE)
endif
