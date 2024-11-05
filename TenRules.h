/*
*********************************

	The Power of 10: Rules for Developing Safety-Critical Code
	This header will ensure its implemented.

*********************************
*/

/*
RULE 1: Restrict all code to very simple control flow constructsódo not use goto statements, setjmp or longjmp
constructs, or direct or indirect recursion. (Due to WIN32 API being based on indirect recursion system of messages, you can use only windows messaging system in terms of indirect recursion, however if recursion is not using messaging system, it cannot be justified)

This means no:

for(x)
	if(a=z)
	{
	 do(x);
	}


directly nor indirectly.
*/

/*
RULE 2: Give all loops a fixed upper bound. It must be trivially possible for a checking tool to prove statically that the loop cannot exceed a preset upper bound on the number of iterations. If a tool cannot prove the loop bound statically, the rule is considered violated.

*/
#define MAX_VARIABLE_LOOP_ITERATION 1000 // generic maximum iterations

/*
Rule 3: Do not use dynamic memory allocation after initialization.
Note that the only way to dynamically claim memory in the absence of memory allocation from the heap is to use stack
memory. In the absence of recursion, an upper bound on the use of stack memory can be derived statically, thus making
it possible to prove that an application will always live within its resource bounds.
(We will try and meet this rule, partially by the end of the program finishing, so we can globally understand how much memory, statically, it can use max, until then, we simply dont know.)
*/
//====CONTINUE HERE
/*
Rule 4: No function should be longer than what can be printed on a single sheet of paper in a standard format with one
line per statement and one line per declaration. Typically, this means no more than about 60 lines of code per function.

Rationale : Each function should be a logical unit in the code that is understandable and verifiable as a unit. It is much
harder to understand a logical unit that spans multiple pages. Excessively long functions are often a sign of poorly
structured code.
(dUE TO STRUCTURE of winAPI treat each "message" in a switch, as a logical unit, thus 60 lines per message switch as well)
*/

/*
Rule 5: The code's assertion density should average to minimally two assertions per function. Assertions must be used to
check for anomalous conditions that should never happen in real-life executions. Assertions must be side-effect free and
should be defined as Boolean tests. When an assertion fails, an explicit recovery action must be taken such as returning
an error condition to the caller of the function that executes the failing assertion. Any assertion for which a static checking
tool can prove that it can never fail or never hold violates this rule.

Rationale : Statistics for industrial coding efforts indicate that unit tests often find at least one defect per 10 to 100 lines
of written code. The odds of intercepting defects increase significantly with increasing assertion density. Using assertions
is often recommended as part of a strong defensive coding strategy. Developers can use assertions to verify pre- and
postconditions of functions, parameter values, return values of functions, and loop invariants. Because the proposed
assertions are side-effect free, they can be selectively disabled after testing in performance-critical code.
*/

/*
RULE 6: Declare all data objects at the smallest possible level of scope.

Fewer the statements where the value could have been assigned, the easier it is to diagnose the problem. The rule also discourages
the reuse of variables for multiple, incompatible purposes, which can complicate fault diagnosis.
*/

/*
RULE 7: Each calling function must check the return value of nonvoid functions, and each called function must check the
validity of all parameters provided by the caller.

 Yet, if the response to an error would be no different than the response to success, there is little point
in explicitly checking a return value. This is often the case with calls to printf and close. In cases like these, explicitly
casting the function return value to (void) can be acceptable, thereby indicating that the programmer explicitly and not
accidentally decided to ignore a return value.
*/

/*
RULE 8: The use of the preprocessor must be limited to the inclusion of header files and simple macro definitions. Token
pasting, variable argument lists (ellipses), and recursive macro calls are not allowed. All macros must expand into
complete syntactic units. The use of conditional compilation directives must be kept to a minimum.

Rationale : The C preprocessor is a powerful obfuscation tool that can destroy code clarity and befuddle many text-based
checkers. The effect of constructs in unrestricted preprocessor code can be extremely hard to decipher, even with a formal
language definition.
*/

/*
Rule 9: The use of pointers must be restricted. Specifically, no more than one level of dereferencing should be used.
Pointer dereference operations may not be hidden in macro definitions or inside typedef declarations. Function pointers
are not permitted.

Rationale: Pointers are easily misused, even by experienced programmers. They can make it hard to follow or analyze the
flow of data in a program, especially by tool-based analyzers. Similarly, function pointers should be used only if there is a
very strong justification for doing so because they can seriously restrict the types of automated checks that code checkers
can perform. For example, if function pointers are used, it can become impossible for a tool to prove the absence of
recursion, requiring alternate guarantees to make up for this loss in checking power.
*/

/*
Rule 10: All code must be compiled, from the first day of development, with all compiler warnings enabled at the most
pedantic setting available. All code must compile without warnings. All code must also be checked daily with at least one,
but preferably more than one, strong static source code analyzer and should pass all analyses with zero warnings.

The rule of zero warnings applies even when the compiler or the static analyzer gives an erroneous warning: If the
compiler or analyzer gets confused, the code causing the confusion should be rewritten. Many developers have been
caught in the assumption that a warning was surely invalid, only to realize much later that the message was in fact valid
for less obvious reasons. Static analyzers have a somewhat bad reputation due to early versions that produced mostly
invalid messages, but this is no longer the case. The best static analyzers today are fast, and they produce accurate
*/