#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "sat.h"
#include "util.h"


// Checks if a satisfies f. 
int interpret(formula* f, assignment* a){
  switch(f->conn){
    case AND:{
     /*printf("AND\n");*/
     int arg1 = interpret(f->land.f,a);
     if(f->land.next != NULL){
      int arg2 = interpret(f->land.next,a);
      return arg1 && arg2;
     }else{
      return arg1;
     }
    }
    case OR: {
     /*printf("OR1\n");*/
     int arg1 = interpret(f->lor.f1,a);
     /*printf("OR2\n");*/
     int arg2 = interpret(f->lor.f2,a);
     /*printf("OR3\n");*/
     int arg3 = interpret(f->lor.f3,a);
     return arg1 || arg2 || arg3;
    }
    case NEG:{
     /*printf("NEG\n");*/
     int arg = interpret(f->lneg.f,a);
     return !arg;
    }
    case VAR:{
      /*printf("VAR\n");*/
      printf("LITERAL: %d\n",f->lvar.lit);
      return a->map[f->lvar.lit];
    }
    default:
      return -1;
  }
}

void print_assignment_map(assignment* a){
  printf("Assignment Map: ");
  for(size_t i = 0; i < a->size; ++i){
    if(i < a->size-1){
      printf("%d ",a->map[i]);
    }else{
      printf("%d\n",a->map[i]);
    }
  }
  return;
}

void assign_values(assignment* a){
  size_t num_pos_solns = 1;
  num_pos_solns << a->size;
  printf("Num Pos Solutions: %d\n",num_pos_solns);
  
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage %s: [FORMULA-FILE]\n", argv[0]);
    exit(1);
  }

	init_lib(argv[1]);

  while (1) {
    formula *f = next_formula();
		if (f == NULL) {
			break;
		}
    assignment *a = make_assignment(f);
    assign_values(a);
    print_assignment_map(a);
    printf("Answer:%s\n",interpret(f,a) ? "True":"False");
    free_assignment(a);
    free_formula(f);
  }

	free_lib();
  
  return 0;
}
