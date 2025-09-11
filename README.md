
This project implements a functional simulator for a subset of RISC-V architecture, developed in three phases. The current implementation covers:

Phase II: Non-Pipelined Execution
Basic single-cycle processor implementation

Five classic RISC stages: Fetch, Decode, Execute, Memory Access, Write Back

Phase III: Pipelined Execution
5-stage pipeline implementation with hazard handling

Data forwarding and stalling mechanisms

Branch prediction with 1-bit predictor

Pipeline register tracing capabilities

Installation & Usage
Compilation
   # Non-pipelined version
g++ main.cpp alu_unit.cpp control_unit.cpp dec2bin.cpp global_variables.cpp immediate.cpp memory_read_write_funcs.cpp myRISCVSim.cpp registerfile.cpp -o output.exe

# Pipelined version (includes forwarding and branch prediction)
g++ main.cpp alu_unit.cpp BTB_operations.cpp control_unit.cpp forwarding_unit.cpp dec2bin.cpp global_variables.cpp immediate.cpp memory_read_write_funcs.cpp myRISCVSim.cpp registerfile.cpp -o output.exe

Execution: 
./output.exe <program_file.mc>

Example: ./output ../test/bubble_sort.mc

Key Features
Configuration Knobs
Pipeline Mode
0 - Non-pipelined | 1 - Pipelined

Hazard Resolution
0 - Stalling | 1 - Data Forwarding

Register Tracing
0 - Disable | 1 - Print registers each cycle

Pipeline Register Tracing
0 - Disable | 1 - Enable pipeline register inspection

Instruction-specific Tracing
Enter instruction number for detailed pipeline tracking

Branch Prediction Unit (BPU) Debugging
0 - Disable | 1 - Print BPU state (PC, PHT, BTP) each cycle

Architectural Components
5 Pipeline Stages with inter-stage buffers

Forwarding Unit for data hazard resolution

Branch Target Buffer (BTB) with 1-bit predictor

Memory Subsystem supporting byte/halfword/word accesses

Implementation Details
Pipeline Structure
graph LR
    FETCH --> DECODE --> EXECUTE --> MEM --> WRITEBACK

Exit Condition
Simulation terminates on encountering instruction 0xFFFFFFFF

Development Team
Aditya Yadav (2022MEB1291)

Dhruv Agarwal (2022MEB1306)

Satyam Kumar (2022MCB1279)

Simulation Flow
Memory initialization from input file

Sequential instruction execution

Runtime configuration through interactive prompts

Statistics generation (cycles, CPI, hazards, etc.)
