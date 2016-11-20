#ifndef MINIX_H_INCLUDED
#define MINIX_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>

extern char welcome[];
extern char *prompt;
extern char blankline[];
extern char tokendel[];
extern char e_invalidparameter[];
extern char e_cannotfork[];
extern char e_cannotexecute[];
extern char e_cannotwait[];

bool verifyNoAnyParameter(char *pParam);

void quit();
void cd(char *);
void path();
void pathPlus(char *);
void pathMinus(char *);
void lineIntepreter(char *);
void changePrompt();

#endif // MINIX_H_INCLUDED
