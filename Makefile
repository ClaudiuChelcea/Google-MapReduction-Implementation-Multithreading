# Chelcea Claudiu-Marian
# claudiu.chelcea@stud.acs.upb.ro
# Makefile

# ********************************************************
# ******************** COMPILER SETUP ******************** 
CC = g++
CFLAGS = #-Wall -Wextra -g -std=c++17
SRC = main.cpp io_manager.cpp
HEADERS = io_manager.h
OBJECTS = main.o io_manager.o
NAME = tema1.exe

# ********************************************************
# ******************** EXECUTE BUILD *********************
build: $(NAME)

$(NAME): $(OBJECTS) 
	$(CC) $(CFLAGS) $^ -o $(NAME)

main.o: main.cpp
	$(CC) -c $^

io_manager.o: io_manager.cpp io_manager.h
	$(CC) -c $^

run:
	./$(NAME)
	
.PHONY: clean

clean: 
	rm $(NAME)
	rm $(OBJECTS)
	


# ******************** END ******************************
# *******************************************************
