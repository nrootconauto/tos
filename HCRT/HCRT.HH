#define NULL (0(U0*))
#define FALSE 0
#define TRUE 1
#define U8 U8i
#define I8 I8i
class CCmpCtrl;
U16i union U16
{
  I8i	i8[2];
  U8i	u8[2];
};
I16i union I16
{
  I8i	i8[2];
  U8i	u8[2];
};

U32i union U32
{
  I8i	i8[4];
  U8i	u8[4];
  I16	i16[2];
  U16	u16[2];
};

I32i union I32
{
  I8i	i8[4];
  U8i	u8[4];
  I16	i16[2];
  U16	u16[2];
};

U64i union U64
{
  I8i	i8[8];
  U8i	u8[8];
  I16	i16[4];
  U16	u16[4];
  I32	i32[2];
  U32	u32[2];
};

I64i union I64
{
  I8i	i8[8];
  U8i	u8[8];
  I16	i16[4];
  U16	u16[4];
  I32	i32[2];
  U32	u32[2];
};

class CFs {
  U64i except_ch;
  U64i catch_except;
  I64i rand_seed;
  CCmpCtrl *last_cc;
};

//MATH
#define I8_MIN (-0x80)
#define I8_MAX 0x7F
#define U8_MIN 0
#define U8_MAX 0xFF
#define I16_MIN (-0x8000)
#define I16_MAX 0x7FFF
#define U16_MIN 0
#define U16_MAX 0xFFFF
#define I32_MIN (-0x80000000)
#define I32_MAX 0x7FFFFFFF
#define U32_MIN 0
#define U32_MAX 0xFFFFFFFF
#define I64_MIN (-0x8000000000000000)
#define I64_MAX 0x7FFFFFFFFFFFFFFF
#define U64_MIN 0
#define U64_MAX 0xFFFFFFFFFFFFFFFF
#define U64_F64_MAX (0x43F0000000000000(F64))
#define F64_MAX (0x7FEFFFFFFFFFFFFF(F64))
#define F64_MIN (0xFFEFFFFFFFFFFFFF(F64))
#define inf	(0x7FF0000000000000(F64))
#define pi	(0x400921FB54442D18(F64))
extern I64 ToI64(F64 i);
extern F64 ToF64(I64 i);
extern F64 Clamp(F64 d,F64 lo,F64 hi);
extern I64 MinI64(I64 n1,I64 n2);
extern F64 Min(F64 n1,F64 n2);
extern F64 Max(F64 n1,F64 n2);
extern I64 MaxI64(I64 n1,I64 n2);
extern U64 FloorU64(U64 num,U64 to);
extern U64 CeilU64(U64 num,U64 to);
extern I64 RoundI64(I64 num,I64 to);
extern I64 FloorI64(I64 num,I64 to);
extern I64 CeilI64(I64 num,I64 to);
extern I16 RandI16();
extern U16 RandU16();
extern I32 RandI32();
extern U32 RandU32();
extern I64 RandI64();
extern U64 RandU64();
extern F64 Rand();
extern I64 RandRange(I64 min=0, I64 max);
extern I64 RandRangeSeed(I64 min=0, I64 max, I64 seed);
extern U64 ClampU64(U64 a,U64 min=0,U64 max);
extern I64 ClampI64(I64 a,I64 min=0,I64 max);
//BITS.HC
extern I64 BCnt(I64 value);
extern Bool Bt(U8 *data,I64 bit);
extern U0 BEqu(U8 *data,I64 bit,Bool v);
extern Bool Btc(U8 *data,I64 bit);
extern Bool Bts(U8 *data,I64 bit);
extern Bool Btr(U8 *data,I64 bit);
extern Bool BFieldOrU32(U8 *f,I64 bit,U32 patt);
extern U32 BFieldExtU32(U8 *f,I64 bit,I64 size);
//COMPRESS.HC
class CArcCompress;
extern U8 *ExpandBuf(CArcCompress *arc);
extern CArcCompress *CompressBuf(U8 *src,I64 size);
extern U8 *StrNew(U8 *in);
extern U8 ToUpper(U8 chr);
extern  U32
  char_bmp_alpha[16],
  char_bmp_alpha_numeric[16],
  char_bmp_alpha_numeric_no_at[16],
  char_bmp_word[16],
  char_bmp_filename[16],
  char_bmp_dec_numeric[16],
  char_bmp_hex_numeric[16],
  char_bmp_white_space[16],
  char_bmp_non_eol_white_space[16],
  char_bmp_zero_cr_nl_cursor[16],
  char_bmp_zero_tab_cr_nl_cursor[16],
  char_bmp_zero_tab_cr_nl_cursor_dollar[16],
  char_bmp_macro[16],
  char_bmp_printable[16],
  char_bmp_displayable[16],
  char_bmp_safe_dollar[16],
  char_bmp_non_eol[16];
extern I64 Str2I64(U8 *st,I64 radix=10,U8 **_end_ptr=NULL);
extern F64 Str2F64(U8 *src,U8 **_end_ptr=NULL);
extern U8 *StrScan(U8 *src,U8 *fmt,...);
extern U8 *GetStr(U8 *msg=NULL,U8 *dft=NULL,I64 flags=0);
extern Bool YorN();
extern I64 PressAKey();
extern I64 GetI64(U8 *msg=NULL,I64 dft=0,I64 lo=I64_MIN,I64 hi=I64_MAX);
extern F64 GetF64(U8 *msg=NULL,F64 dft=0,F64 lo=F64_MIN,F64 hi=F64_MAX);
extern U8 *StrIMatch(U8 *src,U8 *find);
extern I64 StrOcc(U8 *src, I64 ch);
extern U8 *StrFirstOcc(U8 *src,U8 *marker);
extern U8 *StrLastOcc(U8 *str,U8 *marker);
//FIFO.HC
class CFifoU8 {
  U8 *buf;
  I64 mask,in_ptr,out_ptr;
};
class CFifoI64 {
  I64 *buf;
  I64 mask,in_ptr,out_ptr;
};
extern CFifoU8 *FifoU8New(I64 size);
extern U0 FifoU8Del(CFifoU8 *f);
extern Bool FifoU8Ins(CFifoU8 *f,U8 b);
extern Bool FifoU8Rem(CFifoU8 *f,U8 *_b);
extern Bool FifoU8Peek(CFifoU8 *f,U8 *_b);
extern U0 FifoU8Flush(CFifoU8 *f);
extern I64 FifoU8Cnt(CFifoU8 *f);
extern CFifoI64 *FifoI64New(I64 size);
extern U0 FifoI64Del(CFifoI64 *f);
extern Bool FifoI64Ins(CFifoI64 *f,I64 q);
extern Bool FifoI64Rem(CFifoI64 *f,I64 *_q);
extern Bool FifoI64Peek(CFifoI64 *f,I64 *_q);
extern U0 FifoI64Flush(CFifoI64 *f);
extern I64 FifoI64Cnt(CFifoI64 *f);
#define CDIR_FILENAME_LEN 1024
//FILE.HC
class CDirEntry
{
  CDirEntry *next,*parent,*prev;
 U16 attr;	//Unused in Cubit
 U8 name[CDIR_FILENAME_LEN];
 I64 size;	//In bytes
 I64 datetime; //In Unix time(not used yet)
};
extern U8 *FileExtDot(U8 *src);

#define FUF_RECURSE (1<<0)
#define FUF_SINGLE (1<<1)
#define FUF_JUST_DIRS (1<<3)
#define FUF_JUST_DIRS (1<<4)
#define FUF_JUST_FILES (1<<5)
#define FUF_JUST_TXT (1<<6)
#define FUF_JUST_DD (1<<7)
#define FUF_JUST_SRC (1<<9)
extern CDirEntry *FilesFind(U8 *mask,I64 flags=0);
extern U0 DirEntryFree(CDirEntry *ents);
extern U0 Dir(U8 *path=".");
extern U0 Zip(U8 *f,I64 flags=0);
extern U0 Unzip(U8 *f,I64 flags=0);
extern U0 Move(U8 *mask,U8 *to);
//HASH.HC
class CHash {
  CHash* next;
  U8* str;
  U32 type, use_cnt;
};
class CHashTable {
  I64 mask;
  CHash** body;
};
extern CHashTable* HashTableNew(I64 size, U0* pool = NULL);
extern I64 HashStr(U8* str);
extern U0 HashAdd(CHash* tmp, CHashTable* t);
extern CHash* HashFind(U8* needle, CHashTable* table);
U0 HashTableDel(CHashTable* table, U0(*free)(CHash* h) = NULL);
//PRINT.HC
extern U8 *StrPrintJoin(U8 *dst,U8 *fmt,I64 argc,I64 *argv);
extern U8 *StrPrint(U8 *dst,U8 *fmt,...);
extern U8 *CatPrint(U8 *_dst,U8 *fmt,...);
extern U0 Print(U8 *fmt,...);
extern U8 *MStrPrint(U8 *fmt,...);
//QSORT.HC
extern U0 SwapI64(I64 *a,I64 *b);
extern U0 QSortI64(I64 *base,I64 num, I64 (*fp_compare)(I64 e1,I64 e2));
extern U0 QSort2a(U8 **base,I64 num,I64 (*fp_compare)(U8 **_e1,U8 **_e2));
extern U0 QSort2b(U8 *base,I64 num, I64 width,
	I64 (*fp_compare)(U8 *e1,U8 *e2),U8 *tmp);
extern U0 QSort(U8 *base,I64 num, I64 width, I64 (*fp_compare)(U8 *e1,U8 *e2));
//ASM.HC
#define SREG 0
#define GP_8 1
#define GP_16 2
#define GP_32 3
#define GP_64 4
#define X87 5
#define MM 6
#define XMM 7
class CReg:CHash {
    I64 code,type;
};
#define X64ModRMIndir 0b00
#define X64ModRMIndir8 0b01
#define X64ModRMIndir32 0b10
#define X64ModRMDirect 0b11
#define OPC_F_SR 1 // /R
#define OPC_F_ADDR (1<<1) //+R
#define OPC_F_IB (1<<3)
#define OPC_F_IW (1<<4)
#define OPC_F_ID (1<<5)
#define OPC_F_IQ (1<<6)
#define OPC_F_O0 (1<<7)
#define OPC_F_O1 (1<<8)
#define OPC_F_O2 (1<<9)
#define OPC_F_O3 (1<<10)
#define OPC_F_O4 (1<<11)
#define OPC_F_O5 (1<<12)
#define OPC_F_O6 (1<<13)
#define OPC_F_O7 (1<<14)
#define OPC_F_32bit (1<<15)
#define OPC_F_48REX (1<<16)
#define OPC_F_REX_R8_15ONLY (1<<17)
#define OPC_F_STI (1<<18)
#define OPC_F_ADDI (1<<19)
//Operand size prefix
#define OPC_F_16 (1<<20)
#define OPC_F_COND (1<<21)
class COpcodeArg {
    I64 type;
    #define OPCA_NONE 0
    #define OPCA_REG 1
    #define OPCA_R8 2
    #define OPCA_R16 3
    #define OPCA_R32 4
    #define OPCA_R64 5
    #define OPCA_RM8 6
    #define OPCA_RM16 7
    #define OPCA_RM32 8
    #define OPCA_RM64 9
    #define OPCA_IMM8 10
    #define OPCA_IMM16 11
    #define OPCA_IMM32 12
    #define OPCA_IMM64 13
    #define OPCA_MOFFS8 14
    #define OPCA_MOFFS16 15
    #define OPCA_MOFFS32 16
    #define OPCA_UIMM8 17
    #define OPCA_UIMM16 18
    #define OPCA_UIMM32 19
    #define OPCA_UIMM64 20
    #define OPCA_REL8 21
    #define OPCA_REL16 22
    #define OPCA_REL32 23
    #define OPCA_M8 24
    #define OPCA_M16 25
    #define OPCA_M32 26
    #define OPCA_M64 27
    #define OPCA_STI 28
    #define OPCA_SREG 29
    union {
        CReg *reg;
    };
};
#define HASH_TYPE_OPCODE 0
#define HASH_TYPE_OPCODE_ALIAS 1
class COpcode:CHash {
    COpcode *prev_op,*next_op;
    U8 bytes[16];
    I64 bcnt;
    I64 flags,arg_count;
    COpcodeArg args[3];
};
class COpcodeAlias:CHash {
    COpcode *opcode;
};
class X64Mode {
    I64 type;
    #define X64O_REG 0
    #define X64O_MEM 1
    #define X64O_IMM 2
    #define X64O_LABEL 3
    union {
        CReg *reg;
        class {
	    CReg *segment;
            CReg *base;
            CReg *index;
            I64 scale;
            I32 disp;
            I64 width;
        } mem;
        class {
            U0 *jit_label;
            I64 label_offset;
        };
        U64 imm;
    };
};
extern CReg *Str2Reg(U8 *str);
extern X64Mode *X64ModeLabel(U0 *lab);
extern X64Mode *X64ModeReg(CReg *reg);
extern U8 *AsmError;
extern X64Mode *X64ModeMem(CReg *segment,I64 scale,CReg *index,CReg *base,I64 disp,I64 width=8);
extern X64Mode *X64ModeImm(U64 imm);
extern U8 X64ModRM(I64 modrmMode,I64 r, I64 rm);
extern U8 X64SIB(I64 s,I64 i,I64 b);
extern Bool IsOpcode(U8 *name);
extern CReg *GetRegister(U8 *name);
extern U8 *X86ModRMSIBDisp(U8 *it,I64 size,U8 *opcode,I64 opc_len,CReg *reg,I64 scale,CReg *index,CReg *base,I64 disp,I64 *sibooff=NULL);
extern U0 X64ModeFree(X64Mode *m) ;
/**
 * Jumps are not encoded here
 */
extern U8 *EncodeOpcode(U8 *ip,U8 *name,Bool *is_jmp,I64 *sibooff ,I64 *immoff,...);
//LEXER.HC
CCmpCtrl *CmpCtrlNew(U8 *buf=NULL,I64 flags=0,U8 *filename=NULL);
I64 Lex(CCmpCtrl *cc);
U0 *LexItem(CCmpCtrl *ctrl);
U0 LexIncludeStr(CCmpCtrl *cc,U8 *abs_filename,U8 *src,Bool actual_file);
Bool LexSetReplMode(CCmpCtrl *ctrl,Bool set); //Returns old value
U8 *LexMatchFile(CCmpCtrl *cc,U8 *name);
I64 *LexFileLines(CCmpCtrl *cc,U8 *name,I64 *cnt);
U0 __StreamPrint(U8 *fmt,...);
// ../HolyEd/EDITOR.HC
U0 Ed(U8 *fn);
