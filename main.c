#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "sat.h"
#include "util.h"
#include<math.h>


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

/*void assign_values(assignment* a){*/
  /*size_t num_pos_solns = 1;*/
  /*num_pos_solns << a->size;*/
  /*printf("Num Pos Solutions: %d\n",num_pos_solns);*/
  
/*}*/

void print_binary(size_t num, size_t num_bits){
  long current;
  for(long i = num_bits; i >= 0; i--){
    current = num >> i;
    if (current & 1) printf("1");
    else printf("0");
  }
  printf("\n");
}

assignment* get_case(assignment* a, size_t num, size_t len){
  size_t current;
  for(size_t ind = len; ind >= 0; ind--){
    current = num >> ind;
    size_t bool_val = (current & 1);
    if(a->map[ind] == bool_val) continue;
    else{ 
      if(bool_val){
        a->map[ind] = 1;
      }else{
        a->map[ind] = 0;
      }
    }
  }
  return a;
}

assignment* solve(size_t start, size_t end, formula* f, assignment* a){
  assignment* a_curr = a;
  for(size_t curr_num = start; curr_num < end+1; ++curr_num){
    assignment* a_new = get_case(a_curr,curr_num,a_curr->size);
    if(interpret(f,a_new)) return a_new;
    else a_curr = a_new;
  }
  return NULL;
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
    size_t num_combs = 1 << (a->size);
    assignment* a_sol = solve(0,num_combs-1,f,a);
    print_assignment_map(a_sol);
    /*printf("Answer:%s\n",interpret(f,a) ? "True":"False");*/
    free_assignment(a);
    free_formula(f);
  }

  free_lib();
  
  return 0;
}
