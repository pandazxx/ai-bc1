APP_NAME = dino_runner

CC = cc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lraylib -lm

all: $(APP_NAME)

$(APP_NAME): main.c
	$(CC) $(CFLAGS) -o $(APP_NAME) main.c $(LDFLAGS)

test: tests/test_placeholder.c
	$(CC) $(CFLAGS) -o test_placeholder tests/test_placeholder.c
	./test_placeholder

run: $(APP_NAME)
	./$(APP_NAME)

clean:
	rm -f $(APP_NAME) test_placeholder
