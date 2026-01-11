CC = g++
OPT = -O3
#OPT = -g
# Set the standard to C++11 as it is usually required for this project
STANDARD = -std=c++11
WARN = -Wall
CFLAGS = $(OPT) $(STANDARD) $(WARN)

# Directory for source files
SRC_DIR = src

# Source and Object files
SIM_SRC = $(SRC_DIR)/sim_proc.cc
SIM_OBJ = sim_proc.o

#################################

# default rule
all: sim
	@echo "my work is done here..."

# rule for making sim
sim: $(SIM_OBJ)
	$(CC) -o sim $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH sim-----------"

# Generic rule for converting .cc in src/ to .o in the root directory
# Also tracks changes in sim_proc.h
sim_proc.o: $(SRC_DIR)/sim_proc.cc $(SRC_DIR)/sim_proc.h
	$(CC) $(CFLAGS) -c $< -o $@

# Test command: Creates a 3-instruction trace and runs with ROB=16, IQ=8, Width=4
test: sim
	@printf "ab120024 0 1 2 3\nab120028 1 4 1 3\nab12002c 2 -1 4 7\n" > test_trace.txt
	./sim 16 8 4 test_trace.txt
	@rm test_trace.txt
	@echo "Test run successful."

# type "make clean" to remove all .o files plus the sim binary
clean:
	rm -f *.o sim test_trace.txt

# type "make clobber" to remove all .o files (leaves sim binary)
clobber:
	rm -f *.o

.PHONY: all clean clobber test