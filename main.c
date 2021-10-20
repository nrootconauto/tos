#include "3d.h"
#include "ext/argtable3/argtable3.h"
#include <signal.h>
#ifndef TARGET_WIN32
#include <sys/syscall.h>
#include <unistd.h>
#define HCRT_INSTALLTED_DIR "/usr/local/include/HolyC/HCRT.HC"
#include <libgen.h>
#else
#include <windows.h>
#include <libloaderapi.h>
#include <fileapi.h>
#include <winnt.h>
#define HCRT_INSTALLTED_DIR "\\HCRT\\HCRT.HC"
#endif
static struct arg_lit *helpArg;
static struct arg_lit *dbgArg;
static struct arg_lit *boundsArg;
static struct arg_lit *silentArg;
static struct arg_end *endArg;
static struct arg_file *includeArg;
static struct arg_file *tagsArg;
static struct arg_file *errsFile;
ExceptBuf SigPad;
char CompilerPath[1024];
#ifdef TARGET_WIN32
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x4
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x200
static LONG WINAPI VectorHandler (struct _EXCEPTION_POINTERS *info) {
  switch(info->ExceptionRecord->ExceptionCode) {
    #define FERR(code) case code: printf("Caught %s.\nType 'Exit(0);' to exit.\n",#code); HCLongJmp(&SigPad);
    FERR(EXCEPTION_ACCESS_VIOLATION);
    FERR(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
    FERR(EXCEPTION_DATATYPE_MISALIGNMENT);
    FERR(EXCEPTION_FLT_DENORMAL_OPERAND);
    FERR(EXCEPTION_FLT_DIVIDE_BY_ZERO);
    FERR(EXCEPTION_FLT_INEXACT_RESULT);
    FERR(EXCEPTION_FLT_INVALID_OPERATION);
    FERR(EXCEPTION_FLT_OVERFLOW);
    FERR(EXCEPTION_FLT_STACK_CHECK);
    FERR(EXCEPTION_FLT_UNDERFLOW);
    FERR(EXCEPTION_ILLEGAL_INSTRUCTION);
    FERR(EXCEPTION_IN_PAGE_ERROR);
    FERR(EXCEPTION_INT_DIVIDE_BY_ZERO);
    FERR(EXCEPTION_INVALID_DISPOSITION);
    FERR(EXCEPTION_STACK_OVERFLOW);
    default:;
  }
  //SignalHandler(0);
  return EXCEPTION_CONTINUE_EXECUTION;
}
BOOL WINAPI CtrlCHandlerRoutine(DWORD c) {
  printf("User Abort.\n");
  return FALSE;
}
#endif
int main(int argc,char **argv) {
    #ifndef TARGET_WIN32
    char *rp=realpath(argv[0],NULL);
    strcpy(CompilerPath,rp);
    free(rp);
    #else
    SetConsoleCtrlHandler(CtrlCHandlerRoutine,TRUE);
    GetFullPathNameA(argv[0],sizeof(CompilerPath),CompilerPath,NULL);
    DWORD omode, origOmode;
    GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &omode);
    origOmode = omode;
    omode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), omode);
    #endif
    void *argtable[]= {
        helpArg=arg_lit0("h", "help", "Display this help message."),
        dbgArg=arg_lit0("d","debug", "Start in debug mode(Use \"Debugger;\")."),
        silentArg=arg_lit0("s","silent", "Dont report warnings and errors(usefull when dumping tags)."),
        boundsArg=arg_lit0("b", "bounds", "This enables bounds checking."),
        tagsArg=arg_file0("t", "tags", "<file>", "File to dump tags to(ctags compatable)."),
        errsFile=arg_file0("e","diags","<file>","Dump diagnostics to a file."),
        includeArg=arg_filen(NULL, NULL, "<file>", 0, 1024, "Files to include after loading."),
        endArg=arg_end(1),
    };
    int errs=arg_parse(argc, argv, argtable);
    int run=1;
    if(helpArg->count||errs) {
        printf("Usage is: 3d");
        arg_print_syntaxv(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "  %-25s %s\n");
        run=0;
    }
    if(errsFile->count) {
      Compiler.errorsFile=fopen(errsFile->filename[0],"w");
    }
    if(boundsArg->count)
        Compiler.boundsCheckMode=1;
    if(dbgArg->count||boundsArg->count)
        Compiler.debugMode=1;
    if(silentArg->count)
      Compiler.silentMode=1;
    CreateGC(__builtin_frame_address(0), 0!=Compiler.boundsCheckMode);
    AddGCRoot(&Compiler, sizeof(Compiler));
    AddGCRoot(&Debugger, sizeof(Debugger));
    AddGCRoot(&Lexer, sizeof(Lexer));
    CreateLexer(PARSER_HOLYC);
    InitREPL();
    Lexer.replMode=1;
    RegisterBuiltins();
    char *tagf=NULL;
    if(tagsArg->count) {
      tagf=strdup(tagsArg->filename[0]);
    }
    long iter;
    char buffer[2048];
    char buffer2[2048];
#ifndef TARGET_WIN32
    if(0==access(HCRT_INSTALLTED_DIR,F_OK)) {
        sprintf(buffer, "#include \"%s\"", HCRT_INSTALLTED_DIR);
        mrope_append_text(Lexer.source, strdup(buffer));
    } else {
      strcpy(buffer,argv[0]);
      strcat(dirname(buffer),"/HCRT/HCRT.HC");
      if(0==access(buffer, F_OK)) {
        sprintf(buffer2, "#include \"%s\"", buffer);
        mrope_append_text(Lexer.source, strdup(buffer2));
      }
    }
#else
  GetModuleFileNameA(NULL,buffer,sizeof(buffer));
  dirname(buffer);
  strcat(buffer,HCRT_INSTALLTED_DIR);
  if(GetFileAttributesA(buffer)!=INVALID_FILE_ATTRIBUTES) {
    unescapeString(buffer,buffer2);
    sprintf(buffer, "#include \"%s\"", strdup(buffer2));
    mrope_append_text(Lexer.source, strdup(buffer));
  }
#endif
    for(iter=0; iter!=includeArg->count; iter++) {
        unescapeString(includeArg->filename[iter],buffer2);
        sprintf(buffer, "#include \"%s\"", buffer2);
        mrope_append_text(Lexer.source, strdup(buffer));
    }
    arg_freetable(argtable, sizeof(argtable)/sizeof(*argtable));
    signal(SIGSEGV,SignalHandler);
    signal(SIGABRT,SignalHandler);
    #ifndef TARGET_WIN32
    signal(SIGBUS,SignalHandler);
    signal(SIGFPE,SignalHandler);
    signal(SIGILL,SignalHandler);
    signal(SIGINT,SignalHandler);
    #endif
    Compiler.tagsFile=tagf;
    if(Compiler.tagsFile) Lexer.replMode=0;
    if(run&&!errs) {
        for(;;) {
set:
            ;
            int sig;
            #ifdef TARGET_WIN32
            HANDLE h=AddVectoredExceptionHandler(1,VectorHandler);
            #endif
            if(sig=HCSetJmp(SigPad)) {
              err:
                #ifdef TARGET_WIN32
                RemoveVectoredExceptionHandler(h);
                #endif
                vec_truncate(&Debugger.callStack,0);
                printf("Recieved signal %d. Discarding input.\n",sig);
#ifndef TARGET_WIN32
                sigset_t empty;
                sigfillset(&empty);
                sigprocmask(SIG_UNBLOCK,&empty,NULL);
#endif
                vec_truncate(&Lexer.ifStates,0);
                FlushLexer();
                signal(SIGSEGV,SignalHandler);
                signal(SIGABRT,SignalHandler);
                #ifndef TARGET_WIN32
                signal(SIGBUS,SignalHandler);
                signal(SIGFPE,SignalHandler);
                signal(SIGILL,SignalHandler);
                signal(SIGINT,SignalHandler);
                #endif
                goto set;
            }
            Compiler.errorFlag=0;
            Compiler.inFunction=0;
            HC_parse();
            #ifdef TARGET_WIN32
            RemoveVectoredExceptionHandler(h);
            #endif
            if(Compiler.tagsFile) {
              DumpTagsToFile(tagf);
              break;
            }
        }
    }
    TD_FREE(tagf);
    if(Compiler.errorsFile) fclose(Compiler.errorsFile);
    return (errs)?EXIT_SUCCESS:EXIT_FAILURE;
}