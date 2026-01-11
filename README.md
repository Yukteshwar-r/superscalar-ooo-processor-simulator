# superscalar-ooo-processor-simulator

A high-performance C++ architectural simulator that models a dynamic instruction scheduling mechanism for a superscalar processor. This tool simulates a multi-stage out-of-order pipeline to measure the cycle-accurate execution of instruction traces, focusing on register renaming, data dependencies, and structural hazards.

## 🛠 Features
* **N-Wide Superscalar Pipeline**: Fully configurable width for fetch, issue, and retire stages.
* **Dynamic Scheduling**: Implements an Issue Queue (IQ) for out-of-order dispatch and a Reorder Buffer (ROB) for in-order retirement.
* **Register Renaming**: Models a Rename Map Table (RMT) to handle RAW (Read-After-Write) dependencies across 67 architectural registers.
* **Pipelined Function Units**: Universal FUs with variable latencies based on operation type (Type 0: 1 cycle, Type 1: 2 cycles, Type 2: 5 cycles).
* **Comprehensive Timing Logs**: Detailed tracking of instruction movement through all 9 pipeline stages.

## 🚀 Getting Started

### Compilation
Build the simulator using the optimized Makefile:

make

### Testing the Build
Verify the simulator logic and timing output by running:

make test

## ⚙️ Usage
The simulator requires architectural parameters and a trace file as command-line arguments:

./sim <ROB_SIZE> <IQ_SIZE> <WIDTH> <tracefile>

* **ROB_SIZE**: Number of entries in the Reorder Buffer.
* **IQ_SIZE**: Number of entries in the Issue Queue.
* **WIDTH**: Superscalar width (maximum instructions per stage per cycle).

## 📊 Output Analysis
The simulator provides a cycle-accurate trace of every instruction followed by aggregate results:

* **Instruction Timing**: For each instruction, the output shows the start cycle and duration for every stage:
    * FE (Fetch), DE (Decode), RN (Rename), RR (Reg-Read), DI (Dispatch), IS (Issue), EX (Execute), WB (Writeback), RT (Retire).
* **Processor Configuration**: Echoes the ROB size, IQ size, and pipeline width.
* **Simulation Results**:
    * **Dynamic Instruction Count**: Total number of instructions retired.
    * **Cycles**: Total clock cycles required to complete the trace.
    * **Instructions Per Cycle (IPC)**: A measure of the processor's throughput.