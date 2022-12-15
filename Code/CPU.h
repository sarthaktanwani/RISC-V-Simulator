#include <iostream>
#include <bitset>
#include <stdio.h>
#include<stdlib.h>
#include <string>
using namespace std;

// #define LOG

#define OPCODE_SZ_BITS	7
#define REG_SZ_BITS		5
#define FUNCT3_SZ_BITS	3
#define FUNCT7_SZ_BITS	7
#define IMM1_SZ_BITS	REG_SZ_BITS


#define ALU_ADD			0b0010
#define ALU_SUB			0b0110
#define ALU_AND			0b0000
#define ALU_OR			0b0001
#define ALU_XOR			0b0100
#define ALU_SHIFT_RIGHT	0b0101



class instruction {
public:
	bitset<32> instr;//instruction
	instruction(bitset<32> fetch); // constructor

};

class CPU {
private:
	int dmemory[4096]; //data memory byte addressable in little endian fashion;
	unsigned long PC; //pc 
	unsigned long old_PC; 
	int32_t registerFile[32];       // Registers x0-x31
	int32_t readData1;
	int32_t readData2;
	int destReg;
	int Imm_val;
	int mem_block_output;
	int nInstrRType = 0;
	
	//ALU output signals
	int32_t alu_res;
	bitset<1> alu_neg;
	
	class Control {
		public:
		bitset<1> branch;
		bitset<1> MemRead;
		bitset<1> Mem2Reg;
		bitset<2> ALUOp;
		bitset<1> MemWrite;
		bitset<1> ALUSrc;
		bitset<1> RegWrite;
		Control()
		{
			branch = 0;
			MemRead = 0;
			Mem2Reg = 0;
			ALUOp = 0;
			MemWrite = 0;
			ALUSrc = 0;
			RegWrite = 0;
		}
		int ALU_control;
	};
	Control ctrl_block;

public:
	int nCycles = 0;
	CPU();

	enum opcode_type
	{
		//RTYPE
		// ADD = 0b0110011,
		// SUB = 0b0110011,
		// XOR = 0b0110011,
		// SRA = 0b0110011,
		RTYPE = 0b0110011,

		//ITYPE
		// ANDI = 0b0010011,
		// ADDI = 0b0010011,
		ITYPE = 0b0010011,
		
		//LW
		_LW = 0b0000011,
		
		//JALR
		_JALR = 0b1100111,
		
		//STYPE
		// SW = 0b0100011,
		STYPE = 0b0100011,

		//BTYPE
		// BLT = 0b1100011,
		BTYPE = 0b1100011,

		//EXIT
		_EXIT = 0b0000000

	};

	enum opcode
	{
		UNKNOWN,
		ADD,
		SUB,
		XOR,
		SRA,

		ANDI,
		ADDI,

		LW,
		
		JALR,

		SW,

		BLT,

		EXIT
	};

	unsigned long readPC();
	bitset<32> Fetch(bitset<8> *instmem);
	bool Decode(instruction* instr);
	bool Execute(int operand1, int operand2, int alu_control);
	bool Memory(int alu_output, int reg2);
	bool WriteBack(int alu_output, int mem_block_output, int rd);
	void update_PC(int Imm_val, bitset<1> alu_neg);
	
	int Imm_gen(opcode_type type, int imm1, int imm2);
	int Imm_gen(opcode_type type, int imm1);
	int mux(int in1, int in2, int sel);
	int rTypeInstrCount(void);
	int32_t read_regFile(int idx);
};