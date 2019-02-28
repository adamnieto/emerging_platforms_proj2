#ifndef __UTIL_H
#define __UTIL_H

// Ininitialize our utility library with a formula file
void init_lib(char *formulafile);
void free_lib(void);

// Turn a string representation of a formula into our encoding
formula* decode(char *str);
// Grab the next formula from the suppled formulafile, will return
// NULL when no formulas remain
formula* next_formula();

#endif
