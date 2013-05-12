#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "functions.h"
#define debug 0


InstInfo * pipelineInsts[5];
void doStage();
void setDependency(InstInfo ** pipelineInsts, int *stall);
void forwardData(InstInfo ** pipelineInsts);
void clearInst(InstInfo * pipelineInsts);

int main(int argc, char *argv[])
{
	InstInfo curInst[5];
	InstInfo newInst;
	InstInfo * saved;
	int instnum = 0;
	int maxpc;
	int count=0;
	int cycles;			// total cycles in the pipeline
	int needsFetch = 1;	// fetching flag, 1 to fetch, 0 not to fetch
	int stall = 0;		// stall flag, 1 to stall, 0 not to stall
	int isTaken = 0;
	int i=0;
	int j=0;

	FILE *program;
	if (argc != 2)
	{
		printf("Usage: sim filename\n");
		exit(0);
	}

	maxpc = load(argv[1]);
	cycles = maxpc + 4;//**************************change to + 4 for REAL output

	if(debug)printf("cycles : %d \n" , cycles);

	if(debug)printLoad(maxpc);

	// initialize 5 instructions with 0
	for(i=0; i<5; i++){
		pipelineInsts[i]= &curInst[i];
		pipelineInsts[i]->inst=0;
		if(debug)printf("DEBUG: pipelineInsts[%d]: %d\n", i, pipelineInsts[i]->inst);
	}

	do{
		doStage(needsFetch, &stall);	// doStage calls stage functions
		saved = pipelineInsts[4];		// save the unused pointer for reuse
			
		// setting needsFetch flag
		if(count < maxpc)
			needsFetch = 1;
		else
			needsFetch = 0;

		if(debug)printf("DEBUG: stall value: %d\n", stall);

		// if stall is not being set
		if(!stall){
			if(debug)printf("DEBUG: no stall\n");
			if(debug)printf("DEBUG: not stall before, pipelineInst[0] ----> %d, %s\n", pipelineInsts[0]->inst, pipelineInsts[0]->string);
			printP2(pipelineInsts[0], pipelineInsts[1], pipelineInsts[2], pipelineInsts[3], pipelineInsts[4],  count);
	
			 //shift down 4 instructions
			for(i=4; i>0; i--){
				pipelineInsts[i]=pipelineInsts[i-1];
			}
			//printf("DEBUG: no stall, swap....\n");
			//printP2(pipelineInsts[0], pipelineInsts[1], pipelineInsts[2], pipelineInsts[3], pipelineInsts[4],  count);
			//saved->inst = 0;			// reset inst = 0
			pipelineInsts[0] = saved;	// reuse the unused inst pointer
			clearInst(pipelineInsts[0]);
			if(debug)printf("DEBUG: not stall after, pipelineInst[0] ----> %d, %s\n", pipelineInsts[0]->inst, pipelineInsts[0]->string);
		}
		// if stall is being set
		else{

			if(debug)printf("DEBUG: stall before, pipelineInst[0] ----> %d, %s\n", pipelineInsts[0]->inst, pipelineInsts[0]->string);
			if(debug)printf("DEBUG: stalls\n");
			printP2(pipelineInsts[0], pipelineInsts[1], pipelineInsts[2], pipelineInsts[3], pipelineInsts[4],  count);

			pipelineInsts[4] = pipelineInsts[3];	// shift down memory stage
			pipelineInsts[3] = pipelineInsts[2];	// shitf down execute stage
			saved->inst = 0;						// reset the unused inst to be 0
			pipelineInsts[2] = saved;				// stall the execute stage
			clearInst(pipelineInsts[2]);
			stall = 0;								// reset stall flag
			cycles++;
			if(debug)printf("DEBUG: stall after, pipelineInst[0] ----> %d, %s\n", pipelineInsts[0]->inst, pipelineInsts[0]->string);
			//printf("DEBUG: stall, swap....\n");
			//printP2(pipelineInsts[0], pipelineInsts[1], pipelineInsts[2], pipelineInsts[3], pipelineInsts[4],  count);
		}
		count++;					// increment count for cycles
	}while(count < cycles);

	// put in your own variables
	printf("Cycles: %d\n", count);
	printf("Instructions Executed: %d\n", maxpc);
  	exit(0);
}



/*
 * print out the loaded instructions.  This is to verify your loader
 * works.
 */
void printLoad(int max)
{
	int i;
	for(i=0;i<max;i++)
	printf("%d\n",instmem[i]);
}

/* print
 *
 * prints out the state of the simulator after each instruction
 */
void print(InstInfo *inst, int count)
{
	int i, j;
	printf("Instruction %d: %d\n",count,inst->inst);
	printf("%s\n\n",inst->string);
	printf("Fields:\n rd: %d\nrs: %d\nrt: %d\nimm: %d\n\n",
	 inst->fields.rd, inst->fields.rs, inst->fields.rt, inst->fields.imm);
	printf("Control Bits:\nalu: %d\nmw: %d\nmr: %d\nmtr: %d\nasrc: %d\nbt: %d\nrdst: %d\nrw: %d\n\n",inst->signals.aluop, inst->signals.mw, inst->signals.mr, inst->signals.mtr, inst->signals.asrc, inst->signals.btype, inst->signals.rdst, inst->signals.rw);
	printf("ALU Result: %d\n\n",inst->aluout);
	if (inst->signals.mr == 1)
		printf("Mem Result: %d\n\n",inst->memout);
	else
		printf("Mem Result: X\n\n");
	for(i=0;i<8;i++)
	{
		for(j=0;j<32;j+=8)
			printf("$%d: %4d ",i+j,regfile[i+j]);
		printf("\n");
	}
	printf("\n");
}


void printP2(InstInfo *inst0, InstInfo *inst1, InstInfo *inst2, InstInfo *inst3, InstInfo *inst4,  int count)
{
	int i, j;
	printf("Cycle %d:\n",count);
	if(inst0->inst != 0)
		printf("Fetch instruction: %d\n", inst0->inst);
	else
		printf("Fetch instruction: \n");
	if(inst1->inst != 0)
		printf("Decode instruction: %s\n", inst1->string);
	else
		printf("Decode instruction: \n");
	if(inst2->inst !=0)
		printf("Execute instruction: %s\n", inst2->string);
	else
		printf("Execute instruction: \n");

	if(inst3->inst !=0)
		printf("Memory instruction: %s\n", inst3->string);
	else
		printf("Memory instruction: \n");

	if(inst4->inst !=0)
		printf("Writeback instruction: %s\n", inst4->string);
	else
		printf("Writeback instruction: \n");

	for(i=0;i<8;i++)
	{
		for(j=0;j<32;j+=8)
			printf("$%d: %4d ",i+j,regfile[i+j]);
		printf("\n");
	}
	printf("\n");
}

void doStage(int isFirst, int *stall)
{
	if(pipelineInsts[4]->inst!=0){
		if(debug)printf("DEBUG: writing back\n");
		writeback(pipelineInsts[4]);
	}

	if(pipelineInsts[3]->inst!=0){
		if(debug)printf("DEBUG: memorying\n");
		memory(pipelineInsts[3]);
	}
	// if stall is being set, no need to call any of fowllowing functions
	if(!*stall){
		if(debug)printf("DEBUG: no stall...........\n");
		if(pipelineInsts[2]->inst!=0){
			if(debug)printf("DEBUG: executing\n");
			forwardData(pipelineInsts);	// forward data before execute

			if(debug)printf("DEBUG: instruction is about to get executed: %s\n", pipelineInsts[2]->string);
			execute(pipelineInsts[2]);
		}
		if(pipelineInsts[1]->inst!=0){
			if(debug)printf("DEBUG: decoding\n");
			decode(pipelineInsts[1]);
			setDependency(pipelineInsts, stall);	// set dependencies after decode
		}
	}
	if(pipelineInsts[0]->inst!=0 || isFirst){
		if(debug)printf("DEBUG: fetching\n");

		fetch(pipelineInsts[0]);
		if(*stall){		// when stall is being set, needs to defetch one instruction
			if(pipelineInsts[1]->signals.btype == 2){
				if(pipelineInsts[2]->aluout >= 0)
					pc = pc + pipelineInsts[1]->imm + 1;
			}
			defetch(pipelineInsts[0]);
		}
	}
}


void setDependency(InstInfo ** pipelineInsts, int *stall){
	
	InstInfo * cur = pipelineInsts[1]; 
	InstInfo * pre = pipelineInsts[2];
	InstInfo * prepre = pipelineInsts[3];

	if(debug)printf("DEBUG: IN setDependency: pre INST: %s\n", pre->string);
	if(pre->inst != 0 && pre->signals.rw){
		if(pre->fields.rt != 0){
			// if pre uses rt, cur uses rs
			if(pre->fields.rt == cur->fields.rs){
				if(debug)printf("DEBUG: setting depPreExec rs 1\n");
				cur->signals.depPreExec = 1;	//rs exec dependency
			}
			// if pre uses rt, cur uses rt
			else if(pre->fields.rt == cur->fields.rt){
				if(debug)printf("DEBUG: setting depPreExec rt 2\n");
				cur->signals.depPreExec = 2;	//rt exec dependency
			}
			
			// if pre writes to memory, cur rt and rs are same as pre rt
			if(pre->signals.mr && cur->fields.rt == pre->fields.rt && cur->fields.rs == pre->fields.rt){
				if(debug)printf("DEBUG: setting global stall=1, and cur inst to be stall in execution stage\n");
				cur->signals.depPreMem = 3;		// rt mem dependency 
				*stall = 1;
				cur->signals.stall = 1;
			}
			// if pre writes to memory, cur rs is same as pre rt
			else if(pre->signals.mr && cur->fields.rs == pre->fields.rt){
				if(debug)printf("DEBUG: setting global stall=1, and cur inst to be stall in execution stage\n");
				cur->signals.depPreMem = 1;		// rs mem dependency 
				*stall = 1;
				cur->signals.stall = 1;
			}
			// if pre writes to memory, cur rt is same as pre rt
			else if(pre->signals.mr && cur->fields.rt == pre->fields.rt){
				if(debug)printf("DEBUG: setting global stall=1, and cur inst to be stall in execution stage\n");
				cur->signals.depPreMem = 2;		// rt mem dependency 
				*stall = 1;
				cur->signals.stall = 1;
			}
		}
		if(pre->fields.rd != 0){
			// if pre uses rd, cur uses rs
			if(pre->fields.rd == cur->fields.rs){
				if(debug)printf("DEBUG: setting depPreExec rs 1\n");
				cur->signals.depPreExec = 1;		//rs exec dependency
			}
			// if pre uses rd, cur uses rt
			else if(pre->fields.rd == cur->fields.rt){
				if(debug)printf("DEBUG: setting depPreExec rt 2\n");
				cur->signals.depPreExec = 2;	//rt exec dependency
			}
		}

	}		
	
	if(prepre->inst != 0 && prepre->signals.rw){
		if(prepre->fields.rt != 0){
			// if prepre uses rt, cur uses rs
			if(prepre->fields.rt == cur->fields.rs){
				if(debug)printf("DEBUG: setting depPrePreExec rs 1\n");
				cur->signals.depPrePreExec = 1;	//rs exec dependency
			}
			// if prepre uses rt, cur uses rt
			else if(prepre->fields.rt == cur->fields.rt){
				if(debug)printf("DEBUG: setting depPrePreExec rt 2\n");
				cur->signals.depPrePreExec = 2;	//rt exec dependency
			}
		}
		if(prepre->fields.rd != 0){
			// if prepre uses rd, cur uses rs
			if(prepre->fields.rd == cur->fields.rs){
				if(debug)printf("DEBUG: setting depPrePreExec rs 1\n");
				cur->signals.depPrePreExec = 1;		//rs exec dependency
			}
			// if prepre uses rd, cur uses rt
			else if(prepre->fields.rd == cur->fields.rt){
				if(debug)printf("DEBUG: setting depPrePreExec rt 2\n");
				cur->signals.depPrePreExec = 2;	//rt exec dependency
			}
		}
	}
}

void forwardData(InstInfo ** pipelineInsts){
	InstInfo * cur = pipelineInsts[2]; 
	InstInfo * pre = pipelineInsts[3];
	InstInfo * prepre = pipelineInsts[4];

	if(cur->signals.depPreExec == 1){
		cur->s1data = pre->aluout;
	}
	else if(cur->signals.depPreExec == 2){
		cur->s2data = pre->aluout;
	}
	if(cur->signals.depPrePreExec == 1){
		cur->s1data = prepre->aluout;
	}
	else if(cur->signals.depPrePreExec == 2){
		cur->s2data = prepre->aluout;
	}

	// for mem dependency, since we have stalled,
	// so we need to get the data from pre pre instruction
	if(cur->signals.depPreMem == 1){
		cur->s1data = prepre->memout;
	}
	else if(cur->signals.depPreMem == 2){
		cur->s2data = prepre->memout;
	}
	else if(cur->signals.depPreMem == 3){
		cur->s1data = prepre->memout;
		cur->s2data = prepre->memout;
	}
}

void clearInst(InstInfo * inst){
	InstInfo newInst;
	inst = &newInst;
}

