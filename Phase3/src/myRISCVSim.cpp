/* 

The project is developed as part of Computer Architecture class
Project Name: Functional Simulator for subset of RISCV Processor

Developer's Name:Aditya Yadav, Dhruv Agarwal
Developer's Email id:2022MEB1291@iitrpr.ac.in, 2022MEB1306@iitrpr.ac.in


*/


/* myRISCVSim.cpp
   Purpose of this file: implementation file for myRISCVSim
*/
#include "builtin_funcs.hpp"
#include"self_defined_funcs.hpp"
#include "myRISCVSim.hpp"
#ifndef MYCLASSES
#define MYCLASSES
#include "registerfile.hpp"
#include "control_unit.hpp"
#include "forwarding_unit.hpp"
#endif
#include "global_variables.hpp"
using namespace std;


void run_riscvsim(bool knob2) {
    display();//
    n_cycles=1;//
    EXIT=false;
    int i;
    string run_mode="STEP";
    string input;
    cout<<"type STEP or RUN"<<endl;
    cin>>input;
    if(input=="RUN"){
        run_mode="RUN";
    }
    while(!EXIT){
        printf("*** CYCLE : %d  ***",n_cycles);//
        fetch();
        decode(knob2);
        execute(knob2);
        mA(knob2);
        write_back();
        positive_edge_trigger();
        if(run_mode=="STEP"){
            display();
            cout<<"type STEP or RUN"<<endl;
            cin>>input;
            if(input=="RUN"){
                run_mode="RUN";
            }
        } 
        n_cycles++;//  
    }
    n_cycles--;
    display();
    
}

// it is used to set the reset values
//reset all registers and memory content to 0
void reset_proc()
{
    // set PC to zero
    PC = 0;
    PCWrite=true;
    nextPC = 4;
    branchPC = 0;
    EXIT = false;
    mem.clear();
    for(int i=0;i<BTB_SIZE;i++){
        BTB[i].address=-0xF;
    }
    for (int i = 0; i < 32; i++)
    {
        registerFile.set_register(i, 0);
        if (i == 2)
        {
            // x[i] = 0x7FFFFFF0; // sp
            registerFile.set_register(i, strtol("0x7FFFFFF0", NULL, 16)); // sp
        }
        else if (i == 3)
        {
            // x[i] = 0x10000000; // gp
            registerFile.set_register(i, strtol("0x10000000", NULL, 16)); // gp
        }
    }
    Control_unit nop_control;
    nop_control.set_instruction("00000000000000000000000000000000");
    nop_control.build_control();

    if_de_rest.instruction = "00000000000000000000000000000000";
    if_de_rest.writemode=true;
    if_de_rest.PC=-1;
    temp_if_de_rest=if_de_rest;

    de_ex_rest.instruction=if_de_rest.instruction;
    de_ex_rest.branch_target = 0;
    de_ex_rest.A = 0;
    de_ex_rest.B = 0;
    de_ex_rest.op2 = 0;
    de_ex_rest.rd = 0;
    de_ex_rest.rs1=0;
    de_ex_rest.rs2=0;
    de_ex_rest.control=nop_control;
    de_ex_rest.PC=-1;
    de_ex_rest.writemode=true;
    temp_de_ex_rest=de_ex_rest;

    ex_ma_rest.instruction=if_de_rest.instruction;
    ex_ma_rest.alu_result=0;
    ex_ma_rest.op2=0;
    ex_ma_rest.rd=0;
    ex_ma_rest.rs1=0;
    ex_ma_rest.rs2=0;
    ex_ma_rest.control=nop_control;
    ex_ma_rest.PC=-1;
    ex_ma_rest.writemode=true;
    temp_ex_ma_rest=ex_ma_rest;

    ma_wb_rest.instruction=if_de_rest.instruction;
    ma_wb_rest.alu_result = 0;
    ma_wb_rest.ld_result = 0;
    ma_wb_rest.rd = 0;
    ma_wb_rest.rs1=0;
    ma_wb_rest.rs2=0;
    ma_wb_rest.control=nop_control;
    ma_wb_rest.PC=-1;
    ma_wb_rest.writemode=true;
}

//load_program_memory reads the input memory, and pupulates the instruction 
// memory
void load_program_memory(char *file_name) {
    FILE *fp;
  unsigned int address, instruction;
  fp = fopen(file_name, "r");
  if(fp == NULL) {
    printf("Error opening input mem file\n");
    exit(1);
  }
  while(fscanf(fp, "%x %x", &address, &instruction) != EOF) {
    memory_write((unsigned int) address,(unsigned long long int)instruction,4);
    // printf("%x %u\n",address,mem[(unsigned int) address]);//  
  }
  fclose(fp);

}

// //reads from the instruction memory and updates the instruction register
void fetch()
{   
    // nextPC = PC + 4;
    cout<<"\nFETCH STAGE"<<endl;
    printf("Current PC=%x ",PC);
    unsigned int instruct_dec = (unsigned int)memory_read((unsigned int)PC, 4);
    string instruction = dec2bin(instruct_dec);
    temp_if_de_rest.instruction = instruction;  //it is acting like a buffer register
    temp_if_de_rest.PC=PC;
    cout<<"Instruction  "<<temp_if_de_rest.instruction<<endl;
}
// //reads the instruction register, reads operand1, operand2 fromo register file, decides the operation to be performed in execute stage
void decode(bool knob2){
    cout<<"\nDECODE STAGE"<<endl;
        //setting the controls
    if_de_rest.new_control.set_instruction(if_de_rest.instruction);
    if_de_rest.new_control.build_control();
    if(if_de_rest.new_control.isexit){
        PCWrite=false;
        if_de_rest.writemode=false;
    }
    
    // getting destination register
    string rds=if_de_rest.instruction.substr(20,5);
    int rd=(int)unsgn_binaryToDecimal(rds);
    // getting source register 1
    string rs1s=if_de_rest.instruction.substr(12,5);
    int rs1=unsgn_binaryToDecimal(rs1s);
    //getting source register 2
    string rs2s=if_de_rest.instruction.substr(7,5);
    int rs2=unsgn_binaryToDecimal(rs2s);
    int imm=immediate(if_de_rest.instruction);
    //setting the forwarding unit 
    forwarding_unit.de_inst.opcode=if_de_rest.new_control.inst_type;
    forwarding_unit.de_inst.rd=rd;
    forwarding_unit.de_inst.rs1=rs1;
    forwarding_unit.de_inst.rs2=rs2;

    forwarding_unit.ex_inst.opcode=de_ex_rest.control.inst_type;
    forwarding_unit.ex_inst.rd=de_ex_rest.rd;
    forwarding_unit.ex_inst.rs1=de_ex_rest.rs1;
    forwarding_unit.ex_inst.rs2=de_ex_rest.rs2; 

    forwarding_unit.ma_inst.opcode=ex_ma_rest.control.inst_type;
    forwarding_unit.ma_inst.rd=ex_ma_rest.rd;
    forwarding_unit.ma_inst.rs1=ex_ma_rest.rs1;
    forwarding_unit.ma_inst.rs2=ex_ma_rest.rs2; 

    forwarding_unit.wb_inst.opcode=ma_wb_rest.control.inst_type;
    forwarding_unit.wb_inst.rd=ma_wb_rest.rd;
    forwarding_unit.wb_inst.rs1=ma_wb_rest.rs1;
    forwarding_unit.wb_inst.rs2=ma_wb_rest.rs2;

    if(knob2){ 
        int op1=registerFile.get_register(rs1),op2=registerFile.get_register(rs2);
        forwarding_unit.build_mux_selectors();
        printf("select_de_op1: %d\n",forwarding_unit.select_de_op1);
        printf("select_de_op2: %d\n",forwarding_unit.select_de_op2);
        printf("select_ex_A: %d\n",forwarding_unit.select_ex_A);
        printf("select_ex_B: %d\n",forwarding_unit.select_ex_B);
        printf("select_ex_op2: %d\n",forwarding_unit.select_ex_op2);
        printf("select_ma_op2: %d\n",forwarding_unit.select_ma_op2);
        // printf("selector")
        temp_de_ex_rest.instruction=if_de_rest.instruction;
        temp_de_ex_rest.rd=rd;
        
        //forwarding
        unsigned int wb_result = 0;
        if (ma_wb_rest.control.isWb)
        {
            if (ma_wb_rest.control.wbSignal == "alu")
            {
                wb_result = ma_wb_rest.alu_result;
            }
            else if (ma_wb_rest.control.wbSignal == "ld")
            {
                wb_result = ma_wb_rest.ld_result;
            }
            else if (ma_wb_rest.control.wbSignal == "pc+4")
            {
                wb_result = ma_wb_rest.PC + 4;
            }
            else
            {
                cout << "error :undefined wbSignal" << endl;
            }
        }
        //mux1
        if(forwarding_unit.select_de_op1==0){
            temp_de_ex_rest.A=op1;
        }
        else if(forwarding_unit.select_de_op1==1){
            temp_de_ex_rest.A=wb_result;
        }
        else{
            cout<<"unidentified value of select_de_op1"<<endl;
        }

        //mux2
        if(forwarding_unit.select_de_op2==0){
            temp_de_ex_rest.op2=op2;
        }
        else if(forwarding_unit.select_de_op2==1){
            temp_de_ex_rest.op2=wb_result;
        }
        else{
            cout<<"unidentified value of select_de_op2"<<endl;
        }


        
        
        temp_de_ex_rest.branch_target=imm;
        temp_de_ex_rest.PC=if_de_rest.PC;
        temp_de_ex_rest.control=if_de_rest.new_control;
        if(if_de_rest.new_control.isImmediate){
            temp_de_ex_rest.B=imm;
        }
        else{
            temp_de_ex_rest.B=temp_de_ex_rest.op2;
        }
        temp_de_ex_rest.rs1=rs1;
        temp_de_ex_rest.rs2=rs2;    
    }
    else{
        if(forwarding_unit.ifDependencyrs1(forwarding_unit.de_inst,forwarding_unit.ex_inst)
        ||forwarding_unit.ifDependencyrs2(forwarding_unit.de_inst,forwarding_unit.ex_inst)
        ||forwarding_unit.ifDependencyrs1(forwarding_unit.de_inst,forwarding_unit.ma_inst)
        ||forwarding_unit.ifDependencyrs2(forwarding_unit.de_inst,forwarding_unit.ma_inst)
        ||forwarding_unit.ifDependencyrs1(forwarding_unit.de_inst,forwarding_unit.wb_inst)
        ||forwarding_unit.ifDependencyrs2(forwarding_unit.de_inst,forwarding_unit.wb_inst)
        ){
            n_stalls_data++;
            PCWrite=false;
            if_de_rest.writemode=false;
            
            temp_de_ex_rest.instruction="00000000000000000000000000000000";
            temp_de_ex_rest.rd=0;
            temp_de_ex_rest.A=0;
            temp_de_ex_rest.B=0;
            temp_de_ex_rest.op2=0;
            temp_de_ex_rest.branch_target=0;
            temp_de_ex_rest.PC=0;
            temp_de_ex_rest.control.set_instruction(temp_de_ex_rest.instruction);
            temp_de_ex_rest.control.build_control();
            temp_de_ex_rest.rs1=0;
            temp_de_ex_rest.rs2=0;
            return;
            cout<<"nop :stall due to data dependency(no forwarding)";
        }
        else{
            temp_de_ex_rest.instruction=if_de_rest.instruction;
            temp_de_ex_rest.rd=rd;
            temp_de_ex_rest.A=registerFile.get_register(rs1);
            temp_de_ex_rest.op2=registerFile.get_register(rs2);
            temp_de_ex_rest.branch_target=imm;
            temp_de_ex_rest.PC=if_de_rest.PC;
            temp_de_ex_rest.control=if_de_rest.new_control;
            temp_de_ex_rest.rs1=rs1;
            temp_de_ex_rest.rs2=rs2;
            if(if_de_rest.new_control.isImmediate){
                temp_de_ex_rest.B=imm;
            }
            else{
                temp_de_ex_rest.B=registerFile.get_register(rs2);
            }
        }
    }
    
    printf("branch target :%d\n",temp_de_ex_rest.branch_target);//
    printf("A :%d\n",temp_de_ex_rest.A);//
    printf("B :%d\n",temp_de_ex_rest.B);//
    printf("op2 :%d\n",temp_de_ex_rest.op2);//
    printf("rd :%d\n",temp_de_ex_rest.rd); //
}


// //executes the ALU operation based on ALUop
void execute(bool knob2){
    //forwarding
    if(de_ex_rest.control.isexit){
        de_ex_rest.writemode=false;
    }
    if(knob2){
        unsigned int wb_result = 0;
        if (ma_wb_rest.control.isWb)
        {
            if (ma_wb_rest.control.wbSignal == "alu")
            {
                wb_result = ma_wb_rest.alu_result;
            }
            else if (ma_wb_rest.control.wbSignal == "ld")
            {
                wb_result = ma_wb_rest.ld_result;
            }
            else if (ma_wb_rest.control.wbSignal == "pc+4")
            {
                wb_result = ma_wb_rest.PC + 4;
            }
            else
            {
                cout << "error :undefined wbSignal" << endl;
            }
        }
        //mux3
        if(forwarding_unit.select_ex_A==0){
            ;
        }
        else if(forwarding_unit.select_ex_A==1){
            de_ex_rest.A=ex_ma_rest.alu_result;
        }
        else if(forwarding_unit.select_ex_A==2){
            de_ex_rest.A=wb_result;
        }
        else{
            cout<<"unidentified select_ex_A"<<endl;
        }
        //mux4
        if(forwarding_unit.select_ex_B==0){
            ;
        }
        else if(forwarding_unit.select_ex_B==1){
            // printf("%d\n",ex_ma_rest.alu_result);//
            de_ex_rest.B=ex_ma_rest.alu_result;
        }
        else if(forwarding_unit.select_ex_B==2){
            de_ex_rest.B=wb_result;
        }
        else{
            cout<<"unidentified select_ex_A"<<endl;
        }
        //mux5;
        if(forwarding_unit.select_ex_op2==0){
            ;
        }
        else if(forwarding_unit.select_ex_op2==1){
            de_ex_rest.op2=wb_result;
        }
        else{
            cout<<"unidentified select_ex_A"<<endl;
        }

    }
    
    cout<<"\nEXECUTE STAGE"<<endl;
    long long int alu_result;
    alu_result=alu_unit(de_ex_rest.control.aluSignal);
    // printf("%d alu_result\n",alu_result);//
    if(de_ex_rest.control.branchSelect==0){
        //not jalr type
        branchPC=de_ex_rest.branch_target+de_ex_rest.PC;
    }
    else if(de_ex_rest.control.branchSelect==1){
        //if jalr then pc
        branchPC=alu_result;
    }
    if(de_ex_rest.control.branchSignal=="nbr"){
        de_ex_rest.control.setIsBranchTaken(false);
    }
    else if(de_ex_rest.control.branchSignal=="ubr"){
        de_ex_rest.control.setIsBranchTaken(true);
    }
    else{
        if(de_ex_rest.control.branchSignal=="beq"){
            if(alu_result==0){
                de_ex_rest.control.setIsBranchTaken(true);
            }
            else{
                de_ex_rest.control.setIsBranchTaken(false); 
            }   
        }
        else if(de_ex_rest.control.branchSignal=="bne"){
            if(alu_result!=0){
                de_ex_rest.control.setIsBranchTaken(true);
            }
            else{
                de_ex_rest.control.setIsBranchTaken(false); 
            }   
        }
        else if(de_ex_rest.control.branchSignal=="blt"){
            if(alu_result<0){
                de_ex_rest.control.setIsBranchTaken(true);
            }
            else{
                de_ex_rest.control.setIsBranchTaken(false); 
            }   
        }
        else if(de_ex_rest.control.branchSignal=="bge"){
            printf("Bge ##");//
            if(alu_result>=0){
                de_ex_rest.control.setIsBranchTaken(true);
            }
            else{
                de_ex_rest.control.setIsBranchTaken(false); 
            }   
        }    
    }
    if(de_ex_rest.control.isauipc){
        temp_ex_ma_rest.alu_result=alu_result+de_ex_rest.PC;
    }
    else{
        temp_ex_ma_rest.alu_result=alu_result;
    }
    temp_ex_ma_rest.instruction=de_ex_rest.instruction;
    temp_ex_ma_rest.op2=(unsigned int) de_ex_rest.op2;
    temp_ex_ma_rest.rd=(unsigned int) de_ex_rest.rd;
    temp_ex_ma_rest.rs1=de_ex_rest.rs1;
    temp_ex_ma_rest.rs2=de_ex_rest.rs2;
    temp_ex_ma_rest.PC=de_ex_rest.PC;
    temp_ex_ma_rest.control=de_ex_rest.control;
    if(knob2){
        if((forwarding_unit.ma_inst.opcode=="lb"
        ||forwarding_unit.ma_inst.opcode=="lh"
        ||forwarding_unit.ma_inst.opcode=="lw")){
            if(forwarding_unit.ex_inst.opcode=="sb"
            ||forwarding_unit.ex_inst.opcode=="sh"
            ||forwarding_unit.ex_inst.opcode=="sw"){
                if(forwarding_unit.ifDependencyrs1(forwarding_unit.ex_inst,forwarding_unit.ma_inst)){
                    //stall
                    n_stalls_data++;
                    PCWrite=false;
                    if_de_rest.writemode=false;
                    de_ex_rest.writemode=false;
                    temp_ex_ma_rest.instruction="00000000000000000000000000000000";
                    temp_ex_ma_rest.PC=0;
                    temp_ex_ma_rest.control.set_instruction(temp_ex_ma_rest.instruction);
                    temp_ex_ma_rest.control.build_control();
                }

            }
            else if(forwarding_unit.ifDependencyrs1(forwarding_unit.ex_inst,forwarding_unit.ma_inst)
            ||forwarding_unit.ifDependencyrs2(forwarding_unit.ex_inst,forwarding_unit.ma_inst)){
                //stall
                n_stalls_data++;
                PCWrite=false;
                if_de_rest.writemode=false;
                de_ex_rest.writemode=false;
                temp_ex_ma_rest.instruction="00000000000000000000000000000000";
                temp_ex_ma_rest.PC=0;
                temp_ex_ma_rest.control.set_instruction(temp_ex_ma_rest.instruction);
                temp_ex_ma_rest.control.build_control();
            }

        }     
    }
    printf("alu result :%u \n",temp_ex_ma_rest.alu_result);//
    printf("op2 : %u\n",temp_ex_ma_rest.op2);//
    printf("rd :%u\n",temp_ex_ma_rest.rd);//
    printf("isBranchTaken %d\n",de_ex_rest.control.isBranchTaken);
    printf("Branch PC(in hex) = %x\n",branchPC);
}

// //perform the memory operation
void mA(bool knob2) {
    if(ex_ma_rest.control.isexit){
        ex_ma_rest.writemode=false;
    }
    if(knob2){
        //forwarding
        unsigned int wb_result = 0;
        if (ma_wb_rest.control.isWb)
        {
            if (ma_wb_rest.control.wbSignal == "alu")
            {
                wb_result = ma_wb_rest.alu_result;
            }
            else if (ma_wb_rest.control.wbSignal == "ld")
            {
                wb_result = ma_wb_rest.ld_result;
            }
            else if (ma_wb_rest.control.wbSignal == "pc+4")
            {
                wb_result = ma_wb_rest.PC + 4;
            }
            else
            {
                cout << "error :undefined wbSignal" << endl;
            }
        }
        //mux6
        if(forwarding_unit.select_ma_op2==0){
            ;
        }
        else if(forwarding_unit.select_ma_op2==1){
            ex_ma_rest.op2=wb_result;
        }
        else{
            cout<<"unidentified select_ma_op2"<<endl;
        }
    }
    cout<<"\nMEMORY ACCESS STAGE"<<endl;
    unsigned int ldResult=0;
    char my_char;
    short int my_short_int;
    int my_int;

    //load operation
    if(ex_ma_rest.control.isLd){
        if(ex_ma_rest.control.nBytes==1){
            my_char=(char)memory_read((unsigned int)ex_ma_rest.alu_result,1);
            my_int=(int)my_char;
            ldResult=(unsigned int)my_int;
        }
        else if(ex_ma_rest.control.nBytes==2){
            my_short_int=(short int)memory_read((unsigned int)ex_ma_rest.alu_result,2);
            my_int=(int)my_short_int;
            ldResult=(unsigned int)my_int;
        }
        else if(ex_ma_rest.control.nBytes==4){
            ldResult=(int)memory_read((unsigned int)ex_ma_rest.alu_result,4);
        }
        else{
            cout<<"nBytes is "<<ex_ma_rest.control.nBytes<<"not supported"<<endl;
        }
    }
    else{
        ldResult=0;
    }

    //store operation
    if(ex_ma_rest.control.isSt){
        if(ex_ma_rest.control.nBytes==1){
            memory_write((unsigned int)ex_ma_rest.alu_result,(unsigned long long int) ex_ma_rest.op2,1);
        }
        else if(ex_ma_rest.control.nBytes==2){
            memory_write((unsigned int)ex_ma_rest.alu_result,(unsigned long long int) ex_ma_rest.op2,2);
        }
        else if(ex_ma_rest.control.nBytes==4){
            memory_write((unsigned int)ex_ma_rest.alu_result,(unsigned long long int) ex_ma_rest.op2,4);
        }
        else{
            cout<<"nBytes is "<<ex_ma_rest.control.nBytes<<"not supported"<<endl;
        }
    }
    temp_ma_wb_rest.instruction=ex_ma_rest.instruction;
    temp_ma_wb_rest.alu_result=ex_ma_rest.alu_result;
    temp_ma_wb_rest.ld_result=ldResult;
    temp_ma_wb_rest.rd=ex_ma_rest.rd;
    temp_ma_wb_rest.rs1=ex_ma_rest.rs1;
    temp_ma_wb_rest.rs2=ex_ma_rest.rs2;
    temp_ma_wb_rest.PC=ex_ma_rest.PC;
    temp_ma_wb_rest.control=ex_ma_rest.control;
    cout<<"LdResult :"<<temp_ma_wb_rest.ld_result<<endl;
    cout<<"rd :"<<temp_ma_wb_rest.rd<<endl;
}

// //writes the results back to register file
void write_back()
{   
    if(ma_wb_rest.control.isexit){
        EXIT=true;
        ma_wb_rest.writemode=false;
    }
    cout<<"\nWRITE BACK STAGE"<<endl;
    if (ma_wb_rest.control.isWb)
    {
        unsigned int wb_result = 0;
        if (ma_wb_rest.control.wbSignal == "alu")
        {
            wb_result = ma_wb_rest.alu_result;
        }
        else if (ma_wb_rest.control.wbSignal == "ld")
        {
            wb_result = ma_wb_rest.ld_result;
        }
        else if (ma_wb_rest.control.wbSignal == "pc+4")
        {
            wb_result = ma_wb_rest.PC + 4;
        }
        else
        {
            cout << "error :undefined wbSignal" << endl;
        }
        registerFile.set_register(ma_wb_rest.rd, wb_result);
        cout << "rd: " << ma_wb_rest.rd << "\nvalue: " << wb_result << endl;
    }
}
void print_bpu_details() {
    cout << "\n*** Branch Prediction Unit State ***" << endl;
    cout << "| PC Address      | BTP Target       | PHT State | Prediction |" << endl;
    cout << "|-----------------|------------------|-----------|------------|" << endl;
    
    for(int i=0; i<BTB_SIZE; i++) {
        if(BTB[i].address != -0xF) {
            // Calculate 2-bit PHT state from branch history
            string pht_state = "00";  // Default for uninitialized
            if(BTB[i].branch_taken) {
                pht_state = "11";  // Strongly taken
            } else {
                pht_state = "00";  // Strongly not-taken
            }

            printf("| %-15X | %-16X | %-9s | %-10s |\n", 
                   BTB[i].address,
                   BTB[i].branchPC,
                   pht_state.c_str(),
                   BTB[i].branch_taken ? "TAKEN" : "NOT TAKEN");
        }
    }
}


void positive_edge_trigger(){
    //
    if(ma_wb_rest.control.inst_type!="nop"&&ma_wb_rest.control.inst_type!="exit"){
        n_instruct++;
    }
    if(ma_wb_rest.control.inst_type=="sw"||
    ma_wb_rest.control.inst_type=="sh"||
    ma_wb_rest.control.inst_type=="sd"||
    ma_wb_rest.control.inst_type=="ld"||
    ma_wb_rest.control.inst_type=="lw"||
    ma_wb_rest.control.inst_type=="lh")
    {
        n_data_transfer++;
    }
    else if(ma_wb_rest.control.inst_type=="add"||
    ma_wb_rest.control.inst_type=="sub"||
    ma_wb_rest.control.inst_type=="xor"||
    ma_wb_rest.control.inst_type=="or"||
    ma_wb_rest.control.inst_type=="and"||
    ma_wb_rest.control.inst_type=="sll"||
    ma_wb_rest.control.inst_type=="srl"||
    ma_wb_rest.control.inst_type=="sra"||
    ma_wb_rest.control.inst_type=="slt"||
    ma_wb_rest.control.inst_type=="addi"||
    ma_wb_rest.control.inst_type=="xori"||
    ma_wb_rest.control.inst_type=="ori"||
    ma_wb_rest.control.inst_type=="andi"||
    ma_wb_rest.control.inst_type=="slli"||
    ma_wb_rest.control.inst_type=="srli"||
    ma_wb_rest.control.inst_type=="srai"||
    ma_wb_rest.control.inst_type=="slti"){
        n_ALU_instruct++;
    }
    else if(ma_wb_rest.control.inst_type=="beq"||
    ma_wb_rest.control.inst_type=="bne"||
    ma_wb_rest.control.inst_type=="blt"||
    ma_wb_rest.control.inst_type=="bge"||
    ma_wb_rest.control.inst_type=="jal"||
    ma_wb_rest.control.inst_type=="jalr")
    {
        n_control_instruct++;
    }
    if(knob4){
        if(if_de_rest.PC==check_inst){
            fprintf(sp_out,"\n\nIF_DE_REGISTOR\n");
            // fprintf(sp_out,"Instruction: %s",if_de_rest.instruction);
            fprintf(sp_out,"PC: %X\n",if_de_rest.PC);
        }
        if(de_ex_rest.PC==check_inst){
            fprintf(sp_out,"\n\n\nDE_EX_REGISTOR\n");
            fprintf(sp_out,"Branch target :%d\n",de_ex_rest.branch_target);//
            fprintf(sp_out,"A :%d\n",de_ex_rest.A);//
            fprintf(sp_out,"B :%d\n",de_ex_rest.B);//
            fprintf(sp_out,"op2 :%d\n",de_ex_rest.op2);//
            fprintf(sp_out,"rd :%d\n",de_ex_rest.rd); //
            fprintf(sp_out,"PC: %X\n",de_ex_rest.PC);
            fprintf(sp_out,"rs1 :%d\n",de_ex_rest.rs1); //
            fprintf(sp_out,"rs2 :%d\n",de_ex_rest.rs2); //
        }
        if(ex_ma_rest.PC==check_inst){
            fprintf(sp_out,"\n\n\nEX_MA_REGISTOR\n");
            fprintf(sp_out,"alu result :%u \n",ex_ma_rest.alu_result);//
            fprintf(sp_out,"op2 : %u\n",ex_ma_rest.op2);//
            fprintf(sp_out,"rd :%u\n",ex_ma_rest.rd);//
            fprintf(sp_out,"rs1 :%d\n",ex_ma_rest.rs1); //
            fprintf(sp_out,"rs2 :%d\n",ex_ma_rest.rs2); //
            fprintf(sp_out,"Branch PC(in hex) = %x\n",branchPC);
            fprintf(sp_out,"PC: %X\n",ex_ma_rest.PC);
        }
        if(ma_wb_rest.PC==check_inst){
            fprintf(sp_out,"\n\n\nMA_WB_REGISTOR\n");
            fprintf(sp_out,"alu result :%u \n",ma_wb_rest.alu_result);//
            fprintf(sp_out,"ld result :%u \n",ma_wb_rest.ld_result);//
            fprintf(sp_out,"rd :%u\n",ma_wb_rest.rd);//
            fprintf(sp_out,"rs1 :%d\n",ma_wb_rest.rs1); //
            fprintf(sp_out,"rs2 :%d\n",ma_wb_rest.rs2); //
            fprintf(sp_out,"PC: %X\n",ma_wb_rest.PC);

        }
    }
    if(knob6) {
        print_bpu_details();
    }
    //
    if((de_ex_rest.control.branchSignal!="nbr")&&(iscorrect_execute())){
        if(de_ex_rest.control.isBranchTaken){
            if(if_de_rest.PC!=branchPC){ // if prediction was false
                //if_de and de_ex set to nop
                n_control_hazards++;
                n_branch_mispredicts++;
                n_stalls_control+=2;
                temp_if_de_rest.instruction="00000000000000000000000000000000";
                temp_if_de_rest.PC=0;

                temp_de_ex_rest.A=0;temp_de_ex_rest.B=0;temp_de_ex_rest.branch_target=0;
                temp_de_ex_rest.control.set_instruction("00000000000000000000000000000000");
                temp_de_ex_rest.control.build_control();
                temp_de_ex_rest.op2=0;temp_de_ex_rest.PC=0;temp_de_ex_rest.rd=0;

                // //to avoid data depedency and branch conflict 
                PCWrite=true;
                if_de_rest.writemode=true;
                de_ex_rest.writemode=true;
                //ma is always written because never writemode is false;

                //update BTB
                {
                    unsigned int ind=BTB_hash(de_ex_rest.PC);
                    BTB[ind].address=de_ex_rest.PC;
                    BTB[ind].branchPC=branchPC;
                    BTB[ind].branch_taken=true;
                }

                nextPC=branchPC;
                
            }
        }
        else{
            if(if_de_rest.PC!=de_ex_rest.PC+4){
                //if_de and de_ex set to nop
                n_control_hazards++;
                n_branch_mispredicts++;
                n_stalls_control+=2;
                temp_if_de_rest.instruction="00000000000000000000000000000000";
                temp_if_de_rest.PC=0;

                temp_de_ex_rest.A=0;temp_de_ex_rest.B=0;temp_de_ex_rest.branch_target=0;
                temp_de_ex_rest.control.set_instruction("00000000000000000000000000000000");
                temp_de_ex_rest.control.build_control();
                temp_de_ex_rest.op2=0;temp_de_ex_rest.PC=0;temp_de_ex_rest.rd=0;

                //to avoid data depedency and branch conflict 
                PCWrite=true;
                if_de_rest.writemode=true;
                de_ex_rest.writemode=true;

                //update BTB
                {
                    unsigned int ind=BTB_hash(de_ex_rest.PC);
                    BTB[ind].address=de_ex_rest.PC;
                    BTB[ind].branchPC=branchPC;
                    BTB[ind].branch_taken=false;
                }

                nextPC=de_ex_rest.PC+4;
            }
        }
    }
    if(PCWrite){
        PC=nextPC;
    }
    if(BTB[BTB_hash(PC)].address==PC){
        if(BTB[BTB_hash(PC)].branch_taken){
            nextPC=BTB[BTB_hash(PC)].branchPC;
        }
        else{
            nextPC=PC+4;
        }
    }
    else{
        nextPC=PC+4;
    }
    if(if_de_rest.writemode){
        if_de_rest=temp_if_de_rest;
    }
    if(de_ex_rest.writemode){
        de_ex_rest=temp_de_ex_rest;
    }
    if(ex_ma_rest.writemode){
        ex_ma_rest=temp_ex_ma_rest;
    }
    if(ma_wb_rest.writemode){
        ma_wb_rest=temp_ma_wb_rest;
    }

    PCWrite=true;
    if_de_rest.writemode=true;
    de_ex_rest.writemode=true;
    ex_ma_rest.writemode=true;
    ma_wb_rest.writemode=true;
}

void display(){
    int ext=0,set_rst=1;
    while(!ext){
        printf("\n\n**** DISPLAY **** \n\n");
        while(set_rst!=0){
            registerFile.print_registers();
            cout<<"press '1' to set register\n"<< "press '0' to exit registerfile:";
            cin>>set_rst;
            if(set_rst==1){
                int rs,val;
                cout<<"Enter the register index in Range(0,31):";
                cin>>rs;
                cout<<"Current value of register : "<<registerFile.get_register(rs)<<endl;
                cout<<"Enter the value to insert :";
                cin>>val;
                registerFile.set_register(rs,val);
                cout<<"register file updated"<<endl;
            }
        }
        ext=0,set_rst=1;
        int mem_op=1;
        int op=0;
        cout<<"You are in memory section"<<endl;
        while(mem_op){
            cout<<"PRESS \n'0':exit\n'1':memory_lookup\n'2':memory update\nYour Choice :";
            cin>>op;
            if(op==0){
                break;
            }
            else if(op==1){
                int s_addr,e_addr;
                printf("Enter the range in hexa decimal format \nfrom start to end separated by space\nEg. 0x10002000 0x1000200c\nEnter :");
                scanf("%x %x",&s_addr,&e_addr);
                for(int i=0;i<=(e_addr-s_addr)/4;i++){
                    printf("%X %d\n",s_addr+(i*4),(unsigned int)memory_read(s_addr+(i*4),4));
                }
            }
            else if(op==2){
                unsigned addr;
                int val;
                printf("Enter addr in hexa decimal format Eg. 0x10002000\nEnter :");
                scanf("%x",&addr);
                printf("Current value of memory\n%X : %lld\n",addr,memory_read(addr,4));
                cout<<"Enter the new value of memory in decimal:";
                scanf("%d",&val);
                memory_write(addr,val,4);
            }
            else{
                cout<<"make a valid choice"<<endl;
            }   
        }
        printf("\n\n**** Enter 0 to exit display ****:");
        int dis_cod;
        cin>>dis_cod;
        if(dis_cod==0){
            return;
        }
    }
    
}
bool iscorrect_execute(){
    if((forwarding_unit.ma_inst.opcode=="lb"
        ||forwarding_unit.ma_inst.opcode=="lh"
        ||forwarding_unit.ma_inst.opcode=="lw")){
            if((forwarding_unit.ex_inst.opcode=="sb"
            ||forwarding_unit.ex_inst.opcode=="sh"
            ||forwarding_unit.ex_inst.opcode=="sw")){
                if(forwarding_unit.ifDependencyrs1(forwarding_unit.ex_inst,forwarding_unit.ma_inst)){
                    return false;
                }
            }
            else if(forwarding_unit.ifDependencyrs1(forwarding_unit.ex_inst,forwarding_unit.ma_inst)
            ||forwarding_unit.ifDependencyrs2(forwarding_unit.ex_inst,forwarding_unit.ma_inst)){
                return false;
            }
    }
    return true;

}

void print_pipeline_register(){
    printf("IF_DE Register\n");
    // printf("",if_de_rest.instruction)

}

//without pipeline

void run_riscvsim_without_pipeline() {
    PC=0;
    nextPC=0;
    EXIT=false;
    int i;
    string run_mode="STEP";
    string input;
    cout<<"type STEP or RUN"<<endl;
    cin>>input;
    if(input=="RUN"){
        run_mode="RUN";
    }
    while(1){
        fetch_without_pipeline();
        decode_without_pipeline();
        if(EXIT){
            EXIT=false;
            display();
            return;
        }
        execute_without_pipeline();
        mA_without_pipeline();
        write_back_without_pipeline();
        if(run_mode=="STEP"){
            display_without_pipeline();
            cout<<"type STEP or RUN"<<endl;
            cin>>input;
            if(input=="RUN"){
                run_mode="RUN";
            }
        }   
    }
}
void fetch_without_pipeline()
{
    if (without_pipeline_control_unit.isBranchTaken)
    {
        PC = branchPC;
    }
    else
    {
        PC = nextPC;
    }
    nextPC = PC + 4;
    cout<<"\nFETCH STAGE"<<endl;
    printf("Current PC=%x    ",PC);
    unsigned int instruct_dec = (unsigned int)memory_read((unsigned int)PC, 4);
    // printf("Instruction: %x ##\n",instruct_dec);
    string instruction = dec2bin(instruct_dec);
    if_de_rest.instruction = instruction;
    cout<<"Instruction  "<<if_de_rest.instruction<<endl;
}
// //reads the instruction register, reads operand1, operand2 fromo register file, decides the operation to be performed in execute stage
void decode_without_pipeline(){
    cout<<"\nDECODE STAGE"<<endl;
        //setting the controls
    without_pipeline_control_unit.set_instruction(if_de_rest.instruction);
    without_pipeline_control_unit.build_control();
    if(without_pipeline_control_unit.isexit){
        EXIT=true;
        return;
    }

    // getting destination register
    string rds=if_de_rest.instruction.substr(20,5);
    int rd=(int)unsgn_binaryToDecimal(rds);
    // getting source register 1
    string rs1s=if_de_rest.instruction.substr(12,5);
    int rs1=unsgn_binaryToDecimal(rs1s);
    //getting source register 2
    string rs2s=if_de_rest.instruction.substr(7,5);
    int rs2=unsgn_binaryToDecimal(rs2s);
    int imm=immediate(if_de_rest.instruction);

    de_ex_rest.rd=rd;
    de_ex_rest.A=registerFile.get_register(rs1);
    de_ex_rest.op2=registerFile.get_register(rs2);
    de_ex_rest.branch_target=imm;

    if(without_pipeline_control_unit.isImmediate){
        de_ex_rest.B=imm;
    }
    else{
        de_ex_rest.B=registerFile.get_register(rs2);
    }
    printf("branch target :%d\n",de_ex_rest.branch_target);//
    printf("A :%d\n",de_ex_rest.A);//
    printf("B :%d\n",de_ex_rest.B);//
    printf("op2 :%d\n",de_ex_rest.op2);//
    printf("rd :%d\n",de_ex_rest.rd); //
}


// //executes the ALU operation based on ALUop
void execute_without_pipeline(){
    cout<<"\nEXECUTE STAGE"<<endl;
    long long int alu_result;
    alu_result=alu_unit(without_pipeline_control_unit.aluSignal);
    // printf("%d alu_result\n",alu_result);//
    if(without_pipeline_control_unit.branchSelect==0){
        //not jalr type
        branchPC=de_ex_rest.branch_target+PC;
    }
    else if(without_pipeline_control_unit.branchSelect==1){
        //if jalr then pc
        branchPC=alu_result;
    }
    if(without_pipeline_control_unit.branchSignal=="nbr"){
        without_pipeline_control_unit.setIsBranchTaken(false);
    }
    else if(without_pipeline_control_unit.branchSignal=="ubr"){
        without_pipeline_control_unit.setIsBranchTaken(true);
    }
    else{
        if(without_pipeline_control_unit.branchSignal=="beq"){
            if(alu_result==0){
                without_pipeline_control_unit.setIsBranchTaken(true);
            }
            else{
                without_pipeline_control_unit.setIsBranchTaken(false); 
            }   
        }
        else if(without_pipeline_control_unit.branchSignal=="bne"){
            if(alu_result!=0){
                without_pipeline_control_unit.setIsBranchTaken(true);
            }
            else{
                without_pipeline_control_unit.setIsBranchTaken(false); 
            }   
        }
        else if(without_pipeline_control_unit.branchSignal=="blt"){
            if(alu_result<0){
                without_pipeline_control_unit.setIsBranchTaken(true);
            }
            else{
                without_pipeline_control_unit.setIsBranchTaken(false); 
            }   
        }
        else if(without_pipeline_control_unit.branchSignal=="bge"){
            printf("Bge ##");//
            if(alu_result>=0){
                without_pipeline_control_unit.setIsBranchTaken(true);
            }
            else{
                without_pipeline_control_unit.setIsBranchTaken(false); 
            }   
        }    
    }
    if(without_pipeline_control_unit.isauipc){
        ex_ma_rest.alu_result=alu_result+PC;
    }
    else{
        ex_ma_rest.alu_result=alu_result;
    }
    ex_ma_rest.op2=(unsigned int) de_ex_rest.op2;
    ex_ma_rest.rd=(unsigned int) de_ex_rest.rd;
    printf("alu result :%u \n",ex_ma_rest.alu_result);//
    printf("op2 : %u\n",ex_ma_rest.op2);//
    printf("rd :%u\n",ex_ma_rest.rd);//
    printf("Branch PC(in hex) = %x\n",branchPC);
}

// //perform the memory operation
void mA_without_pipeline() {
    cout<<"\nMEMORY ACCESS STAGE"<<endl;
    unsigned int ldResult=0;
    char my_char;
    short int my_short_int;
    int my_int;

    //load operation
    if(without_pipeline_control_unit.isLd){
        if(without_pipeline_control_unit.nBytes==1){
            my_char=(char)memory_read((unsigned int)ex_ma_rest.alu_result,1);
            my_int=(int)my_char;
            ldResult=(unsigned int)my_int;
        }
        else if(without_pipeline_control_unit.nBytes==2){
            my_short_int=(short int)memory_read((unsigned int)ex_ma_rest.alu_result,2);
            my_int=(int)my_short_int;
            ldResult=(unsigned int)my_int;
        }
        else if(without_pipeline_control_unit.nBytes==4){
            ldResult=(int)memory_read((unsigned int)ex_ma_rest.alu_result,4);
        }
        else{
            cout<<"nBytes is "<<without_pipeline_control_unit.nBytes<<"not supported"<<endl;
        }
    }
    else{
        ldResult=0;
    }

    //store operation
    if(without_pipeline_control_unit.isSt){
        if(without_pipeline_control_unit.nBytes==1){
            memory_write((unsigned int)ex_ma_rest.alu_result,(unsigned long long int) ex_ma_rest.op2,1);
        }
        else if(without_pipeline_control_unit.nBytes==2){
            memory_write((unsigned int)ex_ma_rest.alu_result,(unsigned long long int) ex_ma_rest.op2,2);
        }
        else if(without_pipeline_control_unit.nBytes==4){
            memory_write((unsigned int)ex_ma_rest.alu_result,(unsigned long long int) ex_ma_rest.op2,4);
        }
        else{
            cout<<"nBytes is "<<without_pipeline_control_unit.nBytes<<"not supported"<<endl;
        }   
    }
    ma_wb_rest.alu_result=ex_ma_rest.alu_result;
    ma_wb_rest.ld_result=ldResult;
    ma_wb_rest.rd=ex_ma_rest.rd;
    cout<<"LdResult :"<<ma_wb_rest.ld_result<<endl;
    cout<<"rd :"<<ma_wb_rest.rd<<endl;
}

// //writes the results back to register file
void write_back_without_pipeline()
{   
    cout<<"\nWRITE BACK STAGE"<<endl;
    if (without_pipeline_control_unit.isWb)
    {
        unsigned int wb_result = 0;
        if (without_pipeline_control_unit.wbSignal == "alu")
        {
            wb_result = ma_wb_rest.alu_result;
        }
        else if (without_pipeline_control_unit.wbSignal == "ld")
        {
            wb_result = ma_wb_rest.ld_result;
        }
        else if (without_pipeline_control_unit.wbSignal == "pc+4")
        {
            wb_result = PC + 4;
        }
        else
        {
            cout << "error :undefined wbSignal" << endl;
        }
        registerFile.set_register(ma_wb_rest.rd, wb_result);
        cout << "rd: " << ma_wb_rest.rd << "\nvalue: " << wb_result << endl;
    }
}

void display_without_pipeline(){
    int ext=0,set_rst=1;
    while(!ext){
        printf("\n\n**** DISPLAY **** \n\n");
        while(set_rst!=0){
            registerFile.print_registers();
            cout<<"press '1' to set register\n"<< "press '0' to exit registerfile:";
            cin>>set_rst;
            if(set_rst==1){
                int rs,val;
                cout<<"Enter the register index in Range(0,31):";
                cin>>rs;
                cout<<"Current value of register : "<<registerFile.get_register(rs)<<endl;
                cout<<"Enter the value to insert :";
                cin>>val;
                registerFile.set_register(rs,val);
                cout<<"register file updated"<<endl;
            }
        }
        ext=0,set_rst=1;
        int mem_op=1;
        int op=0;
        cout<<"You are in memory section"<<endl;
        while(mem_op){
            cout<<"PRESS \n'0':exit\n'1':memory_lookup\n'2':memory update\nYour Choice :";
            cin>>op;
            if(op==0){
                break;
            }
            else if(op==1){
                int s_addr,e_addr;
                printf("Enter the range in hexa decimal format \nfrom start to end separated by space\nEg. 0x10002000 0x1000200c\nEnter :");
                scanf("%x %x",&s_addr,&e_addr);
                for(int i=0;i<=(e_addr-s_addr)/4;i++){
                    printf("%X %d\n",s_addr+(i*4),(unsigned int)memory_read(s_addr+(i*4),4));
                }
            }
            else if(op==2){
                unsigned addr;
                int val;
                printf("Enter addr in hexa decimal format Eg. 0x10002000\nEnter :");
                scanf("%x",&addr);
                printf("Current value of memory\n%X : %lld\n",addr,memory_read(addr,4));
                cout<<"Enter the new value of memory in decimal:";
                scanf("%d",&val);
                memory_write(addr,val,4);
            }
            else{
                cout<<"make a valid choice"<<endl;
            }   
        }
        printf("\n\n**** Enter 0 to exit display ****:");
        int dis_cod;
        cin>>dis_cod;
        if(dis_cod==0){
            return;
        }
    }
    
}