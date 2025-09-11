RISC-V Functional Simulator
This project is a functional simulator for a subset of the RISC-V instruction set architecture. It models both a simple non-pipelined, single-cycle processor and a more complex 5-stage pipelined processor with hazard detection and resolution.

✨ Key Features
Dual Execution Modes: Supports both non-pipelined (single-cycle) and 5-stage pipelined execution.

Hazard Handling: Implements data forwarding and stalling mechanisms to resolve data hazards in the pipelined model.

Branch Prediction: Features a Branch Target Buffer (BTB) with a 1-bit predictor to minimize control hazards.

Detailed Tracing: Offers extensive debugging capabilities, including cycle-by-cycle register tracing, pipeline register inspection, and instruction-specific tracking.

Runtime Configuration: Allows users to interactively enable or disable features like pipelining, forwarding, and tracing at the start of the simulation.

🏛️ Architectural Design
The simulator models the classic 5-stage RISC pipeline and includes several key architectural components.

Pipeline Structure
The pipelined processor follows the five classic RISC stages with inter-stage buffers (pipeline registers) between them.

graph LR
    FETCH --> DECODE --> EXECUTE --> MEM --> WRITEBACK

Core Components
5 Pipeline Stages: Fetch, Decode, Execute, Memory Access, and Write Back.

Forwarding Unit: Resolves data hazards by forwarding results from later stages to earlier stages, minimizing stalls.

Branch Target Buffer (BTB): A cache that stores the target addresses of taken branches to speed up instruction fetching, coupled with a 1-bit branch predictor.

Memory Subsystem: Supports byte, half-word, and word memory accesses (lb, lh, lw, sb, sh, sw).

🚀 Getting Started
Follow these instructions to compile and run the simulator.

Compilation
You can compile the simulator in either non-pipelined or pipelined mode.

Non-Pipelined Version
g++ main.cpp alu_unit.cpp control_unit.cpp dec2bin.cpp global_variables.cpp immediate.cpp memory_read_write_funcs.cpp myRISCVSim.cpp registerfile.cpp -o output.exe

Pipelined Version (includes forwarding and branch prediction)
g++ main.cpp alu_unit.cpp BTB_operations.cpp control_unit.cpp forwarding_unit.cpp dec2bin.cpp global_variables.cpp immediate.cpp memory_read_write_funcs.cpp myRISCVSim.cpp registerfile.cpp -o output.exe

Execution
Run the simulator by providing a machine code file as a command-line argument.

./output.exe <program_file.mc>

Example:

./output.exe ../test/bubble_sort.mc

⚙️ Runtime Configuration
Before the simulation begins, you will be prompted to configure the following settings interactively:

Pipeline Mode:

0: Non-pipelined execution

1: Pipelined execution

Hazard Resolution (if pipelining is enabled):

0: Use stalling only

1: Enable data forwarding

Register Tracing:

0: Disable

1: Print the state of all 32 registers after each clock cycle

Pipeline Register Tracing:

0: Disable

1: Enable pipeline register inspection

Instruction-specific Tracing:

Enter an instruction number to see its detailed path through the pipeline stages.

Branch Prediction Unit (BPU) Debugging:

0: Disable

1: Print the BPU state (PC, PHT, BTP) each cycle

💻 Simulation Flow
Initialization: The instruction and data memories are initialized from the input machine code (.mc) file.

Configuration: The user sets the desired simulation parameters through interactive prompts.

Execution: The simulator fetches and executes instructions sequentially based on the configured mode.

Termination: The simulation runs until it fetches the instruction 0xFFFFFFFF, which serves as the exit condition.

Statistics: Upon completion, it generates and displays key performance statistics, including total cycles, CPI, hazards encountered, and branch prediction accuracy.

🧑‍💻 Development Team
Aditya Yadav (2022MEB1291)

Dhruv Agarwal (2022MEB1306)

Satyam Kumar (2022MCB1279)
