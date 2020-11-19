#include "ysim.h"

#include "errors.h"

/************************** Utility Routines ****************************/

/** Return nybble from op (pos 0: least-significant; pos 1:
 *  most-significant)
 */
static Byte
get_nybble(Byte op, int pos) {
	return (op >> (pos * 4)) & 0xF;
}

/************************** Condition Codes ****************************/

/** Conditions used in instructions */
typedef enum {
	ALWAYS_COND, LE_COND, LT_COND, EQ_COND, NE_COND, GE_COND, GT_COND
} Condition;

/** accessing condition code flags */
static inline bool get_cc_flag(Byte cc, unsigned flagBitIndex) {
	return !!(cc & (1 << flagBitIndex));
}
static inline bool get_zf(Byte cc) { return get_cc_flag(cc, ZF_CC); }
static inline bool get_sf(Byte cc) { return get_cc_flag(cc, SF_CC); }
static inline bool get_of(Byte cc) { return get_cc_flag(cc, OF_CC); }

/** Return true iff the condition specified in the least-significant
 *  nybble of op holds in y86.  Encoding of Figure 3.15 of Bryant's
 *  CompSys3e.
 */
	bool
check_cc(const Y86 *y86, Byte op)
{
	bool ret = false;
	Condition condition = get_nybble(op, 0);
	Byte cc = read_cc_y86(y86);
	switch (condition) {
		case ALWAYS_COND:
			ret = true;
			break;
		case LE_COND:
			ret = (get_sf(cc) ^ get_of(cc)) | get_zf(cc);
			break;
		case LT_COND:
			ret = get_sf(cc) ^ get_of(cc);
			break;
		case EQ_COND:
			ret = get_zf(cc);
			break;
		case NE_COND:
			ret = !(get_zf(cc));
			break;
		case GE_COND:
			ret = !(get_sf(cc) ^ get_of(cc));
			break;
		case GT_COND:
			ret = !(get_sf(cc) ^ get_of(cc)) & !(get_zf(cc));
			break;
		default: {
			Address pc = read_pc_y86(y86);
			fatal("%08lx: bad condition code %d\n", pc, condition);
			break;
			}
	}
	return ret;
}

/** return true iff word has its sign bit set */
static inline bool
isLt0(Word word) {
	return (word & (1UL << (sizeof(Word)*CHAR_BIT - 1))) != 0;
}

/** Set condition codes for addition operation with operands opA, opB
 *  and result with result == opA + opB.
 */
static void
set_add_arith_cc(Y86 *y86, Word opA, Word opB, Word result)
{
	Byte cc = read_cc_y86(y86);
	if(result == 0)
		cc = !!(cc | (1 << ZF_CC));
	else if(isLt0(result)) {
		cc = !!(cc | (1 << SF_CC));
		if(!(isLt0(opA)) && !(isLt0(opB)))
			cc = !!(cc | (1 << OF_CC));
	}
	else
		if(isLt0(opA) && isLt0(opB))
			cc = !!(cc | (1 << OF_CC));

	write_cc_y86(y86, cc);
}

	/** Set condition codes for subtraction operation with operands opA, opB
	 *  and result with result == opA - opB.
	 */
static void
set_sub_arith_cc(Y86 *y86, Word opA, Word opB, Word result)
{
	Byte cc = read_cc_y86(y86);
	if(result == 0)
		cc = !!(cc | (1 << ZF_CC));
	else if(isLt0(result)) { 
		cc = !!(cc | (1 << SF_CC));
		if(!(isLt0(opA)) && isLt0(opB))
			cc = !!(cc | (1 << OF_CC));
	}
	else
		if(isLt0(opA) && !(isLt0(opB)))
			cc = !!(cc | (1 << OF_CC));

	write_cc_y86(y86, cc);

}

static void
set_logic_op_cc(Y86 *y86, Word result) {
	Byte cc = read_cc_y86(y86);
	if(result == 0)
		cc = !!(cc | (1 << ZF_CC));
	else if(isLt0(result)) 
		cc = !!(cc | (1 << SF_CC));
	
	cc = !!(cc & ~(1 << OF_CC));

	write_cc_y86(y86, cc);
}

/**************************** Operations *******************************/

static void
op1(Y86 *y86, Byte op, Register regA, Register regB)
{
	enum {ADDL_FN, SUBL_FN, ANDL_FN, XORL_FN};
	Word result = 0;
	Word opA = read_register_y86(y86, regA);
	Word opB = read_register_y86(y86, regB);

	switch(op) {
	case ADDL_FN: {
		result = opA + opB;
		set_add_arith_cc(y86, opA, opB, result);
		break;
		}
	case SUBL_FN: {
		result = opA - opB;
		set_sub_arith_cc(y86, opA, opB, result);
		break;
		}
	case ANDL_FN: {
		result = opA & opB;
		set_logic_op_cc(y86, result);
		break;
		}
	case XORL_FN: {
		result = opA ^ opB;
		set_logic_op_cc(y86, result);
		break;
		}
	default: {
	write_status_y86(y86, STATUS_INS);
	}
	write_register_y86(y86, regB, result);
  }
}

/*********************** Single Instruction Step ***********************/

typedef enum {
  HALT_CODE, NOP_CODE, CMOVxx_CODE, IRMOVQ_CODE, RMMOVQ_CODE, MRMOVQ_CODE,
  OP1_CODE, Jxx_CODE, CALL_CODE, RET_CODE,
  PUSHQ_CODE, POPQ_CODE } BaseOpCode;

/** Execute the next instruction of y86. Must change status of
 *  y86 to STATUS_HLT on halt, STATUS_ADR or STATUS_INS on
 *  bad address or instruction.
 */
void
step_ysim(Y86 *y86)
{
  //@TODO
	Address pc = read_pc_y86(y86);
	Byte opcode = read_memory_byte_y86(y86, pc);
	if(read_status_y86(y86) != STATUS_AOK) return;

	BaseOpCode baseCode = get_nybble(opcode, 1);
	switch(baseCode){
	case HALT_CODE: 
	    write_status_y86(y86, STATUS_HLT);
	    break;
	case NOP_CODE: //nop
	    write_pc_y86(y86, pc+sizeof(Byte));
	    break;
	case IRMOVQ_CODE: {
	    Byte nextByte = read_memory_byte_y86(y86, pc+sizeof(Byte));
	    if(read_status_y86(y86) != STATUS_AOK) return;
	    Byte reg = get_nybble(nextByte, 0);

	    Word imm = read_memory_word_y86(y86, pc+(2*sizeof(Byte)));
	    if(read_status_y86(y86) != STATUS_AOK) return;

	    write_register_y86(y86, reg, imm);

	    write_pc_y86(y86, pc + 1 + sizeof(Byte) + sizeof(Word));
	    break;
	    }
	 
	case CALL_CODE: {
	    Word callAddress = read_memory_word_y86(y86, pc + sizeof(Byte));//get call address
	    if(read_status_y86(y86) != STATUS_AOK) return;

	    Address returnAddress = pc + sizeof(Byte) + sizeof(Word); //fetch address of next instruction
	    Word stackPointer = read_register_y86(y86, REG_RSP); //fetch stack address
	    stackPointer -= sizeof(Word); //make room for return address
	    write_register_y86(y86, REG_RSP, stackPointer);//update stack address
	    write_memory_word_y86(y86, stackPointer, returnAddress);//write return address to decremented stack address
	    if(read_status_y86(y86) != STATUS_AOK) return;

	    write_pc_y86(y86, callAddress);//update pc to called function address
	    break;
	    }
	case RET_CODE: {
	    Word sPointer = read_register_y86(y86, REG_RSP);//fetch stack pointer address
	    Word returnAddress = read_memory_word_y86(y86, sPointer);//fetch word (the pushed return address) from stack pointer address
	    if(read_status_y86(y86) != STATUS_AOK) return;

	    write_pc_y86(y86, returnAddress);//update pc to return to next instruction
	    sPointer += sizeof(Word);//reset stackpointer
	    write_register_y86(y86, REG_RSP, sPointer);
	    break;
	    }
	case MRMOVQ_CODE: {
	    Byte temp = read_memory_byte_y86(y86, pc + sizeof(Byte));
	    if(read_status_y86(y86) != STATUS_AOK) return;

	    Byte rA = get_nybble(temp, 1);//grab rA, rB, and displacment
	    Byte rB = get_nybble(temp, 0);
	    Word displacement = read_memory_word_y86(y86, pc + 1 + sizeof(Byte));
	    if(read_status_y86(y86) != STATUS_AOK) return;

	    Address reg = read_register_y86(y86, rB);

	    write_register_y86(y86, rA, read_memory_word_y86(y86, reg + displacement));//write memory at rB + displacment into rA
	    write_pc_y86(y86, pc + 1 + sizeof(Byte) + sizeof(Word));//pc advance
	    break;
	    }
	case RMMOVQ_CODE: {
	    Byte temp = read_memory_byte_y86(y86, pc + sizeof(Byte));
	    if(read_status_y86(y86) != STATUS_AOK) return;

	    Byte rA = get_nybble(temp, 1);//grab rA, rB, and displacment
	    Byte rB = get_nybble(temp, 0);
	    Word displacement = read_memory_word_y86(y86, pc + 1 + sizeof(Byte));
	    if(read_status_y86(y86) != STATUS_AOK) return;

	    Word reg = read_register_y86(y86, rA);//read rA

	    write_memory_word_y86(y86, displacement + read_register_y86(y86, rB),
			   		 reg); //write rA into rB + displacment
	    write_pc_y86(y86, pc + 1 + sizeof(Byte) + sizeof(Word));//pc advance
	    break;
	    }
	case OP1_CODE: {
	    Byte temp = read_memory_byte_y86(y86, pc + sizeof(Byte));
	    if(read_status_y86(y86) != STATUS_AOK) return;

	    Byte rA = get_nybble(temp, 1);//grab rA, rB
	    Byte rB = get_nybble(temp, 0);
	    Byte op = get_nybble(opcode, 0);

	    op1(y86, op, rA, rB);
	    
	    write_pc_y86(y86, pc + 1 + sizeof(Byte));
	    }
	case CMOVxx_CODE: {
	    Byte temp = read_memory_byte_y86(y86, pc + sizeof(Byte));
	    if(read_status_y86(y86) != STATUS_AOK) return;

	    Byte rA = get_nybble(temp, 1);//grab rA, rB
	    Byte rB = get_nybble(temp, 0);
	    Byte op = get_nybble(opcode, 0);
 
	    if(check_cc(y86, op)){
		    Word reg = read_register_y86(y86, rA);//read rA
		    write_register_y86(y86, rB, reg);
	    } 
	    write_pc_y86(y86, pc + 1 + sizeof(Byte));
	}
	case PUSHQ_CODE: {
	    Word stackPointer = read_register_y86(y86, REG_RSP); //fetch stack add
	    stackPointer -= sizeof(Word); //make room for return address
	    write_register_y86(y86, REG_RSP, stackPointer);//update RSP

	    Byte temp = read_memory_byte_y86(y86, pc + sizeof(Byte));
	    if(read_status_y86(y86) != STATUS_AOK) return;
	    Byte rA = get_nybble(temp, 1);//grab rA

	    write_memory_word_y86(y86, stackPointer, read_register_y86(y86, rA));
	    if(read_status_y86(y86) != STATUS_AOK) return;//write rA into RSP

	    write_pc_y86(y86, pc + 1 + sizeof(Byte));
	}
	case POPQ_CODE: {
	    Word sPointer = read_register_y86(y86, REG_RSP);//fetch stack pointer address
	    Word pop = read_memory_word_y86(y86, sPointer);//fetch word from stack
	    if(read_status_y86(y86) != STATUS_AOK) return;

	    Byte temp = read_memory_byte_y86(y86, pc + sizeof(Byte));
	    if(read_status_y86(y86) != STATUS_AOK) return;
	    Byte rA = get_nybble(temp, 1);//grab rA

	    write_register_y86(y86, rA, pop);//update pc to return to next instruction
	    sPointer += sizeof(Word);//reset stackpointer
	    write_register_y86(y86, REG_RSP, sPointer);

	    write_pc_y86(y86, pc + 1 + sizeof(Byte));
	    break;
	}
	case Jxx_CODE: {
	    Word jump = read_memory_word_y86(y86, pc + 1);
	    if(read_status_y86(y86) != STATUS_AOK) return;

	    Byte op = get_nybble(opcode, 0);
	    if(check_cc(y86, op))
	    	write_pc_y86(y86, jump);
	    else
	        write_pc_y86(y86, pc + 1 + sizeof(Word));

	}
	default: {
	    write_status_y86(y86, STATUS_INS);
	    }

	}
}
