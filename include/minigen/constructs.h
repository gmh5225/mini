#ifndef MINIGEN_CONSTRUCTS_H
#define MINIGEN_CONSTRUCTS_H

/* 
 * Types and Functions used to build an in-memory representation of 
 * a program in Minigen IR.
 */

typedef int MinigenTypeID;
typedef int MinigenProgramID;
typedef int MinigenFunctionID;

MinigenProgramID MinigenProgram();
MinigenFunctionID MinigenFunction(MinigenProgramID program, char *name);

#endif
