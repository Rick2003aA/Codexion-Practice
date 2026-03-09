NAME := codexion

CC := cc
CFLAGS := -Wall -Wextra -Werror -pthread
CFLAGS += -I coders

SRC := coders/app/main.c \
	coders/core/sim_stop.c \
	coders/core/dongle.c \
	coders/core/heap.c \
	coders/core/scheduler.c \
	coders/core/coder_actions.c \
	coders/core/coder_routine.c \
	coders/core/monitor.c \
	coders/core/monitor_checks.c \
	coders/init/parse.c \
	coders/init/sim_init.c \
	coders/common/time.c \
	coders/common/log.c \
	coders/common/sleep.c \
	coders/common/cleanup.c

OBJ := $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

run: all
	./$(NAME) 5 1400 200 100 100 3 0 fifo

.PHONY: all clean fclean re
