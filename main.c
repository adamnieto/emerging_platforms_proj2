#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "sat.h"
#include "util.h"

#include <math.h>
#include <assert.h>
#include <string.h>


typedef struct pair {
  long start;
  long end;
} pair;

void free_pair(pair* p){
  free(p);
}

typedef struct dynam_str {
  char* str; // string 
  size_t size; // bytes used not including null terminator 
  size_t capacity; // bytes allocated not including null terminator 
}dynam_str;

dynam_str* newStr(const char *str){
  dynam_str* res_str = (dynam_str*)malloc(sizeof(dynam_str) );
  res_str->size = (strlen(str))*sizeof(char);
  res_str->str = (char*)malloc(res_str->size+1);
  res_str->capacity = res_str->size;
  strcpy(res_str->str,str);
  return res_str;
}

void free_dynam_str(dynam_str* dyn_str){
  free(dyn_str->str);
  free(dyn_str);
}

void strcatr(dynam_str* dest, const char* source){
  size_t source_len = strlen(source)*sizeof(char);
  char* new_str;
  if((dest->size + source_len) > dest->capacity){
    /*printf("CAP:%ld\n",dest->capacity);*/
    new_str = (char*)malloc((dest->size + (source_len+1)*2));
    dest->capacity = (dest->size + source_len)*2;
  }else{
    /*printf("IN ELSE\n");*/
    new_str = dest->str;
    strcat(new_str,source);
    return; 
  }
  /*printf("Before\n");*/
  /*printf("dest->size: %ld\n",dest->size);*/
  /*printf("dest->capacity: %ld\n",dest->capacity);*/
  /*printf("strlen(source) : %ld\n",source_len);*/
  
  strcpy(new_str,dest->str);
  strcat(new_str,source);
  free(dest->str);
  dest->str = NULL; // out of sanity
  dest->str = new_str;
  dest->size += source_len;
  /*printf("After\n");*/
  /*printf("dest->size: %ld\n",dest->size);*/
  /*printf("dest->capacity: %ld\n",dest->capacity);*/
  /*printf("strlen(source) : %ld\n",source_len);*/
}



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

char* encode(formula *f) {
  switch (f->conn) {
    case AND:
      char* s1 = encode(f->land.f);
      if (f->land.next != NULL) {
        printf("%s"," /\\ ");
        encode(f->land.next);
      }
      break;
    case OR:
      printf("(");
      pretty_print(f->lor.f1);
      printf(" \\/ ");
      pretty_print(f->lor.f2);
      printf(" \\/ ");
      pretty_print(f->lor.f3);
      printf(")");
      break;
    case NEG:
      printf("!");
      pretty_print(f->lneg.f);
      break;
    case VAR:
      printf("%d", f->lvar.lit);
      break;
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
// Gives back the start and end numbers (represents start and end of their cases for each worker) 
pair* distribute(size_t num_combs, size_t num_workers, size_t worker_id){
  // Checks
  assert(worker_id > 0); // workers ids need to start at 1
  assert(num_workers > 0);
  assert(num_combs > 0);

  long split_len = floor((num_combs+1)/num_workers); // num_combs+1 is to include 0 as a case
  long start = 0 + (split_len * (worker_id-1));
  long end = start + (split_len-1);

  // Special Cases  

  // Will give the last worker more cases if split wasn't even
  if(num_combs - end > 0 && worker_id == num_workers){
    // if there are more cases and this is the last worker
    end = num_combs;
  }
  if(end < 0) end = 0; // keep number positive 
  if(end > num_combs) end = num_combs; // keep in domain

  pair* res = malloc(sizeof(pair));
  res->start = start;
  res->end = end;
  return res;
}



int main(int argc, char **argv) {
   /*int n = 10;*/
   
   dynam_str* dest1 = newStr("hello");
   strcatr(dest1, " please stop giving me memory leaks");
   strcatr(dest1, " I BEG");
   printf("\nNew String: %s\n",dest1->str);
   free_dynam_str(dest1);









    /*MPI_Init(&argc, &argv);*/
  
  /*int rank, size;*/
  /*MPI_Comm_rank(MPI_COMM_WORLD, &rank);*/
  /*MPI_Comm_size(MPI_COMM_WORLD, &size);*/

  /*if (argc < 2) {*/
    /*fprintf(stderr, "usage %s: [FORMULA-FILE]\n", argv[0]);*/
    /*exit(1);*/
  /*}*/

  /*init_lib(argv[1]);*/

  /*if(rank == 0){*/
    /*while (1) {*/
      /*formula *f = next_formula();*/
      /*if (f == NULL) {*/
        /*break;*/
      /*}*/
      /*assignment *a = make_assignment(f); // inital assignment struct*/
    
      /*// My Stuff*/
      /*size_t num_combs = 1 << (a->size);*/
      /*size_t num_workers = (size_t)rank;*/
      
      /*[>pretty_print(f);<]*/
      /*[>printf("\n");<]*/
      /*[>printf("Number of Combinations: %ld\n",num_combs); <]*/

      /*for(size_t worker_id = num_workers; worker_id > 0; worker_id--){*/
        /*pair* worker_cases = distribute(num_combs,num_workers,worker_id);*/
        /*[>printf("For Worker %ld: [%ld, %ld]\n",worker_id,res->start,res->end);<]*/
       
        

        /*free_pair(res);*/
      /*}*/
      /*printf("\n");*/
      
      /*[>assignment* a_sol = solve(0,num_combs-1,f,a);<]*/
      /*[>pretty_print(f);<]*/
      /*[>print_assignment_map(a_sol);<]*/
      
      /*free_assignment(a);*/
      /*free_formula(f);*/
    /*}*/
  /*}else{*/
    
    /*assignment* a_sol = solve(worker_cases->start);*/

    
  /*}*/

  /*free_lib();*/
  
  return 0;
}
