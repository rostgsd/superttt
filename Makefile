# Define the compiler
CC = gcc

# Define the output name
TARGET = sttt

# The "all" rule is what happens when you just type 'make'
all:
	$(CC) main.c menu.c sttt_in.c sttt_out.c -o $(TARGET)

# A "clean" rule to remove the executable
clean:
	rm -f $(TARGET)
