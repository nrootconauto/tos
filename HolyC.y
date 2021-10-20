%{
#include "3d.h"
#include <stdio.h>
int yylex();
void yyerror (char const *s);
#define SOT(a,tok) SetPosFromOther(a,tok)
#define SLE(a) SetPosFromLex(a)
static CFuncInfo *CurFuncInfo;
static void *CurFramePtr;
static void (*RunPtr)(CFuncInfo *info,AST *exp,void *framePtr);
%}
%define api.prefix {HC_}
%define api.token.prefix {HC_}
%token CHAR INT FLOAT
%token DOT ARROW
%token  SIZEOF ADDR_OF  LNOT BNOT
%token  POW SHL SHR
%token  MUL DIV MOD
%token  BAND BOR BXOR
%token  ADD SUB
%token  LT GT LE GE
%token  NE EQ
%token  LAND
%token  LXOR
%token  LOR
%token  ASSIGN EQ_SHL EQ_SHR EQ_MUL EQ_DIV EQ_MOD EQ_BAND EQ_BXOR EQ_BOR EQ_ADD EQ_SUB
%token  COMMA
%token  OTHER TRY CATCH LASTCLASS U0 LEFT_PAREN RIGHT_PAREN INC DEC NAME LEFT_SQAURE RIGHT_SQAURE SEMI IF ELSE DO WHILE FOR LEFT_CURLY RIGHT_CURLY CASE COLON DOT_DOT_DOT
%token  EXTERN2 LOCK EXTERN IMPORT IMPORT2 STATIC PUBLIC CLASS UNION INTERN START END DEFAULT BREAK RET GOTO BOOL U8 I8 U16 I16 U32 I32 U64 I64 F64  SWITCH
/**
 * These tokens is used by forking the lexer  and parsing a scope/expression
 */
%token EXE EVAL NL DBG
%start global_stmt
/**
 * Precednece
 */
%nonassoc CHAR INT FLOAT STRING
%left DOT ARROW
%right SIZEOF LNOT BNOT
%left POW SHL SHR
%left MUL DIV MOD
%left BAND
%left BXOR
%left BOR
%left ADD SUB
%left LT GT LE GE
%left EQ NE
%left LAND
%left LXOR
%left LOR
%right ASSIGN  EQ_SHL EQ_SHR EQ_MUL EQ_DIV EQ_MOD EQ_BAND EQ_BXOR EQ_BOR EQ_ADD EQ_SUB
%left COMMA
%nonassoc IF ELSE DO WHILE FOR
%nonassoc TRY CATCH LASTCLASS U0 LEFT_PAREN RIGHT_PAREN INC DEC NAME LEFT_SQAURE RIGHT_SQAURE SEMI  LEFT_CURLY RIGHT_CURLY CASE COLON DOT_DOT_DOT
%nonassoc EXTERN2 LOCK EXTERN IMPORT IMPORT2 STATIC PUBLIC CLASS UNION INTERN START END DEFAULT BREAK GOTO BOOL U8 I8 U16 I16 U32 I32 U64 I64 F64 SWITCH TYPENAME
%%
body: stmts;
expr0[r]: FLOAT {$r=SLE($1);};
expr0[r]: INT {$r=SLE($1);};
expr0[r]: CHAR {$r=SLE($1);};
expr0[r]: STRING {$r=SLE($1);};
expr0[r]: LASTCLASS {
  $r=SLE($1);
};
expr0[r]: NAME {
  $r=SLE($1);
};
_callargs[r]: {$r=NULL;};
_callargs[r]: expr[a] {$r=$a;};
callargs[r]: callargs[a] COMMA[p] _callargs[b] {
  $r=SOT(CreateBinop($a,$b,AST_COMMA),$p);
};
callargs[r]: _callargs[a] {$r=$a;};
expr1: expr0;
expr1[r]: LEFT_PAREN[un1] expr_comma[e] RIGHT_PAREN[un2] {
  $r=$e;
  ReleaseAST($un1),ReleaseAST($un2);
}

expr2[r]: expr1;
expr2[r]: expr2[e] DOT[p] NAME[n] {
  $r=SOT(CreateMemberAccess($e,$n),$p);
  ReleaseAST($p);
};
expr2[r]: expr2[e] ARROW[p] NAME[n] {
  $r=SOT(CreateMemberAccess($e,$n),$p);
  ReleaseAST($p);
};
expr2[r]: expr2[e] LEFT_SQAURE[un1] expr_comma[s] RIGHT_SQAURE[un2] {
  $r=SOT(CreateArrayAccess($e,$s),$e);
  ReleaseAST($un1),ReleaseAST($un2);
};
expr2[r]: expr2[e] DEC[p] {
  $r=SOT(CreateUnop($e,AST_POST_DEC),$p);
  ReleaseAST($p);
};
expr2[r]: expr2[e] INC[p] {
  $r=SOT(CreateUnop($e,AST_POST_INC),$p);
  ReleaseAST($p);
};
expr2[r]: expr2[f] LEFT_PAREN[un1] callargs[e] RIGHT_PAREN[un2] {
  $r=SOT(CreateFuncCall($f,$e),$f);
  ReleaseAST($un1),ReleaseAST($un2);
};
expr2[r]: expr2[f] LEFT_PAREN[un1] primtype0[t] RIGHT_PAREN[un2] {
  $r=CreateExplicitTypecast($f,$t);
  ReleaseAST($un1),ReleaseAST($un2);
};
expr2[r]: expr2[f] LEFT_PAREN[un1] primtype0[t] MUL[un2] RIGHT_PAREN[un3] {
  $t->type2=CreateMultiLvlPtr($t->type2,1);
  $r=CreateExplicitTypecast($f,$t);
  ReleaseAST($un1),ReleaseAST($un2),ReleaseAST($un3);
};
expr2[r]: expr2[f] LEFT_PAREN[un1] primtype0[t] MUL[un2] MUL[un3] RIGHT_PAREN[un4] {
  $t->type2=CreateMultiLvlPtr($t->type2,2);
  $r=CreateExplicitTypecast($f,$t);
  ReleaseAST($un1),ReleaseAST($un2),ReleaseAST($un3),ReleaseAST($un4);
};
expr2[r]: expr2[f] LEFT_PAREN[un1] primtype0[t] MUL[un2] MUL[un3] MUL[un4] RIGHT_PAREN[un5] {
  $t->type2=CreateMultiLvlPtr($t->type2,3);
  $r=CreateExplicitTypecast($f,$t);
  ReleaseAST($un1),ReleaseAST($un2),ReleaseAST($un3),ReleaseAST($un4),ReleaseAST($un5);
};
/**
 * Type declarations for sizeof
 */
sizeof_type[r]: primtype0[n] {$r=$n;};
sizeof_type[r]: sizeof_type[n] MUL[un1] {
  CType *ptr =CreateMultiLvlPtr($n->type2,1);
  $r=CreateTypeNode(ptr);
  ReleaseAST($un1);
}
expr3[r]: expr2;
expr3[r]: SIZEOF[p] NAME[e] {
  $r=SOT(CreateSizeof($e),$p);
  ReleaseAST($p);
};
expr3[r]: SIZEOF[p] sizeof_type[n] {
  $r=SOT(CreateSizeof($n),$p);
  ReleaseAST($p);
};
expr3[r]: SIZEOF[p] LEFT_PAREN[un1] expr_comma[e] RIGHT_PAREN[un2]  {
  $r=SOT(CreateSizeof($e),$p);
  ReleaseAST($p);
  ReleaseAST($un1);
  ReleaseAST($un2);
};
expr3[r]: SIZEOF[p] LEFT_PAREN[un1] sizeof_type[e] RIGHT_PAREN[un2]  {
  $r=SOT(CreateSizeof($e),$p);
  ReleaseAST($p);
  ReleaseAST($un1);
  ReleaseAST($un2);
};
expr3[r]: BAND[p] expr3[e] {
  $r=SOT(CreateUnop($e,AST_ADDROF),$p);
  ReleaseAST($p);
};
expr3[r]: MUL[p] expr3[e] {
  $r=SOT(CreateUnop($e,AST_DERREF),$p);
  ReleaseAST($p);
};
//expr3: expr2 LEFT_PAREN type RIGHT_PAREN;
expr3[r]: DEC[p] expr3[e] {
  $r=SOT(CreateUnop($e,AST_PRE_DEC),$p);
  ReleaseAST($p);
};
expr3[r]: INC[p] expr3[e] {
  $r=SOT(CreateUnop($e,AST_PRE_INC),$p);
  ReleaseAST($p);
};
expr3[r]: ADD[p] expr3[e] {
  $r=SOT(CreateUnop($e,AST_POS),$p);
  ReleaseAST($p);
};
expr3[r]: SUB[p] expr3[e] {
  $r=SOT(CreateUnop($e,AST_NEG),$p);
  ReleaseAST($p);
};
expr3[r]: LNOT[p] expr3[e] {
  $r=SOT(CreateUnop($e,AST_LNOT),$p);
  ReleaseAST($p);
};
expr3[r]: BNOT[p] expr3[e] {
  $r=SOT(CreateUnop($e,AST_BNOT),$p);
  ReleaseAST($p);
};

//
expr4[r]: expr3;
expr4[r]: expr4[a] POW[p] expr4[b] {
  $r=SOT(CreateBinop($a,$b,AST_POW),$p);
  ReleaseAST($p);
};
expr4[r]: expr4[a] SHL[p] expr4[b] {
  $r=SOT(CreateBinop($a,$b,AST_SHL),$p);
  ReleaseAST($p);
};
expr4[r]: expr4[a] SHR[p] expr4[b] {
  $r=SOT(CreateBinop($a,$b,AST_SHR),$p);
  ReleaseAST($p);
};

expr4_5[r]: expr4;
expr4_5[r]: expr4_5[a] MUL[p] expr4_5[b] {
  $r=SOT(CreateBinop($a,$b,AST_MUL),$p);
  ReleaseAST($p);
};
expr4_5[r]: expr4_5[a] DIV[p] expr4_5[b] {
  $r=SOT(CreateBinop($a,$b,AST_DIV),$p);
  ReleaseAST($p);
};
expr4_5[r]: expr4_5[a] MOD[p] expr4_5[b] {
  $r=SOT(CreateBinop($a,$b,AST_MOD),$p);
  ReleaseAST($p);
};

expr5[r]: expr4_5;
expr5[r]: expr5[a] BAND[p] expr5[b] {
  $r=SOT(CreateBinop($a,$b,AST_BAND),$p);
  ReleaseAST($p);
};

expr6[r]: expr5;
expr6[r]: expr6[a] BXOR[p] expr6[b] {
  $r=SOT(CreateBinop($a,$b,AST_BXOR),$p);
  ReleaseAST($p);
};

expr7[r]: expr6;
expr7[r]: expr7[a] BOR[p] expr7[b] {
  $r=SOT(CreateBinop($a,$b,AST_BOR),$p);
  ReleaseAST($p);
};

expr8[r]: expr7;
expr8[r]: expr8[a] ADD[p] expr8[b] {
  $r=SOT(CreateBinop($a,$b,AST_ADD),$p);
  ReleaseAST($p);
};
expr8[r]: expr8[a] SUB[p] expr8[b] {
  $r=SOT(CreateBinop($a,$b,AST_SUB),$p);
  ReleaseAST($p);
};

expr9[r]: expr8;
expr9[r]: expr9[a] LT[p] expr9[b] {
  $r=SOT(AppendToRange($a,$b,AST_LT),$p);
  ReleaseAST($p);
};
expr9[r]: expr9[a] GT[p] expr9[b] {
  $r=SOT(AppendToRange($a,$b,AST_GT),$p);
  ReleaseAST($p);
};
expr9[r]: expr9[a] LE[p] expr9[b] {
  $r=SOT(AppendToRange($a,$b,AST_LE),$p);
  ReleaseAST($p);
};
expr9[r]: expr9[a] GE[p] expr9[b] {
  $r=SOT(AppendToRange($a,$b,AST_GE),$p);
  ReleaseAST($p);
};

expr10: expr9;
expr10[r]: expr10[a] EQ[p] expr10[b] {
  $r=SOT(CreateBinop($a,$b,AST_EQ),$p);
  ReleaseAST($p);
};
expr10[r]: expr10[a] NE[p] expr10[b] {
  $r=SOT(CreateBinop($a,$b,AST_NE),$p);
  ReleaseAST($p);
};

expr11: expr10;
expr11[r]: expr11[a] LAND[p] expr11[b] {
  $r=SOT(CreateBinop($a,$b,AST_LAND),$p);
  ReleaseAST($p);
};

expr12: expr11;
expr12[r]: expr12[a] LXOR[p] expr12[b] {
  $r=SOT(CreateBinop($a,$b,AST_LXOR),$p);
  ReleaseAST($p);
};

expr13: expr12;
expr13[r]: expr13[a] LOR[p] expr13[b] {
  $r=SOT(CreateBinop($a,$b,AST_LOR),$p);
  ReleaseAST($p);
};

expr14: expr13;
expr14[r]: expr14[a] ASSIGN[p] expr14[b] {
  $r=SOT(CreateBinop($a,$b,AST_ASSIGN),$p);
  ReleaseAST($p);
};
expr14[r]: expr14[a] EQ_SHL[p] expr14[b] {
  $r=SOT(CreateBinop($a,$b,AST_ASSIGN_SHL),$p);
  ReleaseAST($p);
};
expr14[r]: expr14[a] EQ_SHR[p] expr14[b] {
  $r=SOT(CreateBinop($a,$b,AST_ASSIGN_SHR),$p);
  ReleaseAST($p);
};
expr14[r]: expr14[a] EQ_MUL[p] expr14[b] {
  $r=SOT(CreateBinop($a,$b,AST_ASSIGN_MUL),$p);
  ReleaseAST($p);
};
expr14[r]: expr14[a] EQ_DIV[p] expr14[b] {
  $r=SOT(CreateBinop($a,$b,AST_ASSIGN_DIV),$p);
  ReleaseAST($p);
};
expr14[r]: expr14[a] EQ_MOD[p] expr14[b] {
  $r=SOT(CreateBinop($a,$b,AST_ASSIGN_MOD),$p);
  ReleaseAST($p);
};
expr14[r]: expr14[a] EQ_BAND[p] expr14[b] {
  $r=SOT(CreateBinop($a,$b,AST_ASSIGN_BAND),$p);
  ReleaseAST($p);
};
expr14[r]: expr14[a] EQ_BXOR[p] expr14[b] {
  $r=SOT(CreateBinop($a,$b,AST_ASSIGN_BXOR),$p);
  ReleaseAST($p);
};
expr14[r]: expr14[a] EQ_BOR[p] expr14[b] {
  $r=SOT(CreateBinop($a,$b,AST_ASSIGN_BOR),$p);
  ReleaseAST($p);
};
expr14[r]: expr14[a] EQ_ADD[p] expr14[b] {
  $r=SOT(CreateBinop($a,$b,AST_ASSIGN_ADD),$p);
  ReleaseAST($p);
};
expr14[r]: expr14[a] EQ_SUB[p] expr14[b] {
  $r=SOT(CreateBinop($a,$b,AST_ASSIGN_SUB),$p);
  ReleaseAST($p);
};

expr: expr14;

expr_comma[r]: expr_comma[a] COMMA[p] expr[b] {
  $r=SOT(CreateBinop($a,$b,AST_COMMA),$p);
  ReleaseAST($p);
};
expr_comma: expr;

/**
 * Types
 */
primtype0: BOOL[un1] {CType *prim =CreatePrimType(TYPE_BOOL);$$=SLE(CreateTypeNode(prim));ReleaseAST($un1);};
primtype0: U0[un1] {CType *prim =CreatePrimType(TYPE_U0);$$=SLE(CreateTypeNode(prim));ReleaseAST($un1);};
primtype0: U8[un1] {CType *prim =CreatePrimType(TYPE_U8);$$=SLE(CreateTypeNode(prim));ReleaseAST($un1);};
primtype0: I8[un1] {CType *prim =CreatePrimType(TYPE_I8);$$=SLE(CreateTypeNode(prim));ReleaseAST($un1);};
primtype0: U16[un1] {CType *prim =CreatePrimType(TYPE_U16);$$=SLE(CreateTypeNode(prim));ReleaseAST($un1);};
primtype0: I16[un1] {CType *prim =CreatePrimType(TYPE_I16);$$=SLE(CreateTypeNode(prim));ReleaseAST($un1);};
primtype0: U32[un1] {CType *prim =CreatePrimType(TYPE_U32);$$=SLE(CreateTypeNode(prim));ReleaseAST($un1);};
primtype0: I32[un1] {CType *prim =CreatePrimType(TYPE_I32);$$=SLE(CreateTypeNode(prim));ReleaseAST($un1);};
primtype0: U64[un1] {CType *prim =CreatePrimType(TYPE_U64);$$=SLE(CreateTypeNode(prim));ReleaseAST($un1);};
primtype0: I64[un1] {CType *prim =CreatePrimType(TYPE_I64);$$=SLE(CreateTypeNode(prim));ReleaseAST($un1);};
primtype0: F64[un1] {CType *prim =CreatePrimType(TYPE_F64);$$=SLE(CreateTypeNode(prim));ReleaseAST($un1);};
primtype0[r]: TYPENAME[n] {
  CType **cls=map_get(&Compiler.types,$n->name);
  CType *t ;
  if(cls) t=*cls;
  else {
    RaiseError($n,"Invalid type \"%s\"",$n->name);
    t=CreatePrimType(TYPE_I64);
  }
  $r=CreateTypeNode(t);
};
primtype0[r]: _class[c] {$r=$c;};
primtype0[r]: _union[c] {$r=$c;};
primtype0[r]: primtype0[bt] _union[t] {
  $r=$t;
  AssignUnionBasetype($t,$bt);
};
/**
 * Array/Class literals
 */
_arrlit[r]: _arrlit[a] COMMA[un1] expr[b] {$r=AppendToArrLiteral($a,$b);ReleaseAST($un1);}
_arrlit[r]: _arrlit[a] COMMA[un1] arrlit[b] {$r=AppendToArrLiteral($a,$b);ReleaseAST($un1);}
_arrlit: expr {$$=AppendToArrLiteral(NULL,$1);};
_arrlit: arrlit {$$=AppendToArrLiteral(NULL,$1);};
arrlit[r]: LEFT_CURLY[un1] _arrlit[b] RIGHT_CURLY[un2] {
  $r=$b;
  ReleaseAST($un1);
  ReleaseAST($un2);
};
arrlit[r]:LEFT_CURLY[un1] _arrlit[b] COMMA[un2] RIGHT_CURLY[un3] {
  $r=$b;
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
};
arrlit[r]:LEFT_CURLY[un1] COMMA[un2] RIGHT_CURLY[un3] {
  $r=NULL;
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
};
/**
 * Classes/unions
 */
metadata[r]: NAME[n] expr[e] {
  $r=AppendToMetaData(NULL,$n,$e);
};
cdecltail[r]: cdecltail[a] COMMA[un1] _cdecltail[b] {
  assert($b->type==__AST_DECL_TAILS);
  CDeclTail b2=vec_last(&$b->declTail);
  AppendToDecls($a,b2.basetype,b2.finalType,b2.name,b2.dft,b2.metaData);
  $r=$a;
  ReleaseAST($un1);
};
cdecltail[r]: _cdecltail[t] {$r=$t;};
_cdecltail[r]: vardecltail[t] {
  AST *t=$t;
  $r=$t;
};
_cdecltail[r]: vardecltail[a] metadata[m] {
  AST *meta=$a->declTail.data[0].metaData;
  meta=AppendToMetaData(meta,$m->metaData.data[0].name,$m->metaData.data[0].value);
  $a->declTail.data[0].metaData=meta;
  $r=$a;
};
cdecl[r]: primtype0[p] cdecltail[t] {
  DeclsFillInBasetype($p->type2,$t);
  $r=$t;
};
cbody[r]: cbody[a] cdecl[b] SEMI[un1] {
 $r=AppendToStmts($a,$b);
 ReleaseAST($un1);
};
cbody[r]: cbody[a] union[b] {
 $r=AppendToStmts($a,$b);
};
cbody[r]: cbody[a] class[b] {
 $r=AppendToStmts($a,$b);
};
cbody: {$$=NULL;};
_cheader[r]: CLASS[un1] {
  CType *t=CreateClassForwardDecl(NULL,NULL);
  $r=CreateTypeNode(t);
  ReleaseAST($un1);
};
_uheader[r]: UNION[un1] {
  CType *t=CreateUnionForwardDecl(NULL,NULL);
  $r=CreateTypeNode(t);
  ReleaseAST($un1);
};
_cheader[r]: CLASS[un1] NAME[n] {
  CType *t=CreateClassForwardDecl(NULL,$n);
  $r=CreateTypeNode(t);
  ReleaseAST($un1);
};
_uheader[r]: UNION[un1] NAME[n] {
  CType *t=CreateUnionForwardDecl(NULL,$n);
  $r=CreateTypeNode(t);
  ReleaseAST($un1);
};
_cheader[r]: CLASS[un1] TYPENAME[n] {
  CType *t=CreateClassForwardDecl(NULL,$n);
  $r=CreateTypeNode(t);
  ReleaseAST($un1);
};
_uheader[r]: UNION TYPENAME[n] {
  CType *t=CreateUnionForwardDecl(NULL,$n);
  $r=CreateTypeNode(t);
};
cheader[r]: _cheader[h] {$r=$h;};
uheader[r]: _uheader[h] {$r=$h;};
cheader[r]: _cheader[h] COLON[un1] TYPENAME[ext] {
  CType *t=*map_get(&Compiler.types,$ext->name);
  InheritFromType($h->type2,t);
  $r=$h;
  ReleaseAST($un1);
};
uheader[r]: _uheader[h] COLON[un1] TYPENAME[ext] {
  CType *t=*map_get(&Compiler.types,$ext->name);
  InheritFromType($h->type2,t);
  $r=$h;
  ReleaseAST($un1);
};
class[r]: _class[s] SEMI[un1] {
  $r=$s;
  ReleaseAST($un1);
};
union[r]: primtype0[bt] _union[s] SEMI[un1] {
  $r=$s;
  AssignUnionBasetype($s,$bt);
  ReleaseAST($un1);
};
union[r]: _union[s] SEMI[un1] {
  $r=$s;
  ReleaseAST($un1);
};

_class[r]: cheader[t] LEFT_CURLY[un1] cbody[b] RIGHT_CURLY[un2] {
  if($b) {
    int iter;
    AST *decl;
    assert($b->type==AST_STMT_GROUP);
    vec_foreach(&$b->stmts,decl,iter)
      AppendToTypeMembers($t,decl);
  }
  $t->type2=FinalizeClass($t->type2);
  $t->type2->cls.isFwd=0;
  AST *t=$t;
  $r=$t;
  ReleaseAST($un1);
  ReleaseAST($un2);
};
_union[r]: uheader[t] LEFT_CURLY[un1] cbody[b] RIGHT_CURLY[un2] {
  if($b) {
    int iter;
    AST *decl;
    vec_foreach(&$b->stmts,decl,iter)
      AppendToTypeMembers($t,decl);
  }
  $t->type2->un.isFwd=0;
  $r=$t;
  ReleaseAST($un1);
  ReleaseAST($un2);
};
//
/**
 * Exceptions
 */
tryblock[r]: TRY[un1] scope[t] CATCH[un2] scope[c] {
  $r=CreateTry($t,$c);
  ReleaseAST($un1);
  ReleaseAST($un2);
};
/**
 * Var decls
 */
namewptrs: NAME[n] {
  CType *t =CreatePrimType(TYPE_I64);
  $$=SOT(AppendToDecls(NULL,t,t,$n,NULL,NULL),$n);
};
namewptrs: MUL[un1] NAME[n] {
  CType *t =CreatePrimType(TYPE_I64);
  CType *ptr =CreateMultiLvlPtr(t,1);
  $$=SOT(AppendToDecls(NULL,t,ptr,$n,NULL,NULL),$n);
  ReleaseAST($un1);
};
namewptrs: MUL[un1] MUL[un2] NAME[n] {
  CType *t =CreatePrimType(TYPE_I64);
  CType *ptr =CreateMultiLvlPtr(t,2);
  $$=SOT(AppendToDecls(NULL,t,ptr,$n,NULL,NULL),$n);
  ReleaseAST($un1);
  ReleaseAST($un2);
};
namewptrs: MUL[un1] MUL[un2] MUL[un3] NAME[n] {
  CType *t =CreatePrimType(TYPE_I64);
  CType *ptr =CreateMultiLvlPtr(t,3);
  $$=SOT(AppendToDecls(NULL,t,ptr,$n,NULL,NULL),$n);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
};
vardecltail: namewptrs;
__ptrcount: {
    $$=CreateI64(0);
}
__ptrcount: MUL[un1] {
  $$=CreateI64(1);
  ReleaseAST($un1);
}
__ptrcount: MUL[un1] MUL[un2] {
  $$=CreateI64(2);
  ReleaseAST($un1),ReleaseAST($un2);
}
__ptrcount: MUL[un1] MUL[un2] MUL[un3] {
  $$=CreateI64(3);
  ReleaseAST($un1),ReleaseAST($un2),ReleaseAST($un3);
}
vardecltail: __ptrcount[cnt] LEFT_PAREN[un1] MUL[un2] NAME[n] RIGHT_PAREN[un3] LEFT_PAREN[un4] funcargs[args] RIGHT_PAREN[un5] {
  CType *bt =CreatePrimType(TYPE_I64);
  CType *btp=CreateMultiLvlPtr(bt,$cnt->integer);
  CType *func =CreateFuncType(btp,$args,0);
  CType *ptr =CreatePtrType(func);
  $$=SOT(AppendToDecls(NULL,bt,ptr,$n,NULL,NULL),$n);
  ReleaseAST($un1),ReleaseAST($cnt);
  ReleaseAST($un2);
  ReleaseAST($un3);
  ReleaseAST($un4);
  ReleaseAST($un5);
};
vardecltail: __ptrcount[cnt] LEFT_PAREN[un1] MUL[un2] MUL[un3] NAME[n] RIGHT_PAREN[un4] LEFT_PAREN[un5] funcargs[args] RIGHT_PAREN[un6] {
  CType *bt =CreatePrimType(TYPE_I64);
  CType *btp=CreateMultiLvlPtr(bt,$cnt->integer);
  CType *func =CreateFuncType(btp,$args,0);
  CType *ptr =CreateMultiLvlPtr(func,2);
  $$=SOT(AppendToDecls(NULL,bt,ptr,$n,NULL,NULL),$n);
  ReleaseAST($un1),ReleaseAST($cnt);
  ReleaseAST($un2);
  ReleaseAST($un3);
  ReleaseAST($un4);
  ReleaseAST($un5);
  ReleaseAST($un6);
};
vardecltail: __ptrcount[cnt] LEFT_PAREN[un1] MUL[un2] MUL[un3] MUL[un4] NAME[n] RIGHT_PAREN[un5] LEFT_PAREN[un6] funcargs[args] RIGHT_PAREN[un7] {
  CType *bt =CreatePrimType(TYPE_I64);
  CType *btp=CreateMultiLvlPtr(bt,$cnt->integer);
  CType *func =CreateFuncType(btp,$args,0);
  CType *ptr =CreateMultiLvlPtr(func,3);
  $$=SOT(AppendToDecls(NULL,bt,ptr,$n,NULL,NULL),$n);
  ReleaseAST($un1),ReleaseAST($cnt);
  ReleaseAST($un2);
  ReleaseAST($un3);
  ReleaseAST($un4);
  ReleaseAST($un5);
  ReleaseAST($un6);
  ReleaseAST($un7);
};
vardecltail[r]: vardecltail[base] LEFT_SQAURE[un1] expr_comma[dim] RIGHT_SQAURE[un2] {
  assert($base->type==__AST_DECL_TAILS);
  CType *bt=vec_last(&$base->declTail).finalType;
  vec_last(&$base->declTail).finalType=CreateArrayType(bt,$dim);
  $r=$base;
  ReleaseAST($un1);
  ReleaseAST($un2);
};
vardecltail[r]: vardecltail[base] LEFT_SQAURE[un1] RIGHT_SQAURE[un2] {
  assert($base->type==__AST_DECL_TAILS);
  CType *bt=vec_last(&$base->declTail).finalType;
  vec_last(&$base->declTail).finalType=CreateArrayType(bt,NULL);
  $r=$base;
  ReleaseAST($un1);
  ReleaseAST($un2);
};

vardecltail_asn[r]: vardecltail[a] ASSIGN[un1] expr[b] {
  assert($a->type==__AST_DECL_TAILS);
  vec_last(&$a->declTail).dft=$b;
  $r=$a;
  ReleaseAST($un1);
}
vardecltail_asn[r]: vardecltail[a] ASSIGN[un1] arrlit[b] {
  assert($a->type==__AST_DECL_TAILS);
  vec_last(&$a->declTail).dft=$b;
  $r=$a;
  ReleaseAST($un1);
}

single_decl[r]: primtype0[bt] vardecltail[tail] {
  DeclsFillInBasetype($bt->type2,$tail);
  $r=$tail;
}
single_decl[r]: primtype0[bt] vardecltail_asn[tail] {
  DeclsFillInBasetype($bt->type2,$tail);
  $r=$tail;
}

vardecltails[r]: vardecltails[a] COMMA[un1] vardecltails[b] {
  assert($b->type==__AST_DECL_TAILS);
  CDeclTail b2=vec_last(&$b->declTail);
  AppendToDecls($a,b2.basetype,b2.finalType,b2.name,b2.dft,NULL);
  $r=$a;
  ReleaseAST($un1);
}
vardecltails: vardecltail_asn;
vardecltails: vardecltail;

multi_decl[r]: primtype0[bt] vardecltails[tail]  {
  DeclsFillInBasetype($bt->type2,$tail);
  $r=$tail;
}

funcargs: single_decl;
funcargs[r]: funcargs[a] COMMA[un1] single_decl[b] {
  assert($b->type==__AST_DECL_TAILS);
  CDeclTail b2=vec_last(&$b->declTail);
  $r=AppendToDecls($a,b2.basetype,b2.finalType,b2.name,b2.dft,NULL);
  ReleaseAST($un1);
};
funcargs: {$$=NULL;};
/**
 * Linkage
 */
linkage: EXTERN[un1] {$$=CreateExternLinkage(NULL);ReleaseAST($un1);};
linkage: IMPORT[un1] {$$=CreateImportLinkage(NULL);ReleaseAST($un1);};
linkage: PUBLIC[un1] {$$=NULL;};
linkage: STATIC[un1] {$$=CreateStaticLinkage();ReleaseAST($un1);};
linkage: EXTERN2[un1] NAME[n] {$$=CreateExternLinkage($n);ReleaseAST($un1);};
linkage: IMPORT2[un1] NAME[n] {$$=CreateImportLinkage($n);ReleaseAST($un1);};
linkage: EXTERN2[un1] TYPENAME[n] {$$=CreateExternLinkage($n);ReleaseAST($un1);};
linkage: IMPORT2[un1] TYPENAME[n] {$$=CreateImportLinkage($n);ReleaseAST($un1);};
/**
 * Functions
 */
func: primtype0[bt] namewptrs[base] LEFT_PAREN[un1] funcargs[args] RIGHT_PAREN[un2] SEMI[un3] {
  DeclsFillInBasetype($bt->type2,$base);
  CType *rtype=$base->declTail.data[0].finalType;
  AST *name=$base->declTail.data[0].name;
  CreateFuncForwardDecl(NULL,rtype,name,$args,0);
 $$=CreateNop();
 ReleaseAST($bt);
 ReleaseAST($base);
 ReleaseAST($args);
 ReleaseAST($un1);
 ReleaseAST($un2);
 ReleaseAST($un3);
};
func: primtype0[bt] namewptrs[base] LEFT_PAREN funcargs[args] COMMA[un1] DOT_DOT_DOT[un2] RIGHT_PAREN[un3] SEMI[un4] {
  DeclsFillInBasetype($bt->type2,$base);
 CType *rtype=$base->declTail.data[0].finalType;
 AST *name=$base->declTail.data[0].name;
 CreateFuncForwardDecl(NULL,rtype,name,$args,1);
 $$=CreateNop();
 ReleaseAST($bt);
 ReleaseAST($base);
 ReleaseAST($args);
 ReleaseAST($un1);
 ReleaseAST($un2);
 ReleaseAST($un3);
 ReleaseAST($un4);
};
func: primtype0[bt] namewptrs[base] LEFT_PAREN[un1] DOT_DOT_DOT[un2] RIGHT_PAREN[un3] SEMI[un4] {
  DeclsFillInBasetype($bt->type2,$base);
 CType *rtype=$base->declTail.data[0].finalType;
 AST *name=$base->declTail.data[0].name;
  CreateFuncForwardDecl(NULL,rtype,name,NULL,1);
 $$=CreateNop();
 ReleaseAST($bt);
 ReleaseAST($base);
 ReleaseAST($un1);
 ReleaseAST($un2);
 ReleaseAST($un3);
 ReleaseAST($un4);
};
func: linkage[l] primtype0[bt] namewptrs[base] LEFT_PAREN[un1] funcargs[args] RIGHT_PAREN[un2] SEMI[un3] {
  DeclsFillInBasetype($bt->type2,$base);
  CType *rtype=$base->declTail.data[0].finalType;
  AST *name=$base->declTail.data[0].name;
  CreateFuncForwardDecl($l,rtype,name,$args,0);
  $$=CreateNop();
  ReleaseAST($l);
  ReleaseAST($bt);
  ReleaseAST($base);
  ReleaseAST($args);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
};
func: linkage[l] primtype0[bt] namewptrs[base] LEFT_PAREN[un1] funcargs[args] COMMA[un2] DOT_DOT_DOT[un3] RIGHT_PAREN[un4] SEMI[un5] {
  DeclsFillInBasetype($bt->type2,$base);
  CType *rtype=$base->declTail.data[0].finalType;
  AST *name=$base->declTail.data[0].name;
  CreateFuncForwardDecl($l,rtype,name,$args,1);
  $$=CreateNop();
  ReleaseAST($bt);
  ReleaseAST($base);
  ReleaseAST($args);
  ReleaseAST($l);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
  ReleaseAST($un4);
  ReleaseAST($un5);
};
func: linkage[l] primtype0[bt] namewptrs[base] LEFT_PAREN[un1] DOT_DOT_DOT[un2] RIGHT_PAREN[un3] SEMI[un4] {
  DeclsFillInBasetype($bt->type2,$base);
  CType *rtype=$base->declTail.data[0].finalType;
  AST *name=$base->declTail.data[0].name;
  CreateFuncForwardDecl($l,rtype,name,NULL,1);
  $$=CreateNop();
  ReleaseAST($l);
  ReleaseAST($bt);
  ReleaseAST($base);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
  ReleaseAST($un4);
};
func[r]: primtype0[bt] namewptrs[base] LEFT_PAREN[un1] DOT_DOT_DOT[un2] RIGHT_PAREN[un3] scope[body] {
  DeclsFillInBasetype($bt->type2,$base);
  CType *rtype=$base->declTail.data[0].finalType;
  AST *name=$base->declTail.data[0].name;
  CompileFunction(NULL,rtype,name,NULL,$body,1);
  $r=CreateNop();

  ReleaseAST($bt);
  ReleaseAST($base);
  ReleaseAST($body);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
}
func[r]: primtype0[bt] namewptrs[base] LEFT_PAREN[un4] funcargs[args] COMMA[un1] DOT_DOT_DOT[un2] RIGHT_PAREN[un3] scope[body] {
  DeclsFillInBasetype($bt->type2,$base);
  CType *rtype=$base->declTail.data[0].finalType;
  AST *name=$base->declTail.data[0].name;
  CompileFunction(NULL,rtype,name,$args,$body,1);
  $r=CreateNop();
  ReleaseAST($bt);
  ReleaseAST($base);
  ReleaseAST($args);
  ReleaseAST($body);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
  ReleaseAST($un4);
}
func[r]: primtype0[bt] namewptrs[base] LEFT_PAREN[un1] funcargs[args] RIGHT_PAREN[un2] scope[body] {
  DeclsFillInBasetype($bt->type2,$base);
  CType *rtype=$base->declTail.data[0].finalType;
  AST *name=$base->declTail.data[0].name;
  CompileFunction(NULL,rtype,name,$args,$body,0);
  $r=CreateNop();
  ReleaseAST($bt);
  ReleaseAST($base);
  ReleaseAST($args);
  ReleaseAST($body);
  ReleaseAST($un1);
  ReleaseAST($un2);
}
func[r]: linkage[l] primtype0[bt] namewptrs[base] LEFT_PAREN[un1] DOT_DOT_DOT[un2] RIGHT_PAREN[un3] scope[body] {
  DeclsFillInBasetype($bt->type2,$base);
  CType *rtype=$base->declTail.data[0].finalType;
  AST *name=$base->declTail.data[0].name;
  CompileFunction($l,rtype,name,NULL,$body,1);
  $r=CreateNop();
  ReleaseAST($l);
  ReleaseAST($bt);
  ReleaseAST($base);
  ReleaseAST($body);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
}
func[r]: linkage[l] primtype0[bt] namewptrs[base] LEFT_PAREN[un1] funcargs[args] COMMA[un2] DOT_DOT_DOT[un3] RIGHT_PAREN[un4] scope[body] {
  DeclsFillInBasetype($bt->type2,$base);
  CType *rtype=$base->declTail.data[0].finalType;
  AST *name=$base->declTail.data[0].name;
  CompileFunction($l,rtype,name,$args,$body,1);
  ReleaseAST($l);
  ReleaseAST($bt);
  ReleaseAST($base);
  ReleaseAST($args);
  ReleaseAST($body);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
  ReleaseAST($un4);
  $r=CreateNop();
}
func[r]: linkage[l] primtype0[bt] namewptrs[base] LEFT_PAREN[un1] funcargs[args] RIGHT_PAREN[un2] scope[body] {
  DeclsFillInBasetype($bt->type2,$base);
  CType *rtype=$base->declTail.data[0].finalType;
  AST *name=$base->declTail.data[0].name;
  CompileFunction($l,rtype,name,$args,$body,0);
  ReleaseAST($l);
  ReleaseAST($bt);
  ReleaseAST($base);
  ReleaseAST($args);
  ReleaseAST($un1);
  ReleaseAST($un2);
  $r=CreateNop($body);
}
/**
 * Body
 */

//http://www.parsifalsoft.com/ifelse.html
ocstmt[r]: ostmt[s] {$r=$s;};
ocstmt[r]: cstmt[s] {$r=$s;};

expr_opt: expr_comma;
expr_opt: {$$=NULL;};

//! Will fill in body later
loop_header[r]: WHILE[p] LEFT_PAREN[un1] expr_comma[cond] RIGHT_PAREN[un2] {
  $r=SOT(CreateWhile($cond,NULL),$p);
  ReleaseAST($un1);
  ReleaseAST($un2);
};
//! Will fill in body later
loop_header[r]: FOR[p] LEFT_PAREN[un1] expr_opt[init] SEMI[un2] expr_opt[cond] SEMI[un3] expr_opt[next] RIGHT_PAREN[un4] {
  $r=SOT(CreateFor($init,$cond,$next,NULL),$p);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
  ReleaseAST($un4);
};

ifcl[r]: IF[p] LEFT_PAREN[un1] expr_comma[e] RIGHT_PAREN[un2] {
  $r=SOT(CreateIf($e,NULL,NULL),$p);
  ReleaseAST($un1);
  ReleaseAST($un2);
};

ostmt[r]: ifcl[i] ocstmt[b] {
  assert($i->type==AST_IF);
  $r=$i;
  $i->ifStmt.body=$b;
};
ostmt[r]: ifcl[i] cstmt[b] ELSE[un1] ostmt[e] {
  assert($i->type==AST_IF);
  $r=$i;
  $i->ifStmt.body=$b;
  $i->ifStmt.elseBody=$e;
  ReleaseAST($un1);
};
ostmt[r]: loop_header[h] ostmt[b] {
  if($h->type==AST_FOR) {
    $h->forStmt.body=$b;
  } else if($h->type==AST_DO) {
    $h->doStmt.body=$b;
  } else if($h->type==AST_WHILE) {
    $h->whileStmt.body=$b;
  }
  $r=$h;
};

cstmt: simple_stmt;
cstmt[r]: ifcl[i] cstmt[b] ELSE[un1] cstmt[e] {
  assert($i->type==AST_IF);
  $r=$i;
  $i->ifStmt.body=$b;
  $i->ifStmt.elseBody=$e;
  ReleaseAST($un1);
};
cstmt[r]: loop_header[h] cstmt[b] {
  if($h->type==AST_FOR) {
    $h->forStmt.body=$b;
  } else if($h->type==AST_DO) {
    $h->doStmt.body=$b;
  } else if($h->type==AST_WHILE) {
    $h->whileStmt.body=$b;
  }
  $r=$h;
};

dostmt[r]: DO[p] ocstmt[b] WHILE[un1] LEFT_PAREN[un2] expr_comma[c] RIGHT_PAREN[un3] {
  $r=SOT(CreateDo($c,$b),$p);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
};
subswit[r]: START[p] COLON[un1] swit_body[b] END[un2] COLON[un3] {
  $r=SOT(CreateSubswitch($b),$p);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
};

swit_body_stmt: ocstmt {$$=$1;};
swit_body_stmt[r]: DEFAULT[p] COLON[un1] {
  $r=SOT(CreateDefault(),$p);
  ReleaseAST($un1);
};
swit_body_stmt[r]: CASE[p] expr[e] COLON[un1] {
  $r=SOT(CreateCase($e,NULL),$p);
  ReleaseAST($un1);
};
swit_body_stmt[r]: CASE[p] COLON[un1] {
  $r=SOT(CreateCase(NULL,NULL),$p);
  ReleaseAST($un1);
};
swit_body_stmt[r]: CASE[p] expr[a] DOT_DOT_DOT[un1] expr[b] COLON[un2] {
  $r=SOT(CreateCase($a,$b),$p);
  ReleaseAST($un1);
  ReleaseAST($un2);

};
swit_body_stmt[r]: subswit[n] {$r=$n;};
swit_body[r]: swit_body[a] swit_body_stmt[s] {$r=AppendToStmts($a,$s);}
swit_body[r]: swit_body_stmt[n] {$r=$n;};
swit[r]: SWITCH[p] LEFT_PAREN[un1] expr_comma[cond] RIGHT_PAREN[un2] LEFT_CURLY[un3] swit_body[body] RIGHT_CURLY[un4] {
  $r=SOT(CreateSwitch($cond,$body),$p);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
  ReleaseAST($un4);
}
swit[r]: SWITCH[p] LEFT_PAREN[un1] expr_comma[cond] RIGHT_PAREN[un2] LEFT_CURLY[un3]  RIGHT_CURLY[un4] {
  $r=SOT(CreateSwitch($cond,NULL),$p);
  ReleaseAST($un1);
  ReleaseAST($un2);
  ReleaseAST($un3);
  ReleaseAST($un4);
}
simple_stmt: tryblock;
simple_stmt[r]: multi_decl[d] SEMI[un] {
  $r=$d;
  ReleaseAST($un);
};
simple_stmt[r]: linkage[l] multi_decl[d] SEMI[un] {
  $r=ApplyLinkageToDecls($l,$d);
  ReleaseAST($un);
};
simple_stmt: func;
simple_stmt[r]: GOTO[p] NAME[n] SEMI[un] {
  $r=SOT(CreateGoto($n),$p);
  ReleaseAST($un);
};
simple_stmt[r]: dostmt[n] SEMI[un] {
  $r=$n;
  ReleaseAST($un);
};
simple_stmt[r]: expr_comma[n] SEMI[un] {
  $r=$n;
  if($n->type==AST_STRING) {
    $r=SOT(CreatePrint($n->string),$n);
  }
  ReleaseAST($un);
};
simple_stmt[r]: swit[n] {$r=$n;};
simple_stmt[r]: scope[n] {$r=$n;};
simple_stmt[r]: BREAK[p] SEMI[un] {
  $r=SOT(CreateBreak(),$p);
  ReleaseAST($un);
};
simple_stmt[r]: SEMI[un] {
  $r=CreateNop();
  ReleaseAST($un);
}
simple_stmt[r]: RET expr_comma[e] SEMI[un] {
  $r=SOT(CreateReturn($e),$e);
  ReleaseAST($un);
};
simple_stmt[r]: RET[r2] SEMI[un] {
  $r=SOT(CreateReturn(NULL),$r2);
  ReleaseAST($un);
};
simple_stmt[r]: NAME[n] COLON[un] {
  $r=SOT(CreateLabel($n),$n);
  ReleaseAST($un);
};
simple_stmt[r]: class[s] {$r=$s;};
simple_stmt[r]: union[s] {$r=$s;};
simple_stmt[r]: linkage[l] UNION[un1] NAME[n] SEMI[un2] {
  CType *t=CreateUnionForwardDecl($l,$n);
  $r=CreateTypeNode(t);
  ReleaseAST($un1);
  ReleaseAST($un2);
};
simple_stmt[r]: linkage[l] CLASS[un1] NAME[n] SEMI[un2] {
  CType *t=CreateClassForwardDecl($l,$n);
  $r=CreateTypeNode(t);
  ReleaseAST($un1);
  ReleaseAST($un2);
};
stmts[r]: ocstmt[n] {$r=$n;};
stmts[r]: stmts[a] ocstmt[s] {$r=AppendToStmts($a,$s);};
stmts: stmts error {YYABORT;};
scope[r]: LEFT_CURLY[un1] stmts[s] RIGHT_CURLY[un2] {
  $r=$s;
  ReleaseAST($un1);
  ReleaseAST($un2);
};
scope[r]: LEFT_CURLY[un1] RIGHT_CURLY[un2] {
  $r=CreateNop();
  ReleaseAST($un1);
  ReleaseAST($un2);
};
global_stmts[r]:{$$=NULL;};
global_stmts[r]: global_stmts ocstmt[s] {
  RunStatement($s);
  ReleaseAST($s);
  $r=NULL;
};
global_stmt[r]: global_stmts[s] {
  $r=NULL;
};

global_stmt[r]: EXE scope[s] {
  RunStatement($s);
  YYACCEPT;
  $r=NULL;
};
global_stmt[r]: EVAL expr_comma[s] NL {
  RunPtr(CurFuncInfo,$s,CurFramePtr);
  YYACCEPT;
  $r=NULL;
};

global_stmt[r]: DBG expr_comma[s] {
  RunPtr(CurFuncInfo,$s,CurFramePtr);
  YYACCEPT;
  $r=NULL;
};
%%
static int code;
int yylex() {
  yylval=LexItem();
  if(!yylval) return 0;
  return ASTToToken(yylval);
}
static void LexerCB(int tok,AST *d) {
  code=tok;
  yylval=d;
  yylex();
}
void AttachParserToLexer() {
    //Lexer.cb=LexerCB;
}
static int __IsTruePassed;
static void __IsTrue(CFuncInfo *dummy1,AST *node,void *fp) {
  CType *rtype =AssignTypeToNode(node);
  COldFuncState old=CreateCompilerState();
  vec_CVariable_t args;
  vec_init(&args);
  Compiler.returnType=rtype;
  AST *retn =CreateReturn(node);
  CFunction *f=CompileAST(NULL,retn,args);
  int ret;
  if(IsF64(rtype)) {
    ret=0!=((double(*)())f->funcptr)();
  } else if(IsIntegerType(rtype)) {
    ret=0!=((int64_t(*)())f->funcptr)();
  }
  ReleaseFunction(f);
  vec_deinit(&args);
  RestoreCompilerState(old);
  __IsTruePassed=ret;
}
int EvalConditionForPreproc() {
  Lexer.stopAtNewline=1;
  Lexer.isEvalMode=1;
  RunPtr=__IsTrue;
  __IsTruePassed=0;
  yyparse();
  Lexer.stopAtNewline=0;
  return __IsTruePassed;
}
void
yyerror (char const *s)
{
  RaiseError(Lexer.lastToken,"Parsing stoped here.");
  FlushLexer();
}
void DebugEvalExpr(void *frameptr,CFuncInfo *info,char *text) {
  CLexer old=ForkLexer(PARSER_HOLYC);
  Lexer.isDebugExpr=1;
  mrope_append_text(Lexer.source,strdup(text));
  CurFuncInfo=info;
  CurFramePtr=frameptr;
  RunPtr=&EvalDebugExpr;
  yyparse();
  printf("\n");
  Lexer=old;
}