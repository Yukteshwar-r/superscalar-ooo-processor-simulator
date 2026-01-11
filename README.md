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

`make`

### Testing the Build
Verify the simulator logic and timing output by running:

`make test`

## ⚙️ Usage
The simulator requires architectural parameters and a trace file as command-line arguments:

`./sim <ROB_SIZE> <IQ_SIZE> <WIDTH> <tracefile>`

* **ROB_SIZE**: Number of entries in the Reorder Buffer.
* **IQ_SIZE**: Number of entries in the Issue Queue.
* **WIDTH**: Superscalar width (maximum instructions per stage per cycle).

## 📂 Trace File Format
The simulator reads trace files where each line represents a dynamic instruction in the following format:

`<PC> <operation_type> <dest_reg> <src1_reg> <src2_reg>`

* **PC**: Program Counter (hexadecimal).
* **operation_type**: 0 (1-cycle latency), 1 (2-cycle latency), or 2 (5-cycle latency).
* **dest_reg**: Destination register index (-1 if none).
* **src1_reg**: First source register index (-1 if none).
* **src2_reg**: Second source register index (-1 if none).

Example line: ab120024 0 1 2 3 (Op Type 0, Dest: R1, Sources: R2, R3)



## 📊 Output Analysis
The simulator provides a cycle-accurate trace of every instruction followed by aggregate results:

* **Instruction Timing**: For each instruction, the output shows the start cycle and duration for every stage:
    * FE (Fetch), DE (Decode), RN (Rename), RR (Reg-Read), DI (Dispatch), IS (Issue), EX (Execute), WB (Writeback), RT (Retire).
* **Processor Configuration**: Echoes the ROB size, IQ size, and pipeline width.
* **Simulation Results**:
    * **Dynamic Instruction Count**: Total number of retired instructions.
    * **Cycles**: Total clock cycles required to complete the trace.
    * **Instructions Per Cycle (IPC)**: Throughput measurement (Instructions / Cycles).
