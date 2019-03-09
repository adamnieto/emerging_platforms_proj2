#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "sat.h"
#include "util.h"

#include <math.h>
#include <assert.h>
#include <string.h>
#include <mpi.h>
#include <gmp.h>


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
    new_str = (char*)malloc((dest->size + (source_len+1)));
    dest->capacity = (dest->size + source_len);
  }
  /*}else{*/
    /*[>printf("IN ELSE\n");<]*/
    /*new_str = dest->str;*/
    /*strcat(new_str,source);*/
    /*return; */
  /*}*/
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

typedef struct {
  int* arr; // an array of memory segment pointers
  int size;
  int cap;
} ivec;

ivec* new_ivec(int cap) {
  ivec* iv = malloc(sizeof(ivec));
  iv->arr = malloc(sizeof(int) * cap);
  iv->size = 0;
  iv->cap = cap;
  return iv;
}

void free_ivec(ivec *iv) {
  free(iv->arr);
  free(iv);
}

void insert(ivec *iv, int val) {
  if (iv->size == iv->cap) {
    int *narr = malloc(sizeof(int) * (iv->cap * 2));
    iv->cap *= 2;
    memcpy(narr, iv->arr, (sizeof(int) * iv->size));
    free(iv->arr);
    iv->arr = narr;
  }
  iv->arr[iv->size] = val;
  iv->size++;
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

int count_num_digits(int number){
  int count = 0;
  while(number != 0){
     number = number/10;
     number /= 10;
     ++count;
  }
  return count;
}

void encode(formula *f, dynam_str* res) {
  switch (f->conn) {
    case AND:{
      encode(f->land.f, res);
      if (f->land.next != NULL) {
        strcatr(res, " /\\ ");
        encode(f->land.next, res);
      }
      break;
     }
    case OR:{
      strcatr(res,"(");
      encode(f->lor.f1,res);
      strcatr(res," \\/ ");
      encode(f->lor.f2,res);
      strcatr(res," \\/ ");
      encode(f->lor.f3,res);
      strcatr(res, ")");
      break;
    }
    case NEG:{
      strcatr(res,"!");
      encode(f->lneg.f,res);
      break;
    }
    case VAR:{
      int n = count_num_digits(f->lvar.lit);
      char* string = malloc(sizeof(char)*(n+1));
      sprintf(string, "%d",f->lvar.lit);
      strcatr(res,string);
      free(string);
      break;
    }
    default:
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

void get_case(assignment* a, size_t num, int len){
  /*assignment* a_new = a;*/
  // num represent the current case (represented in the nums binary)
  long temp = 0; // holds the bits 
  size_t counter = 0; // keeps track of the current index in of map 
  // Iterate over each bit and correct the assignment map if we find a difference
  for(long ind = len; ind >= 0; ind--){
    temp = num >> ind;
    long bool_val = (temp & 1);
    if(a->map[counter] == bool_val) continue; // means bit and array truth value are exactly the same 
    else{ 
      // Means we need to change the value in the assignment array
      if(bool_val){
        a->map[counter] = 1;
      }else{
        a->map[counter] = 0;
      }
    }
    counter += 1;
  }
  /*return a_new;*/
}

int solve(size_t start, size_t end, formula* f, assignment* a){
  for(size_t curr_num = start; curr_num < end+1; ++curr_num){
    get_case(a,curr_num,a->size);
    int res = interpret(f,a); 
    // Debugging
    /*printf("Trying:\t");*/
    /*print_assignment_map(a);*/
    /*printf("Res from Interpret: %d\n", res);*/
    if(res) return res;
    /*else a_curr = a_new;*/
  }
  /*return NULL;*/
  return 0;
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

int dedup(int *array, int length){
  int res = length;
  int *current, *end = array + length - 1;
  for (current = array + 1; array < end; array++, current = array + 1){
    while(current <= end){
      if(*current == *array ){
        *current = *end--;
        res--;
      }else{
        current++;
      }
    }
  }
  return res;
}


size_t count_num_variables_helper(formula* f, ivec* all_variables){
  switch (f->conn) {
    case AND:{
      size_t len1 = count_num_variables_helper(f->land.f, all_variables);
      if (f->land.next != NULL) {
        size_t len2 = count_num_variables_helper(f->land.next,all_variables);
        return len1 + len2;
      }
      return len1;
    }
    case OR:{
      size_t len3 = count_num_variables_helper(f->lor.f1,all_variables);
      size_t len4 = count_num_variables_helper(f->lor.f2,all_variables);
      size_t len5 = count_num_variables_helper(f->lor.f3,all_variables);
      return len3 + len4 + len5;
    }
    case NEG:{
      size_t len6 = count_num_variables_helper(f->lneg.f,all_variables);
      return len6;
    }
    case VAR:{
      insert(all_variables, f->lvar.lit);
      return 1;
    }
    default:{
      return 0;
    }

  }
}

size_t count_num_variables(formula* f){
  size_t default_cap = 10;
  ivec* all_variables = new_ivec(default_cap);
  int length = (int)count_num_variables_helper(f,all_variables);
  int res = dedup(all_variables->arr,length);
  printf("res:%d\n",res);
  free_ivec(all_variables);
  return res;
}



int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (argc < 2) {
    fprintf(stderr, "usage %s: [FORMULA-FILE]\n", argv[0]);
    exit(1);
  }

  init_lib(argv[1]);

  if(rank == 0){
    while (1) {
      formula *f = next_formula();
      if (f == NULL) {
        break;
      }
      dynam_str* encode_str = newStr("");
      encode(f,encode_str);
      printf("encode_str->str: %s\n",encode_str->str);
      assignment *a = make_assignment(f); // inital assignment struct
      // My Stuff
     int num_variables = count_num_variables(f);
      // long* lookup = (long*)malloc(sizeof(long)*);
      
      /*size_t num_combs = 1 << (a->size);*/
      printf("Size of: %lu\n",sizeof(unsigned long long));
      unsigned long long num_combs = 1;
      num_combs <<= num_variables;
      /*size_t num_workers = (size_t)rank*/
      size_t num_workers = 5; 
      /*pretty_print(f);*/
      /*printf("\n");*/
      printf("Number of Combinations: %llu\n",num_combs);

      for(size_t worker_id = num_workers; worker_id > 0; worker_id--){
        pair* worker_cases = distribute(num_combs,num_workers,worker_id);
        printf("For Worker %ld: [%ld, %ld]\n",worker_id,worker_cases->start,worker_cases->end);
        int sol = solve(worker_cases->start,worker_cases->end,f,a);
        if(sol){
          printf("ANSWER:\n");
          print_assignment_map(a);
        }
        free_pair(worker_cases);
      }
      /*printf("\n");*/
      
      free_assignment(a);
      free_dynam_str(encode_str);
      free_formula(f);
    }
  }
  else{
    printf("hello\n"); 
    /*assignment* a_sol = solve(worker_cases->start);*/

  }
  free_lib();
  MPI_Finalize();
  return 0;

}
