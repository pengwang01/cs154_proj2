#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "functions.h"
#define debug 0


InstInfo * pipelineInsts[5];
void doStage();
void setDependency(InstInfo ** pipelineInsts, int *stall);
void forwardData(InstInfo ** pipelineInsts);

int main(int argc, char *argv[])
{
	InstInfo curInst[5];
	InstInfo newInst;
	InstInfo * saved;
	int instnum = 0;
	int maxpc;
	int count=0;
	int cycles;
	int needsFetch = 1;
	int rotator = 4;
	int stall = 0;

	FILE *program;
	if (argc != 2)
	{
		printf("Usage: sim filename\n");
		exit(0);
	}

	maxpc = load(argv[1]);
	cycles = maxpc + 5;//**************************change to + 4 for REAL output

	printf("cycles : %d \n" , cycles);

	printLoad(maxpc);
	int i=0;
	int j=0;
	for(i=0; i<5; i++){
		pipelineInsts[i]= &curInst[i];
		pipelineInsts[i]->inst=0;
		printf("pipelineInsts[%d]: %d\n", i, pipelineInsts[i]->inst);
	}

	do{
		doStage(needsFetch, &stall);
		// save the unused pointer
		saved = pipelineInsts[4];
		if(count < maxpc)
			needsFetch = 1;
		else
			needsFetch = 0;

		printf("DEBUG: stall value: %d\n", stall);
		if(!stall){
			printf("DEBUG: no stall\n");
			printP2(pipelineInsts[0], pipelineInsts[1], pipelineInsts[2], pipelineInsts[3], pipelineInsts[4],  count);
	
			 //shift right
			for(i=4; i>0; i--){
				pipelineInsts[i]=pipelineInsts[i-1];
			}
			pipelineInsts[0] = saved;
      			count++;
		}
		else{
			printf("DEBUG: stalls\n");
			printP2(pipelineInsts[0], pipelineInsts[1], pipelineInsts[2], pipelineInsts[3], pipelineInsts[4],  count);
			saved = pipelineInsts[4];
			pipelineInsts[4] = pipelineInsts[3];
			pipelineInsts[3] = pipelineInsts[2];
			saved->inst = 0;
			pipelineInsts[2] = saved;

			stall = 0;
		}
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
		printf("writing back\n");
		writeback(pipelineInsts[4]);
	}

	if(pipelineInsts[3]->inst!=0){
		printf("memorying\n");
		memory(pipelineInsts[3]);
	}
	if(!*stall){
		printf("no stall...........\n");
		if(pipelineInsts[2]->inst!=0){
			printf("executing\n");
			forwardData(pipelineInsts);	// forward data before execute

			printf("DEBUG: %s", pipelineInsts[2]->string);
			execute(pipelineInsts[2]);
		}
		if(pipelineInsts[1]->inst!=0){
			printf("decoding\n");
			decode(pipelineInsts[1]);
			setDependency(pipelineInsts, stall);	// set dependencies after decode
		}
		if(!*stall){
			if(pipelineInsts[0]->inst!=0 || isFirst){
				printf("fetching\n");
				fetch(pipelineInsts[0]);
			}
		}
	}
}


void setDependency(InstInfo ** pipelineInsts, int *stall){
	
	InstInfo * cur = pipelineInsts[1]; 
	InstInfo * pre = pipelineInsts[2];
	InstInfo * prepre = pipelineInsts[3];
	
	/*
	if(pre->inst != 0){
		printf("pre rd: %d\n",pre->fields.rd);
		printf("pre rt: %d\n",pre->fields.rt);
		printf("pre rw: %d\n",pre->signals.rw);
		printf("current rs: %d\n", cur->fields.rs);
		printf("current rt: %d\n", cur->fields.rt);
	}
	if(prepre->inst != 0){
		printf("prepre rd: %d\n",prepre->fields.rd);
		printf("prepre rt: %d\n",prepre->fields.rt);
		printf("prepre rw: %d\n",prepre->signals.rw);
		printf("current rs: %d\n", cur->fields.rs);
		printf("current rt: %d\n", cur->fields.rt);
	}
	*/

	if(pre->inst != 0 && pre->signals.rw){
		if(pre->fields.rt != 0){
			// if pre uses rt, cur uses rs
			if(pre->fields.rt == cur->fields.rs){
				printf("setting depPreExec rs 1\n");
				cur->signals.depPreExec = 1;	//rs dependency
			}
			// if pre uses rt, cur uses rt
			else if(pre->fields.rt == cur->fields.rt){
				printf("setting depPreExec rt 2\n");
				cur->signals.depPreExec = 2;	//rt dependency
			}
			if(cur->signals.mr && cur->fields.rs == pre->fields.rt){
				printf("setting global stall=1, and cur inst to be stall in execution stage\n");
				cur->signals.depPreMem = 1;		// rs dependency 
				*stall = 1;
				cur->signals.stall = 1;
			}
			if(cur->signals.mr && cur->fields.rt == pre->fields.rt){
				printf("setting global stall=1, and cur inst to be stall in execution stage\n");
				cur->signals.depPreMem = 2;		// rs dependency 
				*stall = 1;
				cur->signals.stall = 1;
			}
		}
		if(pre->fields.rd != 0){
			// if pre uses rd, cur uses rs
			if(pre->fields.rd == cur->fields.rs){
				printf("setting depPreExec rs 1\n");
				cur->signals.depPreExec = 1;		//rs dependency
			}
			// if pre uses rd, cur uses rt
			else if(pre->fields.rd == cur->fields.rt){
				printf("setting depPreExec rt 2\n");
				cur->signals.depPreExec = 2;	//rt dependency
			}
		}

	}		
	
	if(prepre->inst != 0 && prepre->signals.rw){
		if(prepre->fields.rt != 0){
			// if prepre uses rt, cur uses rs
			if(prepre->fields.rt == cur->fields.rs){
				printf("setting depPrePreExec rs 1\n");
				cur->signals.depPrePreExec = 1;	//rs dependency
			}
			// if prepre uses rt, cur uses rt
			else if(prepre->fields.rt == cur->fields.rt){
				printf("setting depPrePreExec rt 2\n");
				cur->signals.depPrePreExec = 2;	//rt dependency
			}
		}
		if(prepre->fields.rd != 0){
			// if prepre uses rd, cur uses rs
			if(prepre->fields.rd == cur->fields.rs){
				printf("setting depPrePreExec rs 1\n");
				cur->signals.depPrePreExec = 1;		//rs dependency
			}
			// if prepre uses rd, cur uses rt
			else if(prepre->fields.rd == cur->fields.rt){
				printf("setting depPrePreExec rt 2\n");
				cur->signals.depPrePreExec = 2;	//rt dependency
			}
		}
	}
	// if pre inst is not 0, and pre inst is writing to memory
	/*if(pre->inst != 0 && pre->signals.rw && pre->signals.mr){
		printf("pre is not 0, and memory write\n");
		if(cur->signals.mr)
			printf("cur inst do memory reads\n");
		printf("cur s1: %d, cur imm: %d, pre alu: %d\n", cur->s1data, cur->fields.imm, pre->aluout);
		if(cur->signals.mr && ((cur->s1data + cur->fields.imm) == pre->aluout)){
			printf("setting global stall=1, and cur inst to be stall in execution stage\n");
			*stall = 1;
			cur->signals.stall = 1;
		}
	}*/
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
	if(cur->signals.depPreMem == 1){
		cur->s1data = pre->memout;
	}
	else if(cur->signals.depPreMem == 2){
		cur->s2data = pre->memout;
	}
}

