#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "sat.h"
#include "util.h"
#include<math.h>


// Checks if assignment a will satisfy formula f. 
int interpret(formula* f, assignment* a){
  switch(f->conn){
    case AND:{
     int arg1 = interpret(f->land.f,a);
     if(f->land.next != NULL){
      int arg2 = interpret(f->land.next,a);
      return arg1 && arg2;
     }else{
      return arg1;
     }
    }
    case OR: {
     int arg1 = interpret(f->lor.f1,a);
     int arg2 = interpret(f->lor.f2,a);
     int arg3 = interpret(f->lor.f3,a);
     return arg1 || arg2 || arg3;
    }
    case NEG:{
     int arg = interpret(f->lneg.f,a);
     return !arg;
    }
    case VAR:{
      return a->map[f->lvar.lit];
    }
    default:
      return 0; // returns false
  }
}

void print_assignment_map(assignment* a){
  printf("\t Assignment Map: ");
  for(size_t i = 0; i < a->size; ++i){
    if(i < a->size-1){
      printf("%d ",a->map[i]);
    }else{
      printf("%d\n",a->map[i]);
    }
  }
}
// Prints the binary representation of a number and to length num_bits
void print_binary(size_t num, int num_bits){
  long temp;
  for(long i = num_bits; i >= 0; i--){
    temp = num >> i;
    if (temp & 1) printf("1");
    else printf("0");
  }
  printf("\n");
}

assignment* get_case(assignment* a, size_t num, int len){
  assignment* a_new = a;
  // num represent the current case (represented in the nums binary)
  long temp = 0; // holds the bits 
  size_t counter = 0; // keeps track of the current index in of map 
  // Iterate over each bit and correct the assignment map if we find a difference
  for(long ind = len; ind >= 0; ind--){
    temp = num >> ind;
    long bool_val = (temp & 1);
    if(a_new->map[counter] == bool_val) continue; // means bit and array truth value are exactly the same 
    else{ 
      // Means we need to change the value in the assignment array
      if(bool_val){
        a_new->map[counter] = 1;
      }else{
        a_new->map[counter] = 0;
      }
    }
    counter += 1;
  }
  return a_new;
}

assignment* solve(size_t start, size_t end, formula* f, assignment* a){
  assignment* a_curr = a;
  for(size_t curr_num = start; curr_num < end+1; ++curr_num){
    assignment* a_new = get_case(a_curr,curr_num,a_curr->size);
    int res = interpret(f,a_new); 
    // Debugging
    /*printf("Trying:\t");*/
    /*print_assignment_map(a_new);*/
    /*printf("Res from Interpret: %d\n", res);*/
    if(res) return a_new;
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
   
    // My Stuff
    size_t num_combs = 1 << (a->size);
    assignment* a_sol = solve(0,num_combs-1,f,a);
    pretty_print(f);
    print_assignment_map(a_sol);
    
    
    free_assignment(a);
    free_formula(f);
  }

  free_lib();
  
  return 0;
}
