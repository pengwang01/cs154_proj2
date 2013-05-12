//Group: Carina Rammelkamp, Andro Stotts, Peng Wang
//CS154, Project 1

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "functions.h"
#define debug 0
// these are the structures used in this simulator


// global variables
// register file
int regfile[32];
// instruction memory
int instmem[100];  // only support 100 static instructions
// data memory
int datamem[1000];
// program counter
int pc;

/* load
 *
 * Given the filename, which is a text file that 
 * contains the instructions stored as integers 
 *
 * You will need to load it into your global structure that 
 * stores all of the instructions.
 *
 * The return value is the maxpc - the number of instructions
 * in the file
 */
int load(char *filename)
{
	FILE *fr;
	long instruction;
	char line[80];
	int instrCounter = 0;

	fr = fopen(filename, "rt");	
	while(fgets(line, 80, fr) != NULL){
		if(line[0] == '\n')
			break;
		sscanf(line, "%ld", &instruction);
		instmem[instrCounter] = instruction;
		instrCounter++;
	}
	fclose(fr);
	return instrCounter; //***************************This may need to be instrCounter-1
}

/* fetch
 *
 * This fetches the next instruction and updates the program counter.
 * "fetching" means filling in the inst field of the instruction.
 */
void fetch(InstInfo *instruction)
{
	instruction->inst = instmem[pc];
	instruction->signals.depPreExec = 0;
 	instruction->signals.depPrePreExec = 0;
	instruction->signals.stall = 0;
	instruction->signals.depPreMem = 0;
	pc++;
	
}
void defetch(InstInfo *instruction)
{
	pc--;
}

/* decode
 *
 * This decodes an instruction.  It looks at the inst field of the 
 * instruction.  Then it decodes the fields into the fields data 
 * member.  The first one is given to you.
 *
 * Then it checks the op code.  Depending on what the opcode is, it
 * fills in all of the signals for that instruction.
 */
void decode(InstInfo *instruction)
{

	// fill in the signals and fields
	int val = instruction->inst;
	int op, func;
	instruction->fields.op = (val >> 26) & 0x03f;
	// fill in the rest of the fields here
	instruction->fields.func = val & 0x03f;
	instruction->fields.rs = (val >> 21) & 0x01f;
	instruction->fields.rt = (val >> 16) & 0x01f;
	instruction->fields.rd = (val >> 11) & 0x01f;
	instruction->fields.imm = val << 16 >> 16;

	// now fill in the signals
	if (instruction->fields.op == 48){
		//add
		if(instruction->fields.func == 10)
		{
			instruction->signals.aluop = 1;
			instruction->signals.mw = 0;
			instruction->signals.mtr = 0;
			instruction->signals.mr = 0;
			instruction->signals.asrc = 0;
			instruction->signals.btype = 0;
			instruction->signals.rdst = 1;
			instruction->signals.rw = 1;
			sprintf(instruction->string,"add $%d, $%d, $%d",
				instruction->fields.rd, instruction->fields.rs, 
				instruction->fields.rt);
		}
		//or
		else if (instruction->fields.func == 48){
			instruction->signals.aluop = 4;
			instruction->signals.mw = 0;
			instruction->signals.mtr = 0;
			instruction->signals.mr = 0;
			instruction->signals.asrc = 0;
			instruction->signals.btype = 0;
			instruction->signals.rdst = 1;
			instruction->signals.rw = 1;
			sprintf(instruction->string,"or $%d, $%d, $%d",
				instruction->fields.rd, instruction->fields.rs, 
				instruction->fields.rt);
		}
		//slt
		else if (instruction->fields.func == 15){
			instruction->signals.aluop = 6;
			instruction->signals.mw = 0;
			instruction->signals.mtr = 0;
			instruction->signals.mr = 0;
			instruction->signals.asrc = 0;
			instruction->signals.btype = 0;
			instruction->signals.rdst = 1;
			instruction->signals.rw = 1;
			sprintf(instruction->string,"slt $%d, $%d, $%d",
				instruction->fields.rd, instruction->fields.rs, 
				instruction->fields.rt);
		}
		//xor
		else if (instruction->fields.func == 20){
			instruction->signals.aluop = 3;
			instruction->signals.mw = 0;
			instruction->signals.mtr = 0;
			instruction->signals.mr = 0;
			instruction->signals.asrc = 0;
			instruction->signals.btype = 0;
			instruction->signals.rdst = 1;
			instruction->signals.rw = 1;
			sprintf(instruction->string,"xor $%d, $%d, $%d",
				instruction->fields.rd, instruction->fields.rs, 
				instruction->fields.rt);
		}
		else
		{
			//ERROR
		}
		instruction->destreg = instruction->fields.rd;	
		instruction->sourcereg = instruction->fields.rs;
		instruction->targetreg = instruction->fields.rt;
	}
	//subi
	else if(instruction->fields.op == 28){
		instruction->signals.aluop = 5;
		instruction->signals.mw = 0;
		instruction->signals.mtr = 0;
		instruction->signals.mr = 0;
		instruction->signals.asrc = 1;
		instruction->signals.btype = 0;
		instruction->signals.rdst = 0;
		instruction->signals.rw = 1;
		sprintf(instruction->string,"subi $%d, $%d, %d",
			instruction->fields.rt, instruction->fields.rs, 
			instruction->fields.imm);
		instruction->destreg = instruction->fields.rt;
		instruction->sourcereg = instruction->fields.rs;
	}
	//lw
	else if(instruction->fields.op == 6){
		instruction->signals.aluop = 1;
		instruction->signals.mw = 0;
		instruction->signals.mtr = 1;
		instruction->signals.mr = 1;
		instruction->signals.asrc = 1;
		instruction->signals.btype = 0;
		instruction->signals.rdst = 0;
		instruction->signals.rw = 1;
		sprintf(instruction->string,"lw $%d, %d ($%d)",
			instruction->fields.rt, instruction->fields.imm, 
			instruction->fields.rs);
		instruction->sourcereg = instruction->fields.rs;
		instruction->destreg = instruction->fields.rt;
		
	}
	//sw
	else if(instruction->fields.op == 2){
		instruction->signals.aluop = 1;
		instruction->signals.mw = 1;
		instruction->signals.mtr = -1;
		instruction->signals.mr = 0;
		instruction->signals.asrc = 1;
		instruction->signals.btype = 0;
		instruction->signals.rdst = -1;
		instruction->signals.rw = 0;
		sprintf(instruction->string,"sw $%d, %d ($%d)",
			instruction->fields.rt, instruction->fields.imm, 
			instruction->fields.rs);
		instruction->sourcereg= instruction->fields.rs;
		instruction->destreg = instruction->fields.rt;
	}
	//bge
	else if(instruction->fields.op == 39){
		instruction->signals.aluop = 5;
		instruction->signals.mw = 0;
		instruction->signals.mtr = -1;
		instruction->signals.mr = 0;
		instruction->signals.asrc = 0;
		instruction->signals.btype = 2;
		instruction->signals.rdst = -1;
		instruction->signals.rw = 0;
		sprintf(instruction->string,"bge $%d, $%d, %d",
			instruction->fields.rs, instruction->fields.rt,
			instruction->fields.imm);
		instruction->sourcereg = instruction->fields.rs;
		instruction->targetreg = instruction->fields.rt;
	}
	//j
	else if(instruction->fields.op == 36){
		instruction->signals.aluop = -1;
		instruction->signals.mw = 0;
		instruction->signals.mtr = -1;
		instruction->signals.mr = 0;
		instruction->signals.asrc = -1;
		instruction->signals.btype = 1;
		instruction->signals.rdst = -1;
		instruction->signals.rw = 0;
		sprintf(instruction->string,"j %d",
			val << 6 >> 6);
		instruction->fields.imm = val << 6 >> 6;
	}
	//jal
	else if(instruction->fields.op == 34){
		instruction->signals.aluop = -1;
		instruction->signals.mw = 0;
		instruction->signals.mtr = -1;
		instruction->signals.mr = 0;
		instruction->signals.asrc = -1;
		instruction->signals.btype = 1;
		instruction->signals.rdst = -1;
		instruction->signals.rw = 1;
		sprintf(instruction->string,"jal %d",
			val << 6 >> 6 );
		instruction->fields.imm = val << 6 >> 6;
	}
	else{
		//ERROR
	}
	if(instruction->signals.asrc == 1)
		instruction->s1data = regfile[instruction->sourcereg];
	else{
		if(instruction->fields.op == 48){
			instruction->s1data = regfile[instruction->sourcereg];
			instruction->s2data = regfile[instruction->targetreg];
			if(debug)printf("DEBUG: in decode stage----> s1: %d, s2: %d\n", instruction->s1data, instruction->s2data);
		}
		else if(instruction->fields.op == 39){
			instruction->s1data = regfile[instruction->sourcereg];
			instruction->s2data = regfile[instruction->targetreg];

		}
	}
	// fill in s1data and input2
}

/* execute
 *
 * This fills in the aluout value into the instruction and destdata
 */

void execute(InstInfo *instruction)
{
	// ALU source use imm
	if(instruction->signals.asrc == 1){
		//instruction->s1data = regfile[instruction->sourcereg];
		// subi
		if(instruction->fields.op == 28){
			instruction->aluout = instruction->s1data - instruction->fields.imm;
			instruction->destdata = instruction->aluout;
		}
		// sw or lw
		else if(instruction->fields.op == 2 || instruction->fields.op == 6){
			instruction->aluout = instruction->s1data + instruction->fields.imm;
			instruction->destdata = instruction->aluout;
		}
	}
	// ALU source use $rt
	else{
		if(instruction->fields.op == 48){
			//instruction->s1data = regfile[instruction->sourcereg];
			//instruction->s2data = regfile[instruction->targetreg];
			// add
			if(instruction->fields.func == 10){
				if(debug)printf("DEBUG: s1: %d, s2: %d\n", instruction->s1data, instruction->s2data);
				instruction->aluout = instruction->s1data + instruction->s2data;
			}
			// or
			else if(instruction->fields.func == 48){
				instruction->aluout = instruction->s1data | instruction->s2data;
			}
			// slt
			else if(instruction->fields.func == 15){
				if(instruction->s1data < instruction->s2data)
					instruction->aluout = 1;
				else
					instruction->aluout = 0;
			}
			// xor
			else if(instruction->fields.func == 20)
				instruction->aluout = instruction->s1data ^ instruction->s2data;
			// set destdata
			instruction->destdata = instruction->aluout;
		}
		// j
		else if(instruction->fields.op == 36){
			pc = instruction->fields.imm;
		}
		// jal
		else if(instruction->fields.op == 34){
			regfile[31] = pc;
			pc  = instruction->fields.imm;		
		}	
		
		// bge
		else if(instruction->fields.op == 39){
			//instruction->s1data = regfile[instruction->sourcereg];
			//instruction->s2data = regfile[instruction->targetreg];
			instruction->aluout = instruction->s1data - instruction->s2data;
			if(instruction->aluout >= 0)
				pc = pc + instruction->fields.imm;
		}
			
	}
}

/* memory
 *
 * If this is a load or a store, perform the memory operation
 */
void memory(InstInfo *instruction)
{
	// store, memory write
	if(instruction->signals.mw == 1)
		datamem[(instruction->aluout >> 2)] = regfile[instruction->destreg];
					
	// load, meory read
	if(instruction->signals.mr == 1){
		if(debug)printf("DEBUG: mem location: %d\n", instruction->aluout);
		instruction->destdata = datamem[(instruction->aluout >> 2)];
		instruction->memout = datamem[(instruction->aluout >> 2)];
		if(debug)printf("DEBUG: destdata: %d, memout: %d\n", instruction->destdata, instruction->memout);
	}
}

/* writeback
 *
 * If a register file is supposed to be written, write to it now
 */
void writeback(InstInfo *instruction)
{
	//printf("alu result: %d\n", instruction->aluout);
	if(instruction->signals.rw == 1)
		regfile[instruction->destreg] = instruction->destdata;
}

