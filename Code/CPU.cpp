#include "CPU.h"

instruction::instruction(bitset<32> fetch)
{
	// cout << fetch << endl;
	instr = fetch;
	// cout << instr << endl;
}

CPU::CPU()
{
	//TODO: zero initialize ALU control signals here
	PC = 0; //set PC to 0
	nInstrRType = 0;
	for (int i = 0; i < 4096; i++) //copy instrMEM
	{
		dmemory[i] = (0);
	}
}

bitset<32> CPU::Fetch(bitset<8> *instmem) 
{
	//Fetch the next instruction from instmem
	bitset<32> instr = ((((instmem[PC + 3].to_ulong()) << 24)) + ((instmem[PC + 2].to_ulong()) << 16) + ((instmem[PC + 1].to_ulong()) << 8) + (instmem[PC + 0].to_ulong()));  //get 32 bit instruction
	nCycles++;
	 PC += 4;	//increment PC
	return instr;
}


bool CPU::Decode(instruction* curr)
{
	//Decode current instruction
	unsigned long instr = curr->instr.to_ulong();

	//Get the opcode type: I-type, R-type, SW, LW, JALR etc
	opcode_type OP_type = (opcode_type)(instr & 0b1111111);

	//Var to store the actual opcode i.e. ADD,SUB,XOR,ANDI,ADDI etc.
	opcode OPcode;

	//Operands to send to the ALU at the end
	int32_t operand1;
	int32_t operand2;

	//Variable to get different parts of the 32-bit instruction
	//such as funct3,funct7,opcode etc
	int bits_offset = 0;

	//initially set the offset to opcode size as we retrieved the opcode
	//above
	bits_offset = OPCODE_SZ_BITS;

	switch (OP_type)
	{
	case RTYPE:
	{
		nInstrRType++;
		//setting all control signals
		ctrl_block.RegWrite = 1;
		ctrl_block.ALUSrc = 0;
		ctrl_block.branch = 0;
		ctrl_block.MemRead = 0;
		ctrl_block.MemWrite = 0;
		ctrl_block.Mem2Reg = 0;
		ctrl_block.ALUOp = 0b10;
		//parsing information from the instruction
		destReg = ((instr >> bits_offset) & 0b11111);
		bits_offset += REG_SZ_BITS;
		int funct3 = (int)((instr >> bits_offset) & 0b111);
		bits_offset += FUNCT3_SZ_BITS;
		int rs1 = (int)((instr >> bits_offset) & 0b11111);
		bits_offset += REG_SZ_BITS;
		int rs2 = (int)((instr >> bits_offset) & 0b11111);
		bits_offset += REG_SZ_BITS;
		int funct7 = (int)((instr >> bits_offset) & 0b1111111);
		bits_offset = 0;
		#if defined(LOG)
		cout << "got stuff rd=" << destReg << ",rs1=" << rs1 << ",rs2=" << rs2 << endl;
		#endif
		//get rs1,rs2,funct3 and funct7

		//getting Opcode from opcode type funct3 and funct7 values
		switch (funct3)
		{
			case 0b101:
			{
				OPcode = SRA;
				#if defined(LOG)
				cout << "SRA instr" << endl;
				#endif
				ctrl_block.ALU_control = ALU_SHIFT_RIGHT;
				break;	
			}
			case 0b100:
			{
				OPcode = XOR;
				#if defined(LOG)
				cout << "XOR instr" << endl;
				#endif
				ctrl_block.ALU_control = ALU_XOR;
				break;
			}
			case 0b000:
			{
				switch (funct7)
				{
					case 0b0100000:
					{
						OPcode = SUB;
						#if defined(LOG)
						cout << "SUB instr" << endl;
						#endif
						ctrl_block.ALU_control = ALU_SUB;
						break;
					}
					case 0b0000000:
					{
						OPcode = ADD;
						#if defined(LOG)
						cout << "ADD instr" << endl;
						#endif
						ctrl_block.ALU_control = ALU_ADD;
						break;			
					}	
					default:
					{
						OPcode = UNKNOWN;
						#if defined(LOG)
						cout << "UNKNOWN funct7" << endl;
						#endif
						break;
					}
				}
				break;
			}
			default:
			{
				OPcode = UNKNOWN;
				#if defined(LOG)
				cout << "UNKNOWN funct3=" << funct3 << endl;
				#endif
				break;
			}	
			}

		//getting the value stored in the register
		//The work of the register file in the architecture
		readData1 = registerFile[rs1];
		readData2 = registerFile[rs2];

		//setting the operands to the ALU depending on the mux
		//and ALUSrc control signals
		operand1 = readData1;
		operand2 = mux(readData2, Imm_val, ctrl_block.ALUSrc.to_ulong());
		break;
	}

	//Having special case statement for LW and JALR as they have different
	//opcodes than ITYPE instruction. However, the format of the instruction 
	//is exactly the same as ITYPE. Hence, I have setup these case statements
	//in a way that fall-through to the ITYPE case statement and the parsing
	//happens in the same way. I handle the differences by adding conditional
	//if statements in the ITYPE case statement block
	case _LW:

	case _JALR:
	
	case ITYPE:
	{
		//setting all control signals
		ctrl_block.RegWrite = 1;
		ctrl_block.ALUSrc = 1;
		ctrl_block.Mem2Reg = 0;

		if(OP_type == _JALR)
		{
			OPcode = JALR;
			ctrl_block.branch = 1;
			ctrl_block.ALUOp = 0b01;
			ctrl_block.ALU_control = ALU_ADD;
		}
		else
		{
			ctrl_block.branch = 0;
		}
		
		if(OP_type == _LW)
		{
			OPcode = LW;
			ctrl_block.MemRead = 1;
			ctrl_block.ALUOp = 0b00;
			ctrl_block.Mem2Reg = 1;
			ctrl_block.ALU_control = ALU_ADD;
			#if defined(LOG)
			cout << "LW instr" << endl;
			#endif
		}
		else
		{
			ctrl_block.MemRead = 0;
		}

		//parsing information from the instruction
		ctrl_block.MemWrite = 0;
		destReg = (int)((instr >> bits_offset) & 0b11111);
		bits_offset += REG_SZ_BITS;
		int funct3 = (int)((instr >> bits_offset) & 0b111);
		bits_offset += FUNCT3_SZ_BITS;
		int rs1 = (int)((instr >> bits_offset) & 0b11111);
		bits_offset += REG_SZ_BITS;
		int imm = (int)((instr >> bits_offset) & 0b111111111111);

		//checking if the immediate value is a negative 12-bit value
		if(imm > 0b100000000000)
		{
			//if it is, sign extend by setting all the higher bits
			//this would make sure that negative 12-bit value is
			//recognized as a negative value and not a very high 
			//unsigned 12-bit value. This is needed as I am storing
			//a 12-bit imm value in a larger 32-bit (4 byte) container
			//i.e. int
			imm |= 0xfffff000;
		}
		bits_offset = 0;

		//getting the immediate value from the immediate generator							
		Imm_val = Imm_gen(OP_type, imm);
		
		// cout << "JALR or LW or ITYPE stuff" << endl;
		#if defined(LOG)
		cout << "got stuff rd=" << destReg << ",rs1=" << rs1 << ",Imm_val=" << Imm_val << ", imm=" << imm << endl;
		#endif

		//additional checks to get the opcode from the opcode_type using funct3 value
		switch (funct3)
		{
		case 0b111:
		{
			OPcode = ANDI;
			#if defined(LOG)
			cout << "ANDI instr" << endl;
			#endif
			ctrl_block.ALU_control = ALU_AND;
			break;
		}
		case 0b000:
		{
			if(OP_type != _JALR)
			{
				OPcode = ADDI;
				#if defined(LOG)
				cout << "ADDI instr" << endl;
				#endif
				ctrl_block.ALU_control = ALU_ADD;
			}
			else
			{
				#if defined(LOG)
				cout << "******JALR****** instr" << endl;
				#endif
			}

			break;
		}
		default:
			break;
		}
		
		//getting the value stored in the register
		//The work of the register file in the architecture
		readData1 = registerFile[rs1];
		
		//setting the operands to the ALU depending on the mux
		//and ALUSrc control signals
		operand1 = readData1;
		operand2 = mux(readData2, imm, ctrl_block.ALUSrc.to_ulong());

		break;
	}
	case STYPE:
	{
		//setting all control signals
		ctrl_block.RegWrite = 0;
		ctrl_block.ALUSrc = 1;
		ctrl_block.branch = 0;
		ctrl_block.MemRead = 0;
		ctrl_block.MemWrite = 1;
		ctrl_block.Mem2Reg = 1;
		ctrl_block.ALUOp = 0b00;
		OPcode = SW;
		ctrl_block.ALU_control = ALU_ADD;

		//parsing information from the instruction
		int imm1 = (int)((instr >> bits_offset) & 0b11111);
		bits_offset += IMM1_SZ_BITS;
		int funct3 = (int)((instr >> bits_offset) & 0b111);
		bits_offset += FUNCT3_SZ_BITS;
		int rs1 = (int)((instr >> bits_offset) & 0b11111);
		bits_offset += REG_SZ_BITS;
		int rs2 = (int)((instr >> bits_offset) & 0b11111);
		bits_offset += REG_SZ_BITS;
		int imm2 = (int)((instr >> bits_offset) & 0b1111111);
		bits_offset = 0;
		Imm_val = Imm_gen(OP_type, imm1, imm2);
		#if defined(LOG)
		cout << "got stuff rs2=" << rs2 << ",rs1=" << rs1 << ",imm=" << Imm_val << endl;
		#endif

		//getting the value stored in the register
		//The work of the register file in the architecture
		readData1 = registerFile[rs1];
		readData2 = registerFile[rs2];

		//setting the operands to the ALU depending on the mux
		//and ALUSrc control signals
		operand1 = readData1;
		operand2 = mux(readData2, Imm_val, ctrl_block.ALUSrc.to_ulong());
		#if defined(LOG)
		cout << "operand1 is " << readData1 << endl;
		cout << "operand2 is " << operand2 << endl;
		#endif
		break;
	}
	case BTYPE:
	{
		//setting all control signals
		ctrl_block.RegWrite = 0;
		ctrl_block.ALUSrc = 0;
		ctrl_block.branch = 1;
		ctrl_block.MemRead = 0;
		ctrl_block.MemWrite = 0;
		ctrl_block.Mem2Reg = 0;
		ctrl_block.ALUOp = 0b01;
		OPcode = BLT;
		ctrl_block.ALU_control = ALU_SUB;

		//parsing information from the instruction
		int imm1 = (int)((instr >> bits_offset) & 0b11111);
		bits_offset += IMM1_SZ_BITS;
		int funct3 = (int)((instr >> bits_offset) & 0b111);
		bits_offset += FUNCT3_SZ_BITS;
		int rs1 = (int)((instr >> bits_offset) & 0b11111);
		bits_offset += REG_SZ_BITS;
		int rs2 = (int)((instr >> bits_offset) & 0b11111);
		bits_offset += REG_SZ_BITS;
		int imm2 = (int)((instr >> bits_offset) & 0b1111111);
		bits_offset = 0;
		Imm_val = Imm_gen(OP_type, imm1, imm2);

		//getting the value stored in the register
		//The work of the register file in the architecture
		readData1 = registerFile[rs1];
		readData2 = registerFile[rs2];

		//setting the operands to the ALU depending on the mux
		//and ALUSrc control signals
		operand1 = readData1;
		operand2 = mux(readData2, Imm_val, ctrl_block.ALUSrc.to_ulong());

		#if defined(LOG)
		cout << "BLT instr,rs1=" << rs1 << ",rs2=" << rs2 << "got stuff imm=" << Imm_val << endl;
		cout << "op1=" << operand1 << " op2=" << operand2 << endl;
		#endif

		break;
	}
	case _EXIT:
	{
		//EXIT is the last instruction that returns false indicating
		//end of instruction execution
		#if defined(LOG)
		cout << "EXIT" << endl;
		#endif
		return false;
	}
	default:
		break;
	}

	//function to perform ALU operation
	Execute(operand1, operand2, ctrl_block.ALU_control);
	
	
	//Function to read or write to memory in case of sw or
	//lw instruction
	Memory(alu_res, readData2);
	
	//WriteBack function ensures output value is written 
	//to the destReg in case RegWrite control signal is high
	if(ctrl_block.RegWrite == 1)
	{
		WriteBack(alu_res, mem_block_output, destReg);
	}

	return true;
}

//function to perform ALU operation
bool CPU::Execute(int operand1, int operand2, int alu_control)
{
	//alu_control is the output of hte Control block to the ALU
	switch (alu_control)
	{
	case ALU_ADD:
	{
		alu_res = operand1 + operand2;
		#if defined(LOG)
		cout << "alu doing add;"; 
		cout << "got res=" << alu_res << ";";
		#endif
		break;
	}
	case ALU_SUB:
	{	
		alu_res = operand1 - operand2;
		#if defined(LOG)
		cout << "alu doing sub;";
		cout << "got res=" << alu_res << ";";
		#endif
		break;
	}
	case ALU_AND:
	{	
		alu_res = operand1 & operand2;
		#if defined(LOG)
		cout << "alu doing and;";
		cout << "got res=" << alu_res << ";";
		#endif
		break;
	}
	case ALU_OR:
	{	
		alu_res = operand1 | operand2;
		#if defined(LOG)
		cout << "alu doing or;";
		cout << "got res=" << alu_res << ";";
		#endif
		break;
	}
	case ALU_XOR:
	{	
		alu_res = operand1 ^ operand2;
		#if defined(LOG)
		cout << "alu doing xor;";
		cout << "got res=" << alu_res << ";";
		#endif
		break;
	}
	case ALU_SHIFT_RIGHT:
	{	
		alu_res = operand1 >> operand2;
		#if defined(LOG)
		cout << "alu doing shift right;";
		cout << "got res=" << alu_res << ";";
		#endif
		break;
	}
	default:
		break;
	}

	//setting the alu_neg signal to be used for BLT
	//operation
	alu_neg = (alu_res < 0) ? 1 : 0;

	//function that updates the PC in case there is a branch
	//or JALR instruction
	update_PC(Imm_val, alu_neg);
	//Calling update_PC here as it is done parallely at the end
	//of Execution stage and the start of memory stage

	return true;
}

//function that updates the PC in case there is a branch
//or JALR instruction
void CPU::update_PC(int Imm_val, bitset<1> alu_neg)
{
	#if defined(LOG)
	cout << "upd_PC imm=" << Imm_val << " alu_neg=" << alu_neg << " opcode=" << OPcode << ";";
	#endif
	if((ctrl_block.branch == 1) && (ctrl_block.RegWrite == 1))
	{
		PC = alu_res & ~1;
		#if defined(LOG)
		cout << "final PC=" << PC << ";" ;
		#endif
	}
	else
	{
		int val = (PC - 4) + (Imm_val << 1);
		
		PC = mux(PC, val, (ctrl_block.branch & alu_neg).to_ulong());
		#if defined(LOG)
		cout << "&res= \"" << (ctrl_block.branch & alu_neg).to_ulong() << "\"";
		// old_PC = PC;
		// int val = (PC - 4) + (Imm_val.to_ulong() << 1);
		cout << "final PC=" << PC << ";" ;
		#endif
	}
}

//Function to read or write to memory in case of sw or
//lw instruction
bool CPU::Memory(int alu_output, int reg2)
{
	bool flag = 0;
	if(ctrl_block.MemWrite == 1)
	{
		flag = 1;
		dmemory[alu_output] = reg2;
	}
	else if(ctrl_block.MemRead == 1)
	{
		flag = 1;
		mem_block_output = dmemory[alu_res];
	}
	#if defined(LOG)
	if(flag)
	{
		cout << "updated memory(";
		for(int i = 0; i < 20; i++)
		{
			cout << dmemory[i]<<"|";
		}
		cout << ");";
	}
	#endif
	return true;
}



//WriteBack function ensures output value is written 
//to the destReg in case RegWrite control signal is high
bool CPU::WriteBack(int alu_output, int mem_block_output, int rd)
{	
	#if defined(LOG)
	cout << "in wb oldpc=" << old_PC << " alu_res=" << alu_output << " mem output" << mem_block_output << " rd=" << rd << "mem2reg=" <<  ctrl_block.Mem2Reg.to_ulong()  << ";" ;
	#endif
	if((ctrl_block.branch == 1) && (ctrl_block.RegWrite == 1))
	{
		registerFile[rd] = old_PC + 4;
		// return true;
	}
	else
	{
		if(rd != 0)
			registerFile[rd] = mux(alu_output, mem_block_output, ctrl_block.Mem2Reg.to_ulong());
		//return true;
	}
	old_PC = PC;
	return true;
}

//Overloaded function to generate immediate
//This Imm_gen function is for instructions that take 2
//separate immediate values. These are STYPE(SW) and
//BTYPE(BLT) instructions
int CPU::Imm_gen(opcode_type type, int imm1, int imm2)
{
	int ret;
	switch (type)
	{
	case STYPE:
	{
		ret = (imm2 << 5) | imm1;
		break;
	}
	case BTYPE:
		ret = (imm1 & 1) << 11 | (imm1 & 0b11110) | (imm2 & 0b111111) << 5 | (imm2 & 0b1000000) << 5;
		ret >>= 1;
		break;
	
	default:
		break;
	}
	if(ret > 0b100000000000)
	{
		ret |= 0xfffff000;
	}
	return ret;
}

//Overloaded function to generate immediate
//This Imm_gen function is for instructions that have 
//only 1 immediate value in the instruction. These are 
//ITYPE instructions
int CPU::Imm_gen(opcode_type type, int imm1)
{
	if(imm1 > 0b100000000000)
	{
		imm1 |= 0xfffff000;
	}
	return imm1;
}

//Helper function to implement MUX operation
int CPU::mux(int in0, int in1, int sel)
{
	return sel ? in1 : in0;
}

//Helper function to read PC
unsigned long CPU::readPC()
{
	return PC;
}

//Helper function to read registerFile
//NOTE: only used for debugging and logs
int32_t CPU::read_regFile(int idx)
{
	return registerFile[idx];
}

//Helper function to read rType instruction
//counter variable in CPU class
int CPU::rTypeInstrCount(void)
{
	return nInstrRType;
}