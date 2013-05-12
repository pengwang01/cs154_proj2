
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// structures
typedef struct _signals{
        int aluop;
        int mw;
        int mr;
        int mtr;
        int asrc;
        int btype;
        int rdst;
        int rw;
	int depPreExec;
	int depPrePreExec;
        int depPreMem;
	int stall;	
} Signals;

/* these are the decoded fields for a single instruction */
typedef struct _fields{
        int rd;
        int rs;
        int rt;
        int imm;
        int op;
        int func;
} Fields;

/* this is all of the informmemoutation for a single instruction */
typedef struct _instinfo{
        int inst;
        Signals signals;
        Fields fields;
        int pc;
        int aluout;
        int memout;
        int sourcereg;
        int targetreg;
        int destreg;
        int destdata;
        char string[30];
        int s1data;
        int s2data;
        int input1;
        int input2;
} InstInfo;

// You need to fill in
int load(char *filename);
void fetch(InstInfo *);
void decode(InstInfo *);
void execute(InstInfo *);
void memory(InstInfo *);
void writeback(InstInfo *);


void defetch(InstInfo *);
void doBranching(InstInfo ** pipelineInsts, int * stall, int * cycles, int * counter, int * isTaken);

// this function is provided for you
 void print(InstInfo *, int);
 void printLoad(int);

// new print function to be used to print during pipeline
void printP2(InstInfo *inst0, InstInfo *inst1, InstInfo *inst2, InstInfo *inst3, InstInfo *inst4,  int count);


extern int pc;
extern int instmem[100];
extern int datamem[1000];
extern int regfile[32];

//this is for storing the instructions used during the stages of pipeline
extern InstInfo * pipelineInsts[5];

#endif

