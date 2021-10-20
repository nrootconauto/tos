#include <stdlib.h>
#include <stdio.h>

// include the header file
#include "myjit/jitlib.h"

// pointer to a function accepting one argument of type unsigned short and returning long value
typedef long (* plfus)(unsigned short);

int main()
{
	// we create a new instance of a compiler
	struct jit * p = jit_init();

	plfus fib;

	// label of the same function
	jit_label * fibfn  = jit_get_label(p);

	// the code generated by the compiler will be assigned to the function `foo'
	jit_prolog(p, &fib);

	// the first argument of the fucntion is `unsigned short' number
	jit_declare_arg(p, JIT_UNSIGNED_NUM, sizeof(short));

	// moves the first argument into the register R(0)
	jit_getarg(p, R(0), 0);

	jit_op * zero = jit_beqi(p, JIT_FORWARD, R(0), 0);
	jit_op * one = jit_beqi(p, JIT_FORWARD, R(0), 1);

	// calls the fib. function with the first argument having value R(0) - 1
	jit_subi(p, R(0), R(0), 1);

	// prepares function call
	jit_prepare(p);

	// passes an argument
	jit_putargr(p, R(0));

	// calls the functions
	jit_call(p, fibfn);

	// stores the result into R(1)
	jit_retval(p, R(1));

	// another call of fib. function
	jit_subi(p, R(0), R(0), 1);
	jit_prepare(p);
	jit_putargr(p, R(0));
	jit_call(p, fibfn);

	// stores the result into R(2)
	jit_retval(p, R(2));

	// sums values in R(1) and R(2)
	jit_addr(p, R(1), R(1), R(2));

	// jump to return
	jit_op * ret1 = jit_jmpi(p, JIT_FORWARD);

	// returns 0
	jit_patch(p, zero);
	jit_movi(p, R(1), 0);
	jit_op * ret2 = jit_jmpi(p, JIT_FORWARD);


	// returns 1
	jit_patch(p, one);
	jit_movi(p, R(1), 1);


	jit_patch(p, ret1);
	jit_patch(p, ret2);

	// returns the fibonacci number
	jit_retr(p, R(1));

	// compiles the above defined code
	jit_generate_code(p);

	// if you are interested, you can dump the machine code
	//jit_dump_ops(p, JIT_DEBUG_CODE);

	// or you can inspect how each operation is transformed
	// into a machine code
	jit_dump_ops(p, JIT_DEBUG_COMBINED);

	// check
	printf("Check #1: fib(1) = %li\n", fib(1));
	printf("Check #2: fib(5) = %li\n", fib(5));
	printf("Check #3: fib(30) = %li\n", fib(30));

	// cleanup
	jit_free(p);
	return 0;
}