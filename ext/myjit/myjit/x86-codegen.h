/*
 * x86-codegen.h: Macros for generating x86 code
 *
 * Authors:
 *   Petr Krajca (petr.krajca@upol.cz)
 *   Paolo Molaro (lupus@ximian.com)
 *   Intel Corporation (ORP Project)
 *   Sergey Chaban (serge@wildwestsoftware.com)
 *   Dietmar Maurer (dietmar@ximian.com)
 *   Patrik Torstensson
 *
 * Copyright (C)  2000 Intel Corporation.  All rights reserved.
 * Copyright (C)  2001, 2002 Ximian, Inc.
 */

#ifndef X86_H
#define X86_H
#include <assert.h>
#define g_assert(x) assert(x)
/*
// x86 register numbers
*/
typedef enum jit_x86_gp_regs {
	X86_EAX = 0,
	X86_ECX = 1,
	X86_EDX = 2,
	X86_EBX = 3,
	X86_ESP = 4,
	X86_EBP = 5,
	X86_ESI = 6,
	X86_EDI = 7,
	X86_NREG
} X86_Reg_No;

typedef enum jit_x86_fp_regs {
	X86_XMM0,
	X86_XMM1,
	X86_XMM2,
	X86_XMM3,
	X86_XMM4,
	X86_XMM5,
	X86_XMM6,
	X86_XMM7,
	X86_XMM_NREG
} X86_XMM_Reg_No;

/*
// opcodes for alu instructions
*/
typedef enum {
	X86_ADD = 0,
	X86_OR  = 1,
	X86_ADC = 2,
	X86_SBB = 3,
	X86_AND = 4,
	X86_SUB = 5,
	X86_XOR = 6,
	X86_CMP = 7,
	X86_NALU
} X86_ALU_Opcode;
/*
// opcodes for shift instructions
*/
typedef enum {
	X86_SHLD,
	X86_SHLR,
	X86_ROL = 0,
	X86_ROR = 1,
	X86_RCL = 2,
	X86_RCR = 3,
	X86_SHL = 4,
	X86_SHR = 5,
	X86_SAR = 7,
	X86_NSHIFT = 8
} X86_Shift_Opcode;
/*
// opcodes for floating-point instructions
*/
typedef enum {
	X86_FADD  = 0,
	X86_FMUL  = 1,
	X86_FCOM  = 2,
	X86_FCOMP = 3,
	X86_FSUB  = 4,
	X86_FSUBR = 5,
	X86_FDIV  = 6,
	X86_FDIVR = 7,
	X86_NFP   = 8
} X86_FP_Opcode;
/*
// integer conditions codes
*/
typedef enum {
	X86_CC_EQ = 0, X86_CC_E = 0, X86_CC_Z = 0,
	X86_CC_NE = 1, X86_CC_NZ = 1,
	X86_CC_LT = 2, X86_CC_B = 2, X86_CC_C = 2, X86_CC_NAE = 2,
	X86_CC_LE = 3, X86_CC_BE = 3, X86_CC_NA = 3,
	X86_CC_GT = 4, X86_CC_A = 4, X86_CC_NBE = 4,
	X86_CC_GE = 5, X86_CC_AE = 5, X86_CC_NB = 5, X86_CC_NC = 5,
	X86_CC_LZ = 6, X86_CC_S = 6,
	X86_CC_GEZ = 7, X86_CC_NS = 7,
	X86_CC_P = 8, X86_CC_PE = 8,
	X86_CC_NP = 9, X86_CC_PO = 9,
	X86_CC_O = 10,
	X86_CC_NO = 11,
	X86_NCC
} X86_CC;

/* FP status */
enum {
	X86_FP_C0 = 0x100,
	X86_FP_C1 = 0x200,
	X86_FP_C2 = 0x400,
	X86_FP_C3 = 0x4000,
	X86_FP_CC_MASK = 0x4500
};

/* FP control word */
enum {
	X86_FPCW_INVOPEX_MASK = 0x1,
	X86_FPCW_DENOPEX_MASK = 0x2,
	X86_FPCW_ZERODIV_MASK = 0x4,
	X86_FPCW_OVFEX_MASK   = 0x8,
	X86_FPCW_UNDFEX_MASK  = 0x10,
	X86_FPCW_PRECEX_MASK  = 0x20,
	X86_FPCW_PRECC_MASK   = 0x300,
	X86_FPCW_ROUNDC_MASK  = 0xc00,

	/* values for precision control */
	X86_FPCW_PREC_SINGLE    = 0,
	X86_FPCW_PREC_DOUBLE    = 0x200,
	X86_FPCW_PREC_EXTENDED  = 0x300,

	/* values for rounding control */
	X86_FPCW_ROUND_NEAREST  = 0,
	X86_FPCW_ROUND_DOWN     = 0x400,
	X86_FPCW_ROUND_UP       = 0x800,
	X86_FPCW_ROUND_TOZERO   = 0xc00
};

/*
// prefix code
*/
typedef enum {
	X86_LOCK_PREFIX = 0xF0,
	X86_REPNZ_PREFIX = 0xF2,
	X86_REPZ_PREFIX = 0xF3,
	X86_REP_PREFIX = 0xF3,
	X86_CS_PREFIX = 0x2E,
	X86_SS_PREFIX = 0x36,
	X86_DS_PREFIX = 0x3E,
	X86_ES_PREFIX = 0x26,
	X86_FS_PREFIX = 0x64,
	X86_GS_PREFIX = 0x65,
	X86_UNLIKELY_PREFIX = 0x2E,
	X86_LIKELY_PREFIX = 0x3E,
	X86_OPERAND_PREFIX = 0x66,
	X86_ADDRESS_PREFIX = 0x67
} X86_Prefix;

static const unsigned char
x86_cc_unsigned_map [X86_NCC] = {
	0x74, /* eq  */
	0x75, /* ne  */
	0x72, /* lt  */
	0x76, /* le  */
	0x77, /* gt  */
	0x73, /* ge  */
	0x78, /* lz  */
	0x79, /* gez */
	0x7a, /* p   */
	0x7b, /* np  */
	0x70, /* o  */
	0x71, /* no  */
};

static const unsigned char
x86_cc_signed_map [X86_NCC] = {
	0x74, /* eq  */
	0x75, /* ne  */
	0x7c, /* lt  */
	0x7e, /* le  */
	0x7f, /* gt  */
	0x7d, /* ge  */
	0x78, /* lz  */
	0x79, /* gez */
	0x7a, /* p   */
	0x7b, /* np  */
	0x70, /* o  */
	0x71, /* no  */
};

typedef union {
	int val;
	unsigned char b [4];
} x86_imm_buf;

#define X86_NOBASEREG (-1)

/*
// bitvector mask for callee-saved registers
*/
#define X86_ESI_MASK (1<<X86_ESI)
#define X86_EDI_MASK (1<<X86_EDI)
#define X86_EBX_MASK (1<<X86_EBX)
#define X86_EBP_MASK (1<<X86_EBP)

#define X86_CALLEE_REGS ((1<<X86_EAX) | (1<<X86_ECX) | (1<<X86_EDX))
#define X86_CALLER_REGS ((1<<X86_EBX) | (1<<X86_EBP) | (1<<X86_ESI) | (1<<X86_EDI))
#define X86_BYTE_REGS   ((1<<X86_EAX) | (1<<X86_ECX) | (1<<X86_EDX) | (1<<X86_EBX))

#define X86_IS_SCRATCH(reg) (X86_CALLER_REGS & (1 << (reg))) /* X86_EAX, X86_ECX, or X86_EDX */
#define X86_IS_CALLEE(reg)  (X86_CALLEE_REGS & (1 << (reg))) 	/* X86_ESI, X86_EDI, X86_EBX, or X86_EBP */

#define X86_IS_BYTE_REG(reg) ((reg) < 4)

/*
// Frame structure:
//
//      +--------------------------------+
//      | in_arg[0]       = var[0]	     |
//      | in_arg[1]	      = var[1]	     |
//      |	      . . .			         |
//      | in_arg[n_arg-1] = var[n_arg-1] |
//      +--------------------------------+
//      |       return IP                |
//      +--------------------------------+
//      |       saved EBP                | <-- frame pointer (EBP)
//      +--------------------------------+
//      |            ...                 |  n_extra
//      +--------------------------------+
//      |	    var[n_arg]	             |
//      |	    var[n_arg+1]             |  local variables area
//      |          . . .                 |
//      |	    var[n_var-1]             |
//      +--------------------------------+
//      |			                     |
//      |			                     |
//      |		spill area               | area for spilling mimic stack
//      |			                     |
//      +--------------------------------|
//      |          ebx                   |
//      |          ebp [ESP_Frame only]  |
//      |	       esi                   |  0..3 callee-saved regs
//      |          edi                   | <-- stack pointer (ESP)
//      +--------------------------------+
//      |	stk0	                     |
//      |	stk1	                     |  operand stack area/
//      |	. . .	                     |  out args
//      |	stkn-1	                     |
//      +--------------------------------|
//
//
*/


/*
 * useful building blocks
 */
#define x86_modrm_mod(modrm) ((modrm) >> 6)
#define x86_modrm_reg(modrm) (((modrm) >> 3) & 0x7)
#define x86_modrm_rm(modrm) ((modrm) & 0x7)

#define x86_address_byte(inst,m,o,r) do { *(inst)++ = ((((m)&0x03)<<6)|(((o)&0x07)<<3)|(((r)&0x07))); } while (0)
#define x86_imm_emit32(inst,imm)     \
	do {	\
			x86_imm_buf imb; imb.val = (int) (imm);	\
			*(inst)++ = imb.b [0];	\
			*(inst)++ = imb.b [1];	\
			*(inst)++ = imb.b [2];	\
			*(inst)++ = imb.b [3];	\
	} while (0)
#define x86_imm_emit16(inst,imm)     do { *(short*)(inst) = (imm); (inst) += 2; } while (0)
#define x86_imm_emit8(inst,imm)      do { *(inst) = (unsigned char)((imm) & 0xff); ++(inst); } while (0)
#define x86_is_imm8(imm)             (((int)(imm) >= -128 && (int)(imm) <= 127))
#define x86_is_imm16(imm)            (((int)(imm) >= -(1<<16) && (int)(imm) <= ((1<<16)-1)))

#define x86_reg_emit(inst,r,regno)   do { x86_address_byte ((inst), 3, (r), (regno)); } while (0)
#define x86_reg8_emit(inst,r,regno,is_rh,is_rnoh)   do {x86_address_byte ((inst), 3, (is_rh)?((r)|4):(r), (is_rnoh)?((regno)|4):(regno));} while (0)
#define x86_regp_emit(inst,r,regno)  do { x86_address_byte ((inst), 0, (r), (regno)); } while (0)
#define x86_mem_emit(inst,r,disp)    do { x86_address_byte ((inst), 0, (r), 5); x86_imm_emit32((inst), (disp)); } while (0)

#define x86_membase_emit(inst,r,basereg,disp)	do {\
    if ((basereg) == AMD64_RIP) { \
        x86_address_byte ((inst), 0, (r), (0b101));	\
        x86_imm_emit32 ((inst), (disp));	\
        break; \
    }  \
	if ((basereg) == X86_ESP) {	\
		if ((disp) == 0) {	\
			x86_address_byte ((inst), 0, (r), X86_ESP);	\
			x86_address_byte ((inst), 0, X86_ESP, X86_ESP);	\
		} else if (x86_is_imm8((disp))) {	\
			x86_address_byte ((inst), 1, (r), X86_ESP);	\
			x86_address_byte ((inst), 0, X86_ESP, X86_ESP);	\
			x86_imm_emit8 ((inst), (disp));	\
		} else {	\
			x86_address_byte ((inst), 2, (r), X86_ESP);	\
			x86_address_byte ((inst), 0, X86_ESP, X86_ESP);	\
			x86_imm_emit32 ((inst), (disp));	\
		}	\
		break;	\
	}	\
	if ((disp) == 0 && (basereg) != X86_EBP) {	\
		if(basereg==0b1100) { \
			x86_address_byte ((inst), 0, (r), (basereg));	\
			x86_address_byte ((inst), 0, (basereg), 4);	\
		} else \
		x86_address_byte ((inst), 0, (r), (basereg));	\
		break;	\
	}	\
	if (x86_is_imm8((disp))) {	\
		if(basereg==0b1100) { \
			x86_address_byte ((inst), 1, (r), (basereg));	\
			x86_address_byte ((inst), 0, (basereg), 4);	\
		} else \
		x86_address_byte ((inst), 1, (r), (basereg));	\
		x86_imm_emit8 ((inst), (disp));	\
	} else {	\
		if(basereg==0b1100) { \
			x86_address_byte ((inst), 2, (r), (basereg));	\
			x86_address_byte ((inst), 0, (basereg), 4);	\
		} else \
		x86_address_byte ((inst), 2, (r), (basereg));	\
		x86_imm_emit32 ((inst), (disp));	\
	}	\
	} while (0)

#define x86_memindex_emit(inst,r,basereg,disp,indexreg,shift)	\
	do {	\
		if ((basereg) == X86_NOBASEREG) {	\
			x86_address_byte ((inst), 0, (r), 4);	\
			x86_address_byte ((inst), (shift), (indexreg), 5);	\
			x86_imm_emit32 ((inst), (disp));	\
		} else if ((disp) == 0 && (basereg) != X86_EBP) {	\
			x86_address_byte ((inst), 0, (r), 4);	\
			x86_address_byte ((inst), (shift), (indexreg), ((basereg)&0x7));	\
		} else if (x86_is_imm8((disp))) {	\
			x86_address_byte ((inst), 1, (r), 4);	\
			x86_address_byte ((inst), (shift), (indexreg), ((basereg)&0x7));	\
			x86_imm_emit8 ((inst), (disp));	\
		} else {	\
			x86_address_byte ((inst), 2, (r), 4);	\
			x86_address_byte ((inst), (shift), (indexreg), 5);	\
			x86_imm_emit32 ((inst), (disp));	\
		}	\
	} while (0)

/*
 * target is the position in the code where to jump to:
 * target = code;
 * .. output loop code...
 * x86_mov_reg_imm (code, X86_EAX, 0);
 * loop = code;
 * x86_loop (code, -1);
 * ... finish method
 *
 * patch displacement
 * x86_patch (loop, target);
 *
 * ins should point at the start of the instruction that encodes a target.
 * the instruction is inspected for validity and the correct displacement
 * is inserted.
 */
#define x86_patch(ins,target)	\
	do {	\
		unsigned char* pos = (ins) + 1;	\
		int disp, size = 0;	\
		switch (*(unsigned char*)(ins)) {	\
		case 0xe8: case 0xe9: ++size; break; /* call, jump32 */	\
		case 0x0f: if (!(*pos >= 0x70 && *pos <= 0x8f)) assert (0);	\
		   ++size; ++pos; break; /* prefix for 32-bit disp */	\
		case 0xe0: case 0xe1: case 0xe2: /* loop */	\
		case 0xeb: /* jump8 */	\
		/* conditional jump opcodes */	\
		case 0x70: case 0x71: case 0x72: case 0x73:	\
		case 0x74: case 0x75: case 0x76: case 0x77:	\
		case 0x78: case 0x79: case 0x7a: case 0x7b:	\
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:	\
			break;	\
		default: assert (0);	\
		}	\
		disp = (target) - pos;	\
		if (size) x86_imm_emit32 (pos, disp - 4);	\
		else if (x86_is_imm8 (disp - 1)) x86_imm_emit8 (pos, disp - 1);	\
		else assert (0);	\
	} while (0)

#define x86_breakpoint(inst) \
	do {	\
		*(inst)++ = 0xcc;	\
	} while (0)

#define x86_cld(inst) do { *(inst)++ =(unsigned char)0xfc; } while (0)
#define x86_stosb(inst) do { *(inst)++ =(unsigned char)0xaa; } while (0)
#define x86_stosl(inst) do { *(inst)++ =(unsigned char)0xab; } while (0)
#define x86_stosd(inst) x86_stosl((inst))
#define x86_movsb(inst) do { *(inst)++ =(unsigned char)0xa4; } while (0)
#define x86_movsl(inst) do { *(inst)++ =(unsigned char)0xa5; } while (0)
#define x86_movsd(inst) x86_movsl((inst))

#define x86_prefix(inst,p) do { *(inst)++ =(unsigned char) (p); } while (0)

#define x86_rdtsc(inst) \
	do {	\
		*(inst)++ = 0x0f;	\
		*(inst)++ = 0x31;	\
	} while (0)

#define x86_cmpxchg_reg_reg(inst,dreg,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0xb1;	\
		x86_reg_emit ((inst), (reg), (dreg));	\
	} while (0)

#define x86_cmpxchg_mem_reg(inst,mem,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0xb1;	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)

#define x86_cmpxchg_membase_reg(inst,basereg,disp,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0xb1;	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_xchg_reg_reg(inst,dreg,reg,size)	\
	do {	\
		if ((size) == 1)	\
			*(inst)++ = (unsigned char)0x86;	\
		else	\
			*(inst)++ = (unsigned char)0x87;	\
		x86_reg_emit ((inst), (reg), (dreg));	\
	} while (0)

#define x86_xchg_mem_reg(inst,mem,reg,size)	\
	do {	\
		if ((size) == 1)	\
			*(inst)++ = (unsigned char)0x86;	\
		else	\
			*(inst)++ = (unsigned char)0x87;	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)

#define x86_xchg_membase_reg(inst,basereg,disp,reg,size)	\
	do {	\
		if ((size) == 1)	\
			*(inst)++ = (unsigned char)0x86;	\
		else	\
			*(inst)++ = (unsigned char)0x87;	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_xadd_reg_reg(inst,dreg,reg,size)	\
	do {	\
		*(inst)++ = (unsigned char)0x0F;     \
		if ((size) == 1)	\
			*(inst)++ = (unsigned char)0xC0;	\
		else	\
			*(inst)++ = (unsigned char)0xC1;	\
		x86_reg_emit ((inst), (reg), (dreg));	\
	} while (0)

#define x86_xadd_mem_reg(inst,mem,reg,size)	\
	do {	\
		*(inst)++ = (unsigned char)0x0F;     \
		if ((size) == 1)	\
			*(inst)++ = (unsigned char)0xC0;	\
		else	\
			*(inst)++ = (unsigned char)0xC1;	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)

#define x86_xadd_membase_reg(inst,basereg,disp,reg,size)	\
	do {	\
		*(inst)++ = (unsigned char)0x0F;     \
		if ((size) == 1)	\
			*(inst)++ = (unsigned char)0xC0;	\
		else	\
			*(inst)++ = (unsigned char)0xC1;	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_inc_mem(inst,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_mem_emit ((inst), 0, (mem)); 	\
	} while (0)

#define x86_inc_membase(inst,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_membase_emit ((inst), 0, (basereg), (disp));	\
	} while (0)

#define x86_inc_reg(inst,reg) do { *(inst)++ = (unsigned char)0x40 + (reg); } while (0)

#define x86_dec_mem(inst,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_mem_emit ((inst), 1, (mem));	\
	} while (0)

#define x86_dec_membase(inst,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_membase_emit ((inst), 1, (basereg), (disp));	\
	} while (0)

#define x86_dec_reg(inst,reg) do { *(inst)++ = (unsigned char)0x48 + (reg); } while (0)

#define x86_not_mem(inst,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_mem_emit ((inst), 2, (mem));	\
	} while (0)

#define x86_not_membase(inst,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_membase_emit ((inst), 2, (basereg), (disp));	\
	} while (0)

#define x86_not_reg(inst,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_reg_emit ((inst), 2, (reg));	\
	} while (0)

#define x86_neg_mem(inst,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_mem_emit ((inst), 3, (mem));	\
	} while (0)

#define x86_neg_membase(inst,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_membase_emit ((inst), 3, (basereg), (disp));	\
	} while (0)

#define x86_neg_reg(inst,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_reg_emit ((inst), 3, (reg));	\
	} while (0)

#define x86_nop(inst) do { *(inst)++ = (unsigned char)0x90; } while (0)

#define x86_alu_reg_imm(inst,opc,reg,imm) 	\
	do {	\
		if ((reg) == X86_EAX) {	\
			*(inst)++ = (((unsigned char)(opc)) << 3) + 5;	\
			x86_imm_emit32 ((inst), (imm));	\
			break;	\
		}	\
		if (x86_is_imm8((imm))) {	\
			*(inst)++ = (unsigned char)0x83;	\
			x86_reg_emit ((inst), (opc), (reg));	\
			x86_imm_emit8 ((inst), (imm));	\
		} else {	\
			*(inst)++ = (unsigned char)0x81;	\
			x86_reg_emit ((inst), (opc), (reg));	\
			x86_imm_emit32 ((inst), (imm));	\
		}	\
	} while (0)

#define x86_alu_mem_imm(inst,opc,mem,imm) 	\
	do {	\
		if (x86_is_imm8((imm))) {	\
			*(inst)++ = (unsigned char)0x83;	\
			x86_mem_emit ((inst), (opc), (mem));	\
			x86_imm_emit8 ((inst), (imm));	\
		} else {	\
			*(inst)++ = (unsigned char)0x81;	\
			x86_mem_emit ((inst), (opc), (mem));	\
			x86_imm_emit32 ((inst), (imm));	\
		}	\
	} while (0)

#define x86_alu_membase_imm(inst,opc,basereg,disp,imm) 	\
	do {	\
		if (x86_is_imm8((imm))) {	\
			*(inst)++ = (unsigned char)0x83;	\
			x86_membase_emit ((inst), (opc), (basereg), (disp));	\
			x86_imm_emit8 ((inst), (imm));	\
		} else {	\
			*(inst)++ = (unsigned char)0x81;	\
			x86_membase_emit ((inst), (opc), (basereg), (disp));	\
			x86_imm_emit32 ((inst), (imm));	\
		}	\
	} while (0)

#define x86_alu_membase8_imm(inst,opc,basereg,disp,imm) 	\
	do {	\
		*(inst)++ = (unsigned char)0x80;	\
		x86_membase_emit ((inst), (opc), (basereg), (disp));	\
		x86_imm_emit8 ((inst), (imm)); \
	} while (0)

#define x86_alu_mem_reg(inst,opc,mem,reg)	\
	do {	\
		*(inst)++ = (((unsigned char)(opc)) << 3) + 1;	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)

#define x86_alu_membase_reg(inst,opc,basereg,disp,reg)	\
	do {	\
		*(inst)++ = (((unsigned char)(opc)) << 3) + 1;	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_alu_reg_reg(inst,opc,dreg,reg)	\
	do {	\
		*(inst)++ = (((unsigned char)(opc)) << 3) + 3;	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

/**
 * @x86_alu_reg8_reg8:
 * Supports ALU operations between two 8-bit registers.
 * dreg := dreg opc reg
 * X86_Reg_No enum is used to specify the registers.
 * Additionally is_*_h flags are used to specify what part
 * of a given 32-bit register is used - high (TRUE) or low (FALSE).
 * For example: dreg = X86_EAX, is_dreg_h = TRUE -> use AH
 */
#define x86_alu_reg8_reg8(inst,opc,dreg,reg,is_dreg_h,is_reg_h)	\
	do {	\
		*(inst)++ = (((unsigned char)(opc)) << 3) + 2;	\
		x86_reg8_emit ((inst), (dreg), (reg), (is_dreg_h), (is_reg_h));	\
	} while (0)

#define x86_alu_reg_mem(inst,opc,reg,mem)	\
	do {	\
		*(inst)++ = (((unsigned char)(opc)) << 3) + 3;	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)

#define x86_alu_reg_membase(inst,opc,reg,basereg,disp)	\
	do {	\
		*(inst)++ = (((unsigned char)(opc)) << 3) + 3;	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_test_reg_imm(inst,reg,imm)	\
	do {	\
		if ((reg) == X86_EAX) {	\
			*(inst)++ = (unsigned char)0xa9;	\
		} else {	\
			*(inst)++ = (unsigned char)0xf7;	\
			x86_reg_emit ((inst), 0, (reg));	\
		}	\
		x86_imm_emit32 ((inst), (imm));	\
	} while (0)

#define x86_test_mem_imm(inst,mem,imm)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_mem_emit ((inst), 0, (mem));	\
		x86_imm_emit32 ((inst), (imm));	\
	} while (0)

#define x86_test_membase_imm(inst,basereg,disp,imm)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_membase_emit ((inst), 0, (basereg), (disp));	\
		x86_imm_emit32 ((inst), (imm));	\
	} while (0)

#define x86_test_reg_reg(inst,dreg,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x85;	\
		x86_reg_emit ((inst), (reg), (dreg));	\
	} while (0)

#define x86_test_mem_reg(inst,mem,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x85;	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)

#define x86_test_membase_reg(inst,basereg,disp,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x85;	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_shift_reg_imm(inst,opc,reg,imm)	\
	do {	\
		if ((imm) == 1) {	\
			*(inst)++ = (unsigned char)0xd1;	\
			x86_reg_emit ((inst), (opc), (reg));	\
		} else {	\
			*(inst)++ = (unsigned char)0xc1;	\
			x86_reg_emit ((inst), (opc), (reg));	\
			x86_imm_emit8 ((inst), (imm));	\
		}	\
	} while (0)

#define x86_shift_mem_imm(inst,opc,mem,imm)	\
	do {	\
		if ((imm) == 1) {	\
			*(inst)++ = (unsigned char)0xd1;	\
			x86_mem_emit ((inst), (opc), (mem));	\
		} else {	\
			*(inst)++ = (unsigned char)0xc1;	\
			x86_mem_emit ((inst), (opc), (mem));	\
			x86_imm_emit8 ((inst), (imm));	\
		}	\
	} while (0)

#define x86_shift_membase_imm(inst,opc,basereg,disp,imm)	\
	do {	\
		if ((imm) == 1) {	\
			*(inst)++ = (unsigned char)0xd1;	\
			x86_membase_emit ((inst), (opc), (basereg), (disp));	\
		} else {	\
			*(inst)++ = (unsigned char)0xc1;	\
			x86_membase_emit ((inst), (opc), (basereg), (disp));	\
			x86_imm_emit8 ((inst), (imm));	\
		}	\
	} while (0)

#define x86_shift_reg(inst,opc,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0xd3;	\
		x86_reg_emit ((inst), (opc), (reg));	\
	} while (0)

#define x86_shift_mem(inst,opc,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xd3;	\
		x86_mem_emit ((inst), (opc), (mem));	\
	} while (0)

#define x86_shift_membase(inst,opc,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xd3;	\
		x86_membase_emit ((inst), (opc), (basereg), (disp));	\
	} while (0)

/*
 * Cmp ops
 */
#define x86_cmp_reg_imm(inst,reg,imm)	\
	do {	\
		if ((reg) == X86_EAX) {	\
			*(inst)++ = (unsigned char)0x3d;	\
		} else {	\
			*(inst)++ = (unsigned char)0x81;	\
			x86_reg_emit ((inst), 7, (reg));	\
		}	\
		x86_imm_emit32 ((inst), (imm));	\
	} while (0)

#define x86_cmp_reg_reg(inst,dreg,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x39;	\
		x86_reg_emit ((inst), (reg), (dreg));	\
	} while (0)


/*
 * Multi op shift missing.
 */

#define x86_shrd_reg(inst,dreg,reg)                     \
        do {                                            \
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0xad;	\
		x86_reg_emit ((inst), (reg), (dreg));	\
	} while (0)

#define x86_shrd_reg_imm(inst,dreg,reg,shamt)           \
        do {                                            \
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0xac;	\
		x86_reg_emit ((inst), (reg), (dreg));	\
		x86_imm_emit8 ((inst), (shamt));	\
	} while (0)

#define x86_shld_reg(inst,dreg,reg)                     \
        do {                                            \
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0xa5;	\
		x86_reg_emit ((inst), (reg), (dreg));	\
	} while (0)

#define x86_shld_reg_imm(inst,dreg,reg,shamt)           \
        do {                                            \
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0xa4;	\
		x86_reg_emit ((inst), (reg), (dreg));	\
		x86_imm_emit8 ((inst), (shamt));	\
	} while (0)

/*
 * EDX:EAX = EAX * rm
 */
#define x86_mul_reg(inst,reg,is_signed)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_reg_emit ((inst), 4 + ((is_signed) ? 1 : 0), (reg));	\
	} while (0)

#define x86_mul_mem(inst,mem,is_signed)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_mem_emit ((inst), 4 + ((is_signed) ? 1 : 0), (mem));	\
	} while (0)

#define x86_mul_membase(inst,basereg,disp,is_signed)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_membase_emit ((inst), 4 + ((is_signed) ? 1 : 0), (basereg), (disp));	\
	} while (0)

/*
 * r *= rm
 */
#define x86_imul_reg_reg(inst,dreg,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0xaf;	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

#define x86_imul_reg_mem(inst,reg,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0xaf;	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)

#define x86_imul_reg_membase(inst,reg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0xaf;	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

/*
 * dreg = rm * imm
 */
#define x86_imul_reg_reg_imm(inst,dreg,reg,imm)	\
	do {	\
		if (x86_is_imm8 ((imm))) {	\
			*(inst)++ = (unsigned char)0x6b;	\
			x86_reg_emit ((inst), (dreg), (reg));	\
			x86_imm_emit8 ((inst), (imm));	\
		} else {	\
			*(inst)++ = (unsigned char)0x69;	\
			x86_reg_emit ((inst), (dreg), (reg));	\
			x86_imm_emit32 ((inst), (imm));	\
		}	\
	} while (0)

#define x86_imul_reg_mem_imm(inst,reg,mem,imm)	\
	do {	\
		if (x86_is_imm8 ((imm))) {	\
			*(inst)++ = (unsigned char)0x6b;	\
			x86_mem_emit ((inst), (reg), (mem));	\
			x86_imm_emit8 ((inst), (imm));	\
		} else {	\
			*(inst)++ = (unsigned char)0x69;	\
			x86_reg_emit ((inst), (reg), (mem));	\
			x86_imm_emit32 ((inst), (imm));	\
		}	\
	} while (0)

#define x86_imul_reg_membase_imm(inst,reg,basereg,disp,imm)	\
	do {	\
		if (x86_is_imm8 ((imm))) {	\
			*(inst)++ = (unsigned char)0x6b;	\
			x86_membase_emit ((inst), (reg), (basereg), (disp));	\
			x86_imm_emit8 ((inst), (imm));	\
		} else {	\
			*(inst)++ = (unsigned char)0x69;	\
			x86_membase_emit ((inst), (reg), (basereg), (disp));	\
			x86_imm_emit32 ((inst), (imm));	\
		}	\
	} while (0)

/*
 * divide EDX:EAX by rm;
 * eax = quotient, edx = remainder
 */

#define x86_div_reg(inst,reg,is_signed)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_reg_emit ((inst), 6 + ((is_signed) ? 1 : 0), (reg));	\
	} while (0)

#define x86_div_mem(inst,mem,is_signed)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_mem_emit ((inst), 6 + ((is_signed) ? 1 : 0), (mem));	\
	} while (0)

#define x86_div_membase(inst,basereg,disp,is_signed)	\
	do {	\
		*(inst)++ = (unsigned char)0xf7;	\
		x86_membase_emit ((inst), 6 + ((is_signed) ? 1 : 0), (basereg), (disp));	\
	} while (0)

#define x86_mov_mem_reg(inst,mem,reg,size)	\
	do {	\
		switch ((size)) {	\
		case 1: *(inst)++ = (unsigned char)0x88; break;	\
		case 2: *(inst)++ = (unsigned char)0x66; /* fall through */	\
		case 4: *(inst)++ = (unsigned char)0x89; break;	\
		default: assert (0);	\
		}	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)

#define x86_mov_regp_reg(inst,regp,reg,size)	\
	do {	\
		switch ((size)) {	\
		case 1: *(inst)++ = (unsigned char)0x88; break;	\
		case 2: *(inst)++ = (unsigned char)0x66; /* fall through */	\
		case 4: *(inst)++ = (unsigned char)0x89; break;	\
		default: assert (0);	\
		}	\
		x86_regp_emit ((inst), (reg), (regp));	\
	} while (0)

#define x86_mov_membase_reg(inst,basereg,disp,reg,size)	\
	do {	\
		switch ((size)) {	\
		case 1: *(inst)++ = (unsigned char)0x88; break;	\
		case 2: *(inst)++ = (unsigned char)0x66; /* fall through */	\
		case 4: *(inst)++ = (unsigned char)0x89; break;	\
		default: assert (0);	\
		}	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_mov_memindex_reg(inst,basereg,disp,indexreg,shift,reg,size)	\
	do {	\
		switch ((size)) {	\
		case 1: *(inst)++ = (unsigned char)0x88; break;	\
		case 2: *(inst)++ = (unsigned char)0x66; /* fall through */	\
		case 4: *(inst)++ = (unsigned char)0x89; break;	\
		default: assert (0);	\
		}	\
		x86_memindex_emit ((inst), (reg), (basereg), (disp), (indexreg), (shift));	\
	} while (0)

#define x86_mov_reg_reg(inst,dreg,reg,size)	\
	do {	\
		switch ((size)) {	\
		case 1: *(inst)++ = (unsigned char)0x8a; break;	\
		case 2: *(inst)++ = (unsigned char)0x66; /* fall through */	\
		case 4: *(inst)++ = (unsigned char)0x8b; break;	\
		default: assert (0);	\
		}	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

#define x86_mov_reg_mem(inst,reg,mem,size)	\
	do {	\
		switch ((size)) {	\
		case 1: *(inst)++ = (unsigned char)0x8a; break;	\
		case 2: *(inst)++ = (unsigned char)0x66; /* fall through */	\
		case 4: *(inst)++ = (unsigned char)0x8b; break;	\
		default: assert (0);	\
		}	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)

#define x86_mov_reg_membase(inst,reg,basereg,disp,size)	\
	do {	\
		switch ((size)) {	\
		case 1: *(inst)++ = (unsigned char)0x8a; break;	\
		case 2: *(inst)++ = (unsigned char)0x66; /* fall through */	\
		case 4: *(inst)++ = (unsigned char)0x8b; break;	\
		default: assert (0);	\
		}	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_mov_reg_memindex(inst,reg,basereg,disp,indexreg,shift,size)	\
	do {	\
		switch ((size)) {	\
		case 1: *(inst)++ = (unsigned char)0x8a; break;	\
		case 2: *(inst)++ = (unsigned char)0x66; /* fall through */	\
		case 4: *(inst)++ = (unsigned char)0x8b; break;	\
		default: assert (0);	\
		}	\
		x86_memindex_emit ((inst), (reg), (basereg), (disp), (indexreg), (shift));	\
	} while (0)

/*
 * Note: x86_clear_reg () chacnges the condition code!
 */
#define x86_clear_reg(inst,reg) x86_alu_reg_reg((inst), X86_XOR, (reg), (reg))

#define x86_mov_reg_imm(inst,reg,imm)	\
	do {	\
		*(inst)++ = (unsigned char)0xb8 + (reg);	\
		x86_imm_emit32 ((inst), (imm));	\
	} while (0)

#define x86_mov_mem_imm(inst,mem,imm,size)	\
	do {	\
		if ((size) == 1) {	\
			*(inst)++ = (unsigned char)0xc6;	\
			x86_mem_emit ((inst), 0, (mem));	\
			x86_imm_emit8 ((inst), (imm));	\
		} else if ((size) == 2) {	\
			*(inst)++ = (unsigned char)0x66;	\
			*(inst)++ = (unsigned char)0xc7;	\
			x86_mem_emit ((inst), 0, (mem));	\
			x86_imm_emit16 ((inst), (imm));	\
		} else {	\
			*(inst)++ = (unsigned char)0xc7;	\
			x86_mem_emit ((inst), 0, (mem));	\
			x86_imm_emit32 ((inst), (imm));	\
		}	\
	} while (0)

#define x86_mov_membase_imm(inst,basereg,disp,imm,size)	\
	do {	\
		if ((size) == 1) {	\
			*(inst)++ = (unsigned char)0xc6;	\
			x86_membase_emit ((inst), 0, (basereg), (disp));	\
			x86_imm_emit8 ((inst), (imm));	\
		} else if ((size) == 2) {	\
			*(inst)++ = (unsigned char)0x66;	\
			*(inst)++ = (unsigned char)0xc7;	\
			x86_membase_emit ((inst), 0, (basereg), (disp));	\
			x86_imm_emit16 ((inst), (imm));	\
		} else {	\
			*(inst)++ = (unsigned char)0xc7;	\
			x86_membase_emit ((inst), 0, (basereg), (disp));	\
			x86_imm_emit32 ((inst), (imm));	\
		}	\
	} while (0)

#define x86_mov_memindex_imm(inst,basereg,disp,indexreg,shift,imm,size)	\
	do {	\
		if ((size) == 1) {	\
			*(inst)++ = (unsigned char)0xc6;	\
			x86_memindex_emit ((inst), 0, (basereg), (disp), (indexreg), (shift));	\
			x86_imm_emit8 ((inst), (imm));	\
		} else if ((size) == 2) {	\
			*(inst)++ = (unsigned char)0x66;	\
			*(inst)++ = (unsigned char)0xc7;	\
			x86_memindex_emit ((inst), 0, (basereg), (disp), (indexreg), (shift));	\
			x86_imm_emit16 ((inst), (imm));	\
		} else {	\
			*(inst)++ = (unsigned char)0xc7;	\
			x86_memindex_emit ((inst), 0, (basereg), (disp), (indexreg), (shift));	\
			x86_imm_emit32 ((inst), (imm));	\
		}	\
	} while (0)

#define x86_lea_mem(inst,reg,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0x8d;	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)

#define x86_lea_membase(inst,reg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0x8d;	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_lea_memindex(inst,reg,basereg,disp,indexreg,shift)	\
	do {	\
		*(inst)++ = (unsigned char)0x8d;	\
		x86_memindex_emit ((inst), (reg), (basereg), (disp), (indexreg), (shift));	\
	} while (0)

#define x86_widen_reg(inst,dreg,reg,is_signed,is_half)	\
	do {	\
		unsigned char op = 0xb6;	\
                g_assert (is_half ||  X86_IS_BYTE_REG (reg)); \
		*(inst)++ = (unsigned char)0x0f;	\
		if ((is_signed)) op += 0x08;	\
		if ((is_half)) op += 0x01;	\
		*(inst)++ = op;	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

#define x86_widen_mem(inst,dreg,mem,is_signed,is_half)	\
	do {	\
		unsigned char op = 0xb6;	\
		*(inst)++ = (unsigned char)0x0f;	\
		if ((is_signed)) op += 0x08;	\
		if ((is_half)) op += 0x01;	\
		*(inst)++ = op;	\
		x86_mem_emit ((inst), (dreg), (mem));	\
	} while (0)

#define x86_widen_membase(inst,dreg,basereg,disp,is_signed,is_half)	\
	do {	\
		unsigned char op = 0xb6;	\
		*(inst)++ = (unsigned char)0x0f;	\
		if ((is_signed)) op += 0x08;	\
		if ((is_half)) op += 0x01;	\
		*(inst)++ = op;	\
		x86_membase_emit ((inst), (dreg), (basereg), (disp));	\
	} while (0)

#define x86_widen_memindex(inst,dreg,basereg,disp,indexreg,shift,is_signed,is_half)	\
	do {	\
		unsigned char op = 0xb6;	\
		*(inst)++ = (unsigned char)0x0f;	\
		if ((is_signed)) op += 0x08;	\
		if ((is_half)) op += 0x01;	\
		*(inst)++ = op;	\
		x86_memindex_emit ((inst), (dreg), (basereg), (disp), (indexreg), (shift));	\
	} while (0)

#define x86_cdq(inst)  do { *(inst)++ = (unsigned char)0x99; } while (0)
#define x86_wait(inst) do { *(inst)++ = (unsigned char)0x9b; } while (0)

#define x86_fp_op_mem(inst,opc,mem,is_double)	\
	do {	\
		*(inst)++ = (is_double) ? (unsigned char)0xdc : (unsigned char)0xd8;	\
		x86_mem_emit ((inst), (opc), (mem));	\
	} while (0)

#define x86_fp_op_membase(inst,opc,basereg,disp,is_double)	\
	do {	\
		*(inst)++ = (is_double) ? (unsigned char)0xdc : (unsigned char)0xd8;	\
		x86_membase_emit ((inst), (opc), (basereg), (disp));	\
	} while (0)

#define x86_fp_op(inst,opc,index)	\
	do {	\
		*(inst)++ = (unsigned char)0xd8;	\
		*(inst)++ = (unsigned char)0xc0+((opc)<<3)+((index)&0x07);	\
	} while (0)

#define x86_fp_op_reg(inst,opc,index,pop_stack)	\
	do {	\
		static const unsigned char map[] = { 0, 1, 2, 3, 5, 4, 7, 6, 8};	\
		*(inst)++ = (pop_stack) ? (unsigned char)0xde : (unsigned char)0xdc;	\
		*(inst)++ = (unsigned char)0xc0+(map[(opc)]<<3)+((index)&0x07);	\
	} while (0)

/**
 * @x86_fp_int_op_membase
 * Supports FPU operations between ST(0) and integer operand in memory.
 * Operation encoded using X86_FP_Opcode enum.
 * Operand is addressed by [basereg + disp].
 * is_int specifies whether operand is int32 (TRUE) or int16 (FALSE).
 */
#define x86_fp_int_op_membase(inst,opc,basereg,disp,is_int)	\
	do {	\
		*(inst)++ = (is_int) ? (unsigned char)0xda : (unsigned char)0xde;	\
		x86_membase_emit ((inst), opc, (basereg), (disp));	\
	} while (0)

#define x86_fstp(inst,index)	\
	do {	\
		*(inst)++ = (unsigned char)0xdd;	\
		*(inst)++ = (unsigned char)0xd8+(index);	\
	} while (0)

#define x86_fcompp(inst)	\
	do {	\
		*(inst)++ = (unsigned char)0xde;	\
		*(inst)++ = (unsigned char)0xd9;	\
	} while (0)

#define x86_fucompp(inst)	\
	do {	\
		*(inst)++ = (unsigned char)0xda;	\
		*(inst)++ = (unsigned char)0xe9;	\
	} while (0)

#define x86_fnstsw(inst)	\
	do {	\
		*(inst)++ = (unsigned char)0xdf;	\
		*(inst)++ = (unsigned char)0xe0;	\
	} while (0)

#define x86_fnstcw(inst,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xd9;	\
		x86_mem_emit ((inst), 7, (mem));	\
	} while (0)

#define x86_fnstcw_membase(inst,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xd9;	\
		x86_membase_emit ((inst), 7, (basereg), (disp));	\
	} while (0)

#define x86_fldcw(inst,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xd9;	\
		x86_mem_emit ((inst), 5, (mem));	\
	} while (0)

#define x86_fldcw_membase(inst,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xd9;	\
		x86_membase_emit ((inst), 5, (basereg), (disp));	\
	} while (0)

#define x86_fchs(inst)	\
	do {	\
		*(inst)++ = (unsigned char)0xd9;	\
		*(inst)++ = (unsigned char)0xe0;	\
	} while (0)

#define x86_frem(inst)	\
	do {	\
		*(inst)++ = (unsigned char)0xd9;	\
		*(inst)++ = (unsigned char)0xf8;	\
	} while (0)

#define x86_fxch(inst,index)	\
	do {	\
		*(inst)++ = (unsigned char)0xd9;	\
		*(inst)++ = (unsigned char)0xc8 + ((index) & 0x07);	\
	} while (0)

#define x86_fcomi(inst,index)	\
	do {	\
		*(inst)++ = (unsigned char)0xdb;	\
		*(inst)++ = (unsigned char)0xf0 + ((index) & 0x07);	\
	} while (0)

#define x86_fcomip(inst,index)	\
	do {	\
		*(inst)++ = (unsigned char)0xdf;	\
		*(inst)++ = (unsigned char)0xf0 + ((index) & 0x07);	\
	} while (0)

#define x86_fucomi(inst,index)	\
	do {	\
		*(inst)++ = (unsigned char)0xdb;	\
		*(inst)++ = (unsigned char)0xe8 + ((index) & 0x07);	\
	} while (0)

#define x86_fucomip(inst,index)	\
	do {	\
		*(inst)++ = (unsigned char)0xdf;	\
		*(inst)++ = (unsigned char)0xe8 + ((index) & 0x07);	\
	} while (0)

#define x86_fld(inst,mem,is_double)	\
	do {	\
		*(inst)++ = (is_double) ? (unsigned char)0xdd : (unsigned char)0xd9;	\
		x86_mem_emit ((inst), 0, (mem));	\
	} while (0)

#define x86_fld_membase(inst,basereg,disp,is_double)	\
	do {	\
		*(inst)++ = (is_double) ? (unsigned char)0xdd : (unsigned char)0xd9;	\
		x86_membase_emit ((inst), 0, (basereg), (disp));	\
	} while (0)

#define x86_fld80_mem(inst,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xdb;	\
		x86_mem_emit ((inst), 5, (mem));	\
	} while (0)

#define x86_fld80_membase(inst,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xdb;	\
		x86_membase_emit ((inst), 5, (basereg), (disp));	\
	} while (0)

#define x86_fild(inst,mem,is_int64_t)	\
	do {	\
		if ((is_int64_t)) {	\
			*(inst)++ = (unsigned char)0xdf;	\
			x86_mem_emit ((inst), 5, (mem));	\
		} else {	\
			*(inst)++ = (unsigned char)0xdb;	\
			x86_mem_emit ((inst), 0, (mem));	\
		}	\
	} while (0)

#define x86_fild_membase(inst,basereg,disp,is_int64_t)	\
	do {	\
		if ((is_int64_t)) {	\
			*(inst)++ = (unsigned char)0xdf;	\
			x86_membase_emit ((inst), 5, (basereg), (disp));	\
		} else {	\
			*(inst)++ = (unsigned char)0xdb;	\
			x86_membase_emit ((inst), 0, (basereg), (disp));	\
		}	\
	} while (0)

#define x86_fld_reg(inst,index)	\
	do {	\
		*(inst)++ = (unsigned char)0xd9;	\
		*(inst)++ = (unsigned char)0xc0 + ((index) & 0x07);	\
	} while (0)

#define x86_fldz(inst)	\
	do {	\
		*(inst)++ = (unsigned char)0xd9;	\
		*(inst)++ = (unsigned char)0xee;	\
	} while (0)

#define x86_fld1(inst)	\
	do {	\
		*(inst)++ = (unsigned char)0xd9;	\
		*(inst)++ = (unsigned char)0xe8;	\
	} while (0)

#define x86_fldpi(inst)	\
	do {	\
		*(inst)++ = (unsigned char)0xd9;	\
		*(inst)++ = (unsigned char)0xeb;	\
	} while (0)

#define x86_fst(inst,mem,is_double,pop_stack)	\
	do {	\
		*(inst)++ = (is_double) ? (unsigned char)0xdd: (unsigned char)0xd9;	\
		x86_mem_emit ((inst), 2 + ((pop_stack) ? 1 : 0), (mem));	\
	} while (0)

#define x86_fst_membase(inst,basereg,disp,is_double,pop_stack)	\
	do {	\
		*(inst)++ = (is_double) ? (unsigned char)0xdd: (unsigned char)0xd9;	\
		x86_membase_emit ((inst), 2 + ((pop_stack) ? 1 : 0), (basereg), (disp));	\
	} while (0)

#define x86_fst80_mem(inst,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xdb;	\
		x86_mem_emit ((inst), 7, (mem));	\
	} while (0)


#define x86_fst80_membase(inst,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xdb;	\
		x86_membase_emit ((inst), 7, (basereg), (disp));	\
	} while (0)


#define x86_fist_pop(inst,mem,is_int64_t)	\
	do {	\
		if ((is_int64_t)) {	\
			*(inst)++ = (unsigned char)0xdf;	\
			x86_mem_emit ((inst), 7, (mem));	\
		} else {	\
			*(inst)++ = (unsigned char)0xdb;	\
			x86_mem_emit ((inst), 3, (mem));	\
		}	\
	} while (0)

#define x86_fist_pop_membase(inst,basereg,disp,is_int64_t)	\
	do {	\
		if ((is_int64_t)) {	\
			*(inst)++ = (unsigned char)0xdf;	\
			x86_membase_emit ((inst), 7, (basereg), (disp));	\
		} else {	\
			*(inst)++ = (unsigned char)0xdb;	\
			x86_membase_emit ((inst), 3, (basereg), (disp));	\
		}	\
	} while (0)

#define x86_fstsw(inst)	\
	do {	\
			*(inst)++ = (unsigned char)0x9b;	\
			*(inst)++ = (unsigned char)0xdf;	\
			*(inst)++ = (unsigned char)0xe0;	\
	} while (0)

/**
 * @x86_fist_membase
 * Converts content of ST(0) to integer and stores it at memory location
 * addressed by [basereg + disp].
 * is_int specifies whether destination is int32 (TRUE) or int16 (FALSE).
 */
#define x86_fist_membase(inst,basereg,disp,is_int)	\
	do {	\
		if ((is_int)) {	\
			*(inst)++ = (unsigned char)0xdb;	\
			x86_membase_emit ((inst), 2, (basereg), (disp));	\
		} else {	\
			*(inst)++ = (unsigned char)0xdf;	\
			x86_membase_emit ((inst), 2, (basereg), (disp));	\
		}	\
	} while (0)


#define x86_push_reg(inst,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x50 + (reg);	\
	} while (0)

#define x86_push_regp(inst,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_regp_emit ((inst), 6, (reg));	\
	} while (0)

#define x86_push_mem(inst,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_mem_emit ((inst), 6, (mem));	\
	} while (0)

#define x86_push_membase(inst,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_membase_emit ((inst), 6, (basereg), (disp));	\
	} while (0)

#define x86_push_memindex(inst,basereg,disp,indexreg,shift)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_memindex_emit ((inst), 6, (basereg), (disp), (indexreg), (shift));	\
	} while (0)

#define x86_push_imm_template(inst) x86_push_imm (inst, 0xf0f0f0f0)

#define x86_push_imm(inst,imm)	\
	do {	\
		int _imm = (int) (imm);	\
		if (x86_is_imm8 (_imm)) {	\
			*(inst)++ = (unsigned char)0x6A;	\
			x86_imm_emit8 ((inst), (_imm));	\
		} else {	\
			*(inst)++ = (unsigned char)0x68;	\
			x86_imm_emit32 ((inst), (_imm));	\
		}	\
	} while (0)

#define x86_pop_reg(inst,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x58 + (reg);	\
	} while (0)

#define x86_pop_mem(inst,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0x8f;	\
		x86_mem_emit ((inst), 0, (mem));	\
	} while (0)

#define x86_pop_membase(inst,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0x87;	\
		x86_membase_emit ((inst), 0, (basereg), (disp));	\
	} while (0)

#define x86_pushad(inst) do { *(inst)++ = (unsigned char)0x60; } while (0)
#define x86_pushfd(inst) do { *(inst)++ = (unsigned char)0x9c; } while (0)
#define x86_popad(inst)  do { *(inst)++ = (unsigned char)0x61; } while (0)
#define x86_popfd(inst)  do { *(inst)++ = (unsigned char)0x9d; } while (0)

#define x86_loop(inst,imm)	\
	do {	\
		*(inst)++ = (unsigned char)0xe2;	\
		x86_imm_emit8 ((inst), (imm));	\
	} while (0)

#define x86_loope(inst,imm)	\
	do {	\
		*(inst)++ = (unsigned char)0xe1;	\
		x86_imm_emit8 ((inst), (imm));	\
	} while (0)

#define x86_loopne(inst,imm)	\
	do {	\
		*(inst)++ = (unsigned char)0xe0;	\
		x86_imm_emit8 ((inst), (imm));	\
	} while (0)

#define x86_jump32(inst,imm)	\
	do {	\
		*(inst)++ = (unsigned char)0xe9;	\
		x86_imm_emit32 ((inst), (imm));	\
	} while (0)

#define x86_jump8(inst,imm)	\
	do {	\
		*(inst)++ = (unsigned char)0xeb;	\
		x86_imm_emit8 ((inst), (imm));	\
	} while (0)

#define x86_jump_reg(inst,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_reg_emit ((inst), 4, (reg));	\
	} while (0)

#define x86_jump_mem(inst,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_mem_emit ((inst), 4, (mem));	\
	} while (0)

#define x86_jump_membase(inst,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_membase_emit ((inst), 4, (basereg), (disp));	\
	} while (0)

/*
 * target is a pointer in our buffer.
 */
#define x86_jump_code(inst,target)	\
	do {	\
		int t = (unsigned char*)(target) - (inst) - 2;	\
		if (x86_is_imm8(t)) {	\
			x86_jump8 ((inst), t);	\
		} else {	\
			t -= 3;	\
			x86_jump32 ((inst), t);	\
		}	\
	} while (0)

#define x86_jump_disp(inst,disp)	\
	do {	\
		int t = (disp) - 2;	\
		if (x86_is_imm8(t)) {	\
			x86_jump8 ((inst), t);	\
		} else {	\
			t -= 3;	\
			x86_jump32 ((inst), t);	\
		}	\
	} while (0)

#define x86_jump_disp32(inst,disp)	\
	do {	\
		int t = (disp) - 5;	\
		x86_jump32 ((inst), t);	\
	} while (0)


#define x86_branch8(inst,cond,imm,is_signed)	\
	do {	\
		if ((is_signed))	\
			*(inst)++ = x86_cc_signed_map [(cond)];	\
		else	\
			*(inst)++ = x86_cc_unsigned_map [(cond)];	\
		x86_imm_emit8 ((inst), (imm));	\
	} while (0)

#define x86_branch32(inst,cond,imm,is_signed)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		if ((is_signed))	\
			*(inst)++ = x86_cc_signed_map [(cond)] + 0x10;	\
		else	\
			*(inst)++ = x86_cc_unsigned_map [(cond)] + 0x10;	\
		x86_imm_emit32 ((inst), (imm));	\
	} while (0)

#define x86_branch(inst,cond,target,is_signed)	\
	do {	\
		int offset = (target) - (inst) - 2;	\
		if (x86_is_imm8 ((offset)))	\
			x86_branch8 ((inst), (cond), offset, (is_signed));	\
		else {	\
			offset -= 4;	\
			x86_branch32 ((inst), (cond), offset, (is_signed));	\
		}	\
	} while (0)

#define x86_branch_disp(inst,cond,disp,is_signed)	\
	do {	\
		int offset = (disp) - 2;	\
		if (x86_is_imm8 ((offset)))	\
			x86_branch8 ((inst), (cond), offset, (is_signed));	\
		else {	\
			offset -= 4;	\
			x86_branch32 ((inst), (cond), offset, (is_signed));	\
		}	\
	} while (0)

#define x86_branch_disp32(inst,cond,disp,is_signed)	\
	do {	\
		int offset = (disp) - 6;	\
		x86_branch32 ((inst), (cond), offset, (is_signed));	\
	} while (0)



#define x86_set_reg(inst,cond,reg,is_signed)	\
	do {	\
                g_assert (X86_IS_BYTE_REG (reg)); \
		*(inst)++ = (unsigned char)0x0f;	\
		if ((is_signed))	\
			*(inst)++ = x86_cc_signed_map [(cond)] + 0x20;	\
		else	\
			*(inst)++ = x86_cc_unsigned_map [(cond)] + 0x20;	\
		x86_reg_emit ((inst), 0, (reg));	\
	} while (0)

#define x86_set_mem(inst,cond,mem,is_signed)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		if ((is_signed))	\
			*(inst)++ = x86_cc_signed_map [(cond)] + 0x20;	\
		else	\
			*(inst)++ = x86_cc_unsigned_map [(cond)] + 0x20;	\
		x86_mem_emit ((inst), 0, (mem));	\
	} while (0)

#define x86_set_membase(inst,cond,basereg,disp,is_signed)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		if ((is_signed))	\
			*(inst)++ = x86_cc_signed_map [(cond)] + 0x20;	\
		else	\
			*(inst)++ = x86_cc_unsigned_map [(cond)] + 0x20;	\
		x86_membase_emit ((inst), 0, (basereg), (disp));	\
	} while (0)

#define x86_call_imm(inst,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xe8;	\
		x86_imm_emit32 ((inst), (int)(disp));	\
	} while (0)

#define x86_call_reg(inst,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_reg_emit ((inst), 2, (reg));	\
	} while (0)

#define x86_call_mem(inst,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_mem_emit ((inst), 2, (mem));	\
	} while (0)

#define x86_call_membase(inst,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xff;	\
		x86_membase_emit ((inst), 2, (basereg), (disp));	\
	} while (0)

#define x86_call_code(inst,target)	\
	do {	\
		int _x86_offset = (unsigned char*)(target) - (inst);	\
		_x86_offset -= 5;	\
		x86_call_imm ((inst), _x86_offset);	\
	} while (0)

#define x86_ret(inst) do { *(inst)++ = (unsigned char)0xc3; } while (0)

#define x86_ret_imm(inst,imm)	\
	do {	\
		if ((imm) == 0) {	\
			x86_ret ((inst));	\
		} else {	\
			*(inst)++ = (unsigned char)0xc2;	\
			x86_imm_emit16 ((inst), (imm));	\
		}	\
	} while (0)

#define x86_cmov_reg(inst,cond,is_signed,dreg,reg)	\
	do {	\
		*(inst)++ = (unsigned char) 0x0f;	\
		if ((is_signed))	\
			*(inst)++ = x86_cc_signed_map [(cond)] - 0x30;	\
		else	\
			*(inst)++ = x86_cc_unsigned_map [(cond)] - 0x30;	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

#define x86_cmov_mem(inst,cond,is_signed,reg,mem)	\
	do {	\
		*(inst)++ = (unsigned char) 0x0f;	\
		if ((is_signed))	\
			*(inst)++ = x86_cc_signed_map [(cond)] - 0x30;	\
		else	\
			*(inst)++ = x86_cc_unsigned_map [(cond)] - 0x30;	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)

#define x86_cmov_membase(inst,cond,is_signed,reg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char) 0x0f;	\
		if ((is_signed))	\
			*(inst)++ = x86_cc_signed_map [(cond)] - 0x30;	\
		else	\
			*(inst)++ = x86_cc_unsigned_map [(cond)] - 0x30;	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_enter(inst,framesize)	\
	do {	\
		*(inst)++ = (unsigned char)0xc8;	\
		x86_imm_emit16 ((inst), (framesize));	\
		*(inst)++ = 0;	\
	} while (0)

#define x86_leave(inst) do { *(inst)++ = (unsigned char)0xc9; } while (0)
#define x86_sahf(inst)  do { *(inst)++ = (unsigned char)0x9e; } while (0)

#define x86_fsin(inst) do { *(inst)++ = (unsigned char)0xd9; *(inst)++ = (unsigned char)0xfe; } while (0)
#define x86_fcos(inst) do { *(inst)++ = (unsigned char)0xd9; *(inst)++ = (unsigned char)0xff; } while (0)
#define x86_fabs(inst) do { *(inst)++ = (unsigned char)0xd9; *(inst)++ = (unsigned char)0xe1; } while (0)
#define x86_ftst(inst) do { *(inst)++ = (unsigned char)0xd9; *(inst)++ = (unsigned char)0xe4; } while (0)
#define x86_fxam(inst) do { *(inst)++ = (unsigned char)0xd9; *(inst)++ = (unsigned char)0xe5; } while (0)
#define x86_fpatan(inst) do { *(inst)++ = (unsigned char)0xd9; *(inst)++ = (unsigned char)0xf3; } while (0)
#define x86_fprem(inst) do { *(inst)++ = (unsigned char)0xd9; *(inst)++ = (unsigned char)0xf8; } while (0)
#define x86_fprem1(inst) do { *(inst)++ = (unsigned char)0xd9; *(inst)++ = (unsigned char)0xf5; } while (0)
#define x86_frndint(inst) do { *(inst)++ = (unsigned char)0xd9; *(inst)++ = (unsigned char)0xfc; } while (0)
#define x86_fsqrt(inst) do { *(inst)++ = (unsigned char)0xd9; *(inst)++ = (unsigned char)0xfa; } while (0)
#define x86_fptan(inst) do { *(inst)++ = (unsigned char)0xd9; *(inst)++ = (unsigned char)0xf2; } while (0)

#define x86_padding(inst,size)	\
	do {	\
		switch ((size)) {	\
		case 1: x86_nop ((inst)); break;	\
		case 2: *(inst)++ = 0x8b;	\
			*(inst)++ = 0xc0; break;	\
		case 3: *(inst)++ = 0x8d; *(inst)++ = 0x6d;	\
			*(inst)++ = 0x00; break;	\
		case 4: *(inst)++ = 0x8d; *(inst)++ = 0x64;	\
			*(inst)++ = 0x24; *(inst)++ = 0x00;	\
			break;	\
		case 5: *(inst)++ = 0x8d; *(inst)++ = 0x64;	\
			*(inst)++ = 0x24; *(inst)++ = 0x00;	\
			x86_nop ((inst)); break;	\
		case 6: *(inst)++ = 0x8d; *(inst)++ = 0xad;	\
			*(inst)++ = 0x00; *(inst)++ = 0x00;	\
			*(inst)++ = 0x00; *(inst)++ = 0x00;	\
			break;	\
		case 7: *(inst)++ = 0x8d; *(inst)++ = 0xa4;	\
			*(inst)++ = 0x24; *(inst)++ = 0x00;	\
			*(inst)++ = 0x00; *(inst)++ = 0x00;	\
			*(inst)++ = 0x00; break;	\
		default: assert (0);	\
		}	\
	} while (0)

#define x86_prolog(inst,frame_size,reg_mask)	\
	do {	\
		unsigned i, m = 1;	\
		x86_enter ((inst), (frame_size));	\
		for (i = 0; i < X86_NREG; ++i, m <<= 1) {	\
			if ((reg_mask) & m)	\
				x86_push_reg ((inst), i);	\
		}	\
	} while (0)

#define x86_epilog(inst,reg_mask)	\
	do {	\
		unsigned i, m = 1 << X86_EDI;	\
		for (i = X86_EDI; m != 0; i--, m=m>>1) {	\
			if ((reg_mask) & m)	\
				x86_pop_reg ((inst), i);	\
		}	\
		x86_leave ((inst));	\
		x86_ret ((inst));	\
	} while (0)


typedef enum {
	X86_SSE_SQRT = 0x51,
	X86_SSE_RSQRT = 0x52,
	X86_SSE_RCP = 0x53,
	X86_SSE_ADD = 0x58,
	X86_SSE_DIV = 0x5E,
	X86_SSE_MUL = 0x59,
	X86_SSE_SUB = 0x5C,
	X86_SSE_MIN = 0x5D,
	X86_SSE_MAX = 0x5F,
	X86_SSE_COMP = 0xC2,
	X86_SSE_AND = 0x54,
	X86_SSE_ANDN = 0x55,
	X86_SSE_OR = 0x56,
	X86_SSE_XOR = 0x57,
	X86_SSE_UNPCKL = 0x14,
	X86_SSE_UNPCKH = 0x15,

	X86_SSE_ADDSUB = 0xD0,
	X86_SSE_HADD = 0x7C,
	X86_SSE_HSUB = 0x7D,
	X86_SSE_MOVSHDUP = 0x16,
	X86_SSE_MOVSLDUP = 0x12,
	X86_SSE_MOVDDUP = 0x12,
	X86_SSE_SHUF = 0xC6,

	X86_SSE_COMI = 0x2F,

	X86_SSE_PAND = 0xDB,
	X86_SSE_POR = 0xEB,
	X86_SSE_PXOR = 0xEF,

	X86_SSE_PADDB = 0xFC,
	X86_SSE_PADDW = 0xFD,
	X86_SSE_PADDD = 0xFE,
	X86_SSE_PADDQ = 0xD4,

	X86_SSE_PSUBB = 0xF8,
	X86_SSE_PSUBW = 0xF9,
	X86_SSE_PSUBD = 0xFA,
	X86_SSE_PSUBQ = 0xFB,

	X86_SSE_PMAXSB = 0x3C, /*sse41*/
	X86_SSE_PMAXSW = 0xEE,
	X86_SSE_PMAXSD = 0x3D, /*sse41*/

	X86_SSE_PMAXUB = 0xDE,
	X86_SSE_PMAXUW = 0x3E, /*sse41*/
	X86_SSE_PMAXUD = 0x3F, /*sse41*/

	X86_SSE_PMINSB = 0x38, /*sse41*/
	X86_SSE_PMINSW = 0xEA,
	X86_SSE_PMINSD = 0x39,/*sse41*/

	X86_SSE_PMINUB = 0xDA,
	X86_SSE_PMINUW = 0x3A, /*sse41*/
	X86_SSE_PMINUD = 0x3B, /*sse41*/

	X86_SSE_PAVGB = 0xE0,
	X86_SSE_PAVGW = 0xE3,

	X86_SSE_PCMPEQB = 0x74,
	X86_SSE_PCMPEQW = 0x75,
	X86_SSE_PCMPEQD = 0x76,
	X86_SSE_PCMPEQQ = 0x29, /*sse41*/

	X86_SSE_PCMPGTB = 0x64,
	X86_SSE_PCMPGTW = 0x65,
	X86_SSE_PCMPGTD = 0x66,
	X86_SSE_PCMPGTQ = 0x37, /*sse42*/

	X86_SSE_PSADBW = 0xf6,

	X86_SSE_PSHUFD = 0x70,

	X86_SSE_PUNPCKLBW = 0x60,
	X86_SSE_PUNPCKLWD = 0x61,
	X86_SSE_PUNPCKLDQ = 0x62,
	X86_SSE_PUNPCKLQDQ = 0x6C,

	X86_SSE_PUNPCKHBW = 0x68,
	X86_SSE_PUNPCKHWD = 0x69,
	X86_SSE_PUNPCKHDQ = 0x6A,
	X86_SSE_PUNPCKHQDQ = 0x6D,

	X86_SSE_PACKSSWB = 0x63,
	X86_SSE_PACKSSDW = 0x6B,

	X86_SSE_PACKUSWB = 0x67,
	X86_SSE_PACKUSDW = 0x2B,/*sse41*/

	X86_SSE_PADDUSB = 0xDC,
	X86_SSE_PADDUSW = 0xDD,
	X86_SSE_PSUBUSB = 0xD8,
	X86_SSE_PSUBUSW = 0xD9,

	X86_SSE_PADDSB = 0xEC,
	X86_SSE_PADDSW = 0xED,
	X86_SSE_PSUBSB = 0xE8,
	X86_SSE_PSUBSW = 0xE9,

	X86_SSE_PMULLW = 0xD5,
	X86_SSE_PMULLD = 0x40,/*sse41*/
	X86_SSE_PMULHUW = 0xE4,
	X86_SSE_PMULHW = 0xE5,
	X86_SSE_PMULUDQ = 0xF4,

	X86_SSE_PMOVMSKB = 0xD7,

	X86_SSE_PSHIFTW = 0x71,
	X86_SSE_PSHIFTD = 0x72,
	X86_SSE_PSHIFTQ = 0x73,
	X86_SSE_SHR = 2,
	X86_SSE_SAR = 4,
	X86_SSE_SHL = 6,

	X86_SSE_PSRLW_REG = 0xD1,
	X86_SSE_PSRAW_REG = 0xE1,
	X86_SSE_PSLLW_REG = 0xF1,

	X86_SSE_PSRLD_REG = 0xD2,
	X86_SSE_PSRAD_REG = 0xE2,
	X86_SSE_PSLLD_REG = 0xF2,

	X86_SSE_PSRLQ_REG = 0xD3,
	X86_SSE_PSLLQ_REG = 0xF3,

	X86_SSE_PREFETCH = 0x18,
	X86_SSE_MOVNTPS = 0x2B,
 	X86_SSE_MOVHPD_REG_MEMBASE = 0x16,
 	X86_SSE_MOVHPD_MEMBASE_REG = 0x17,

 	X86_SSE_MOVSD_REG_MEMBASE = 0x10,
 	X86_SSE_MOVSD_MEMBASE_REG = 0x11,

	X86_SSE_PINSRB = 0x20,/*sse41*/
	X86_SSE_PINSRW = 0xC4,
	X86_SSE_PINSRD = 0x22,/*sse41*/

	X86_SSE_PEXTRB = 0x14,/*sse41*/
	X86_SSE_PEXTRW = 0xC5,
	X86_SSE_PEXTRD = 0x16,/*sse41*/
} X86_SSE_Opcode;

typedef enum {
	X86_SSE_CMP_EQ = 0,
	X86_SSE_CMP_LT = 1,
	X86_SSE_CMP_LE = 2,
	X86_SSE_CMP_UNORD = 3,
	X86_SSE_CMP_NEQ = 4,
	X86_SSE_CMP_NLT = 5,
	X86_SSE_CMP_NLE = 6,
	X86_SSE_CMP_ORD = 7
} X86_SSE_CmpCode;


/* minimal SSE* support */
#define x86_movsd_reg_membase(inst,dreg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xf2;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x10;	\
		x86_membase_emit ((inst), (dreg), (basereg), (disp));	\
	} while (0)

#define x86_movsd_reg_mem(inst,dreg,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xf2;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x10;	\
		x86_mem_emit ((inst), (dreg), (mem));	\
	} while (0)


#define x86_movsd_reg_reg(inst,dreg,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0xf2;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x11;	\
		x86_reg_emit ((inst), (reg), (dreg));	\
	} while (0)

#define x86_cvtsd2ss(inst,dreg,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0xf2;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x5a;	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

#define x86_cvtss2sd(inst,dreg,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0xf3;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x5a;	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

#define x86_cvtss2sd_reg_membase(inst,reg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xf3;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x5a;	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_cvtss2sd_reg_memindex(inst,reg,basereg,disp,indexreg,shift)	\
	do {	\
		*(inst)++ = (unsigned char)0xf3;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x5a;	\
		x86_memindex_emit ((inst), (reg), (basereg), (disp), (indexreg), (shift));	\
	} while (0)

#define x86_cvtss2sd_reg_mem(inst,reg,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xf3;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x5a;	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)


#define x86_cvttsd2si(inst,dreg,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0xf2;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x2c;	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

#define x86_cvtsi2sd(inst,dreg,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0xf2;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x2a;	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

#define x86_sse_alu_reg_reg(inst,opc,dreg,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x0F;	\
		*(inst)++ = (unsigned char)(opc);	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

#define x86_sse_alu_reg_membase(inst,opc,sreg,basereg,disp)	\
		do {	\
			*(inst)++ = (unsigned char)0x0f;	\
			*(inst)++ = (unsigned char)(opc);	\
			x86_membase_emit ((inst), (sreg), (basereg), (disp));	\
		} while (0)

#define x86_sse_alu_membase_reg(inst,opc,basereg,disp,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x0F;	\
		*(inst)++ = (unsigned char)(opc);	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)


#define x86_sse_alu_pd_reg_reg(inst,opc,dreg,reg)       \
	do {    \
		*(inst)++ = (unsigned char)0x66;        \
		x86_sse_alu_reg_reg ((inst), (opc), (dreg), (reg)); \
	} while (0)

#define x86_sse_alu_pd_membase_reg(inst,opc,basereg,disp,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		x86_sse_alu_membase_reg ((inst), (opc), (basereg), (disp), (reg)); \
	} while (0)

#define x86_sse_alu_pd_reg_membase(inst,opc,dreg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		x86_sse_alu_reg_membase ((inst), (opc), (dreg),(basereg), (disp)); \
	} while (0)

#define x86_sse_alu_pd_reg_mem(inst,opc,dreg,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		*(inst)++ = (unsigned char)0x0F;        \
		*(inst)++ = (unsigned char)(opc);       \
		x86_mem_emit ((inst), (dreg), (mem));	\
	} while (0)


#define x86_sse_alu_pd_reg_reg_imm(inst,opc,dreg,reg,imm)	\
	do {	\
		x86_sse_alu_pd_reg_reg ((inst), (opc), (dreg), (reg)); \
		*(inst)++ = (unsigned char)(imm);	\
	} while (0)

#define x86_sse_alu_sd_reg_reg_imm(inst,opc,dreg,reg,imm)	\
	do {	\
		x86_sse_alu_sd_reg_reg ((inst), (opc), (dreg), (reg)); \
		*(inst)++ = (unsigned char)(imm);	\
	} while (0)

#define x86_sse_alu_pd_reg_membase_imm(inst,opc,dreg,basereg,disp,imm)	\
	do {	\
		x86_sse_alu_pd_reg_membase ((inst), (opc), (dreg),(basereg), (disp)); \
		*(inst)++ = (unsigned char)(imm);	\
	} while (0)


#define x86_sse_alu_ps_reg_reg(inst,opc,dreg,reg)	\
	do {	\
		x86_sse_alu_reg_reg ((inst), (opc), (dreg), (reg)); \
	} while (0)

#define x86_sse_alu_ps_reg_reg_imm(inst,opc,dreg,reg, imm)	\
	do {	\
		x86_sse_alu_reg_reg ((inst), (opc), (dreg), (reg)); \
		*(inst)++ = (unsigned char)imm;	\
	} while (0)


#define x86_sse_alu_sd_reg_reg(inst,opc,dreg,reg)       \
	do {    \
		*(inst)++ = (unsigned char)0xF2;        \
		x86_sse_alu_reg_reg ((inst), (opc), (dreg), (reg)); \
	} while (0)

#define x86_sse_alu_sd_membase_reg(inst,opc,basereg,disp,reg)	\
	do {    \
		*(inst)++ = (unsigned char)0xF2;        \
		x86_sse_alu_membase_reg ((inst), (opc), (basereg), (disp), (reg));	\
	} while (0)

#define x86_sse_alu_sd_reg_mem(inst,opc,dreg,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xF2;	\
		*(inst)++ = (unsigned char)0x0F;        \
		*(inst)++ = (unsigned char)(opc);       \
		x86_mem_emit ((inst), (dreg), (mem));	\
	} while (0)



#define x86_sse_alu_ss_reg_reg(inst,opc,dreg,reg)       \
	do {    \
		*(inst)++ = (unsigned char)0xF3;        \
		x86_sse_alu_reg_reg ((inst), (opc), (dreg), (reg)); \
	} while (0)

#define x86_sse_alu_ss_membase_reg(inst,opc,basereg,disp,reg)       \
	do {    \
		*(inst)++ = (unsigned char)0xF3;        \
		x86_sse_alu_membase_reg ((inst), (opc), (basereg), (disp), (reg));	\
	} while (0)



#define x86_sse_alu_sse41_reg_reg(inst,opc,dreg,reg)       \
	do {    \
		*(inst)++ = (unsigned char)0x66;        \
		*(inst)++ = (unsigned char)0x0F;	\
		*(inst)++ = (unsigned char)0x38;	\
		*(inst)++ = (unsigned char)(opc);	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

#define x86_movups_reg_membase(inst,sreg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x10;	\
		x86_membase_emit ((inst), (sreg), (basereg), (disp));	\
	} while (0)

#define x86_movups_membase_reg(inst,basereg,disp,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x11;	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_movaps_reg_membase(inst,sreg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x28;	\
		x86_membase_emit ((inst), (sreg), (basereg), (disp));	\
	} while (0)

#define x86_movaps_membase_reg(inst,basereg,disp,reg)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x29;	\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_movaps_reg_reg(inst,dreg,sreg)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x28;	\
		x86_reg_emit ((inst), (dreg), (sreg));	\
	} while (0)


#define x86_movd_reg_xreg(inst,dreg,sreg)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x7e;	\
		x86_reg_emit ((inst), (sreg), (dreg));	\
	} while (0)

#define x86_movd_xreg_reg(inst,dreg,sreg)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x6e;	\
		x86_reg_emit ((inst), (dreg), (sreg));	\
	} while (0)

#define x86_movd_xreg_membase(inst,sreg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x6e;	\
		x86_membase_emit ((inst), (sreg), (basereg), (disp));	\
	} while (0)

#define x86_movlpd_xreg_memindex(inst,sreg,basereg,disp,indexreg,shift)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x12;	\
		x86_memindex_emit ((inst), (sreg), (basereg), (disp), (indexreg), (shift));	\
	} while (0)

#define x86_movlpd_xreg_membase(inst,sreg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x12;	\
		x86_membase_emit ((inst), (sreg), (basereg), (disp));	\
	} while (0)

#define x86_movlpd_xreg_mem(inst,sreg,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x12;	\
		x86_mem_emit ((inst), (sreg), (mem));	\
	} while (0)

#define x86_movhpd_xreg_membase(inst,sreg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x16;	\
		x86_membase_emit ((inst), (sreg), (basereg), (disp));	\
	} while (0)

#define x86_movlpd_membase_xreg(inst,sreg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x13;	\
		x86_membase_emit ((inst), (sreg), (basereg), (disp));	\
	} while (0)

#define x86_movlpd_memindex_xreg(inst,sreg,basereg,disp,indexreg,shift)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x13;	\
		x86_memindex_emit ((inst), (sreg), (basereg), (disp), (indexreg), (shift));	\
	} while (0)


#define x86_movlpd_mem_xreg(inst,sreg,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x13;	\
		x86_mem_emit ((inst), (sreg), (mem));	\
	} while (0)

#define x86_movss_mem_xreg(inst,sreg,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xf3;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x11;	\
		x86_mem_emit ((inst), (sreg), (mem));	\
	} while (0)

#define x86_movss_xreg_mem(inst,sreg,mem)	\
	do {	\
		*(inst)++ = (unsigned char)0xf3;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x10;	\
		x86_mem_emit ((inst), (sreg), (mem));	\
	} while (0)

#define x86_movss_membase_xreg(inst,sreg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xf3;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x11;	\
		x86_membase_emit ((inst), (sreg), (basereg), (disp));	\
	} while (0)

#define x86_movss_xreg_membase(inst,sreg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0xf3;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x10;	\
		x86_membase_emit ((inst), (sreg), (basereg), (disp));	\
	} while (0)


#define x86_movss_memindex_xreg(inst,sreg,basereg,disp,indexreg,shift)	\
	do {	\
		*(inst)++ = (unsigned char)0xf3;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x11;	\
		x86_memindex_emit ((inst), (sreg), (basereg), (disp), (indexreg), (shift));	\
	} while (0)

#define x86_movss_xreg_memindex(inst,sreg,basereg,disp,indexreg,shift)	\
	do {	\
		*(inst)++ = (unsigned char)0xf3;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x10;	\
		x86_memindex_emit ((inst), (sreg), (basereg), (disp), (indexreg), (shift));	\
	} while (0)


#define x86_movhpd_membase_xreg(inst,sreg,basereg,disp)	\
	do {	\
		*(inst)++ = (unsigned char)0x66;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x17;	\
		x86_membase_emit ((inst), (sreg), (basereg), (disp));	\
	} while (0)

#define x86_pshufw_reg_reg(inst,dreg,sreg,mask,high_words)	\
	do {	\
		*(inst)++ = (unsigned char)(high_words) ? 0xF3 : 0xF2;	\
		*(inst)++ = (unsigned char)0x0f;	\
		*(inst)++ = (unsigned char)0x70;	\
		x86_reg_emit ((inst), (dreg), (sreg));	\
		*(inst)++ = (unsigned char)mask;	\
	} while (0)

#define x86_sse_shift_reg_imm(inst,opc,mode, dreg,imm)	\
	do {	\
		x86_sse_alu_pd_reg_reg (inst, opc, mode, dreg);	\
		x86_imm_emit8 ((inst), (imm));	\
	} while (0)

#define x86_sse_shift_reg_reg(inst,opc,dreg,sreg)	\
	do {	\
		x86_sse_alu_pd_reg_reg (inst, opc, dreg, sreg);	\
	} while (0)


/*
 * MOVSX, MOVZX
 */

#define x86_movzx_reg_reg(inst,dreg,reg,size)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		switch ((size)) {	\
		case 1: *(inst)++ = (unsigned char)0xb6; break;	\
		case 2: *(inst)++ = (unsigned char)0xb7; break; \
		default: assert (0);	\
		}	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

#define x86_movsx_reg_reg(inst,dreg,reg,size)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		switch ((size)) {	\
		case 1: *(inst)++ = (unsigned char)0xbe; break;	\
		case 2: *(inst)++ = (unsigned char)0xbf; break; \
		default: assert (0);	\
		}	\
		x86_reg_emit ((inst), (dreg), (reg));	\
	} while (0)

#define x86_movsx_reg_mem(inst,reg,mem,size)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		switch ((size)) {	\
		case 1: *(inst)++ = (unsigned char)0xbe; break;	\
		case 2: *(inst)++ = (unsigned char)0xbf; break;	\
		default: assert (0);	\
		}	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)

#define x86_movsx_reg_membase(inst,reg,basereg,disp,size)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		switch (size) {\
			case 1: *(inst)++ = (unsigned char)0xbe; break;	\
			case 2: *(inst)++ = (unsigned char)0xbf; break;	\
			default: assert(0);\
		}\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_movsx_reg_memindex(inst,reg,basereg,disp,indexreg,shift,size)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		switch ((size)) {	\
			case 1: *(inst)++ = (unsigned char)0xbe; break;	\
			case 2: *(inst)++ = (unsigned char)0xbf; break;	\
			default: assert (0);	\
		}	\
		x86_memindex_emit ((inst), (reg), (basereg), (disp), (indexreg), (shift));	\
	} while (0)

#define x86_movzx_reg_mem(inst,reg,mem,size)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		switch ((size)) {	\
		case 1: *(inst)++ = (unsigned char)0xb6; break;	\
		case 2: *(inst)++ = (unsigned char)0xb7; break;	\
		default: assert (0);	\
		}	\
		x86_mem_emit ((inst), (reg), (mem));	\
	} while (0)


#define x86_movzx_reg_membase(inst,reg,basereg,disp,size)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		switch (size) {\
			case 1: *(inst)++ = (unsigned char)0xb6; break;	\
			case 2: *(inst)++ = (unsigned char)0xb7; break;	\
			default: assert(0);\
		}\
		x86_membase_emit ((inst), (reg), (basereg), (disp));	\
	} while (0)

#define x86_movzx_reg_memindex(inst,reg,basereg,disp,indexreg,shift,size)	\
	do {	\
		*(inst)++ = (unsigned char)0x0f;	\
		switch ((size)) {	\
			case 1: *(inst)++ = (unsigned char)0xb6; break;	\
			case 2: *(inst)++ = (unsigned char)0xb7; break;	\
			default: assert (0);	\
		}	\
		x86_memindex_emit ((inst), (reg), (basereg), (disp), (indexreg), (shift));	\
	} while (0)

#define x86_cmc(inst)       \
	do {    \
		*(inst)++ = (unsigned char)0xf5; \
	} while (0)

#endif // X86_H
