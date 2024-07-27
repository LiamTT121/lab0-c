CC = gcc
CFLAGS := -Wall -Wextra -std=c11
CFLAGS += -I. -MMD
TRAIN = train

GIT_HOOKS := .git/hooks/applied

deps += $(TRAIN).d

$(TRAIN): $(TRAIN).c agents/reinforcement_learning.c game.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	-$(RM) $(deps) $(TRAIN) *.o
