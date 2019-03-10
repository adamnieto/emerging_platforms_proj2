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

/*========================================Pair============================================*/
typedef struct pair {
  unsigned long long start;
  unsigned long long end;
} pair;

void free_pair(pair* p){
  free(p);
}
/*====================================Dynamic String=======================================*/
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
/*====================================Dynamic Array=======================================*/
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

void print_vector(ivec* vector){
  for(size_t i = 0; i < vector->size; ++i){
    if(i == 0){
      printf("[");
    }
    if (i == vector->size-1){
      printf("%d]\n",vector->arr[i]);
      break;
    }
    printf("%d,",vector->arr[i]);
  
  }
}

dynam_str* vector_to_string(ivec* vector){
  print_vector(vector);
  dynam_str* res = newStr("");
  for(size_t i = 0; i < vector->size; ++i){
    // if(i == 0){
    //   strcatr(res,"[");
    // }
    if (i == vector->size-1){
      char* temp = (char*)malloc(sizeof(char)*64);
      sprintf(temp,"%d\n",vector->arr[i]);
      strcatr(res,temp);
      free(temp);
      break;
    }
    char* temp1 = (char*)malloc(sizeof(char)*64);
    sprintf(temp1,"%d,",vector->arr[i]);
    strcatr(res,temp1);
    free(temp1);
  }
  return res;
}

ivec* char_star_to_ivec(char* ivec_str){
  // Check
  assert(ivec_str != NULL);

  int default_cap = 10;
  ivec* res = new_ivec(default_cap);

  char* tokens = strtok(ivec_str, ",");
  while(tokens != NULL){
    int curr_element = atoi(tokens);
    insert(res,curr_element);
    tokens = strtok(NULL, ",");
  }
  return res;
}

/*====================================Worker Functions=======================================*/
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
/* Checks the binary pattern by subsituting the look up value in the map           */
/* You should see the pattern of the binary digits and the combinations it is doing*/
/* to figure out the solution:                                                     */
/*  For instance,                                                                  */ 
/*  000                                                                            */
/*  001                                                                            */
/*  010                                                                            */
/*  011                                                                            */
/*  111                                                                            */
void check_pattern(ivec* lookup, assignment* a){
  printf("LOOKUP SIZE: %d\n",lookup->size);
  for(long i = lookup->size-1; i > -1; --i){
    if(i == lookup->size-1 && i == 0) printf("[%d]\n",a->map[lookup->arr[i]]);
    else if (i == lookup->size-1) printf("[%d,",a->map[lookup->arr[i]]);
    else if (i == 0) {
      printf("%d]\n", a->map[lookup->arr[i]]);
      return;
    }else{
      printf("%d,",a->map[lookup->arr[i]]);
    }
  }
}


void get_case(assignment* a, unsigned long long num, size_t len, ivec* lookup){
  /*assignment* a_new = a;*/
  // num represent the current case (represented in the nums binary)
  unsigned long long temp = 0; // holds the bits 
  // size_t counter = 0; // keeps track of the current index in of map 
  // Iterate over each bit and correct the assignment map if we find a difference
  for(long ind = len; ind >= 0; ind--){
    temp = num;
    temp >>= ind;
    long bool_val = (temp & 1);
   
    unsigned long long key = lookup->arr[ind];
    // printf("lookup->arr[ind]: %d\n",lookup->arr[ind]);
    // printf("len: %ld\n",len);
    if(a->map[key] == bool_val) continue; // means bit and array truth value are exactly the same 
    else{ 
      // Means we need to change the value in the assignment array
      if(bool_val){
        a->map[key] = 1;
      }else{
        a->map[key] = 0;
      }
    }
    // ++counter;
  }
  check_pattern(lookup,a);
  print_assignment_map(a);
  /*return a_new;*/
}
/*====================================Sorting===============================================*/
/* Source: https://www.geeksforgeeks.org/insertion-sort/ */
/* Best sort to use if we have an already that is already sorted*/
/* Function to sort an array using insertion sort*/
// void insertion_sort(int arr[], int n) { 
//    int i, key, j; 
//    for (i = 1; i < n; i++) { 
//       key = arr[i]; 
//       j = i-1; 
//       /* Move elements of arr[0..i-1], that are 
//         greater than key, to one position ahead 
//         of their current position */
//       while (j >= 0 && arr[j] > key) { 
//           arr[j+1] = arr[j]; 
//           j = j-1; 
//       } 
//       arr[j+1] = key; 
//    } 
// } 
  
int solve(unsigned long long start, unsigned long long end, formula* f, assignment* a, ivec* lookup){
  // Make sure to sort the lookup 
  // print_vector(lookup);
  // insertion_sort(lookup->arr,lookup->size);
  // print_vector(lookup);
  for(unsigned long long curr_num = start; curr_num < end+1; ++curr_num){
    printf("Current Number: %llu\n", curr_num);
    get_case(a,curr_num,lookup->size, lookup);

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
/*====================================Master Func=======================================*/
// Gives back the start and end numbers (represents start and end of their cases for each worker) 
pair* distribute(size_t num_combs, size_t num_workers, size_t worker_id){
  // Checks
  assert(worker_id > 0); // workers ids need to start at 1
  assert(num_workers > 0);
  assert(num_combs > 0);

  unsigned long long split_len = floor((num_combs+1)/num_workers); // num_combs+1 is to include 0 as a case
  unsigned long long start = 0 + (split_len * (worker_id-1));
  unsigned long long end = start + (split_len-1);

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
/*====================================Lookup Table=======================================*/
void dedup(ivec* vector, int length){
  int* array = vector->arr;
  int new_size = length;

  int *current, *end = array + length - 1;
  for (current = array + 1; array < end; array++, current = array + 1){
    while(current <= end){
      if(*current == *array ){
        *current = *end--;
        new_size--;
      }else{
        current++;
      }
    }
  }
  vector->size = new_size;
}


void generate_lookup_table_helper(formula* f, ivec* all_variables){
  switch (f->conn) {
    case AND:{
      generate_lookup_table_helper(f->land.f, all_variables);
      if (f->land.next != NULL) {
        generate_lookup_table_helper(f->land.next,all_variables);
      }
      break;
    }
    case OR:{
      generate_lookup_table_helper(f->lor.f1,all_variables);
      generate_lookup_table_helper(f->lor.f2,all_variables);
      generate_lookup_table_helper(f->lor.f3,all_variables);
      break;
    }
    case NEG:{
      generate_lookup_table_helper(f->lneg.f,all_variables);
      break;
    }
    case VAR:{
      insert(all_variables, f->lvar.lit);
      break;
    }
    default:{
      return;
    }

  }
}

ivec* generate_lookup_table(formula* f){
  size_t default_cap = 10;
  ivec* all_variables = new_ivec(default_cap);
  generate_lookup_table_helper(f,all_variables);
  dedup(all_variables,all_variables->size);
  return all_variables;
}
/*====================================Messages=======================================*/
typedef struct message{
  unsigned long long start;
  unsigned long long end;
  formula* f;
  ivec* lookup_table;
}message;

void free_message(message* msg){
  //free_formula(msg->f);
  free_ivec(msg->lookup_table);
}

message* decode_message(char* input_string){
  message* res = (message*)malloc(sizeof(message));
  char* tokens = strtok(input_string, ":");

  int counter = 0;
  while (tokens != NULL){
    // start
    if(counter == 0){
      res->start = strtoull(tokens,NULL,10);
    }else if(counter == 1){
      res->end = strtoull(tokens,NULL,10);
    }else if(counter == 2){
      res->f = decode(tokens);
    }else{
      res->lookup_table = char_star_to_ivec(tokens);
    }
    ++counter;
    tokens = strtok(NULL, ":");
  }
  return res;

}

dynam_str* encode_message(dynam_str* formula_str, unsigned long long start_bin, 
                    unsigned long long end_bin, ivec* lookup_table){

  dynam_str* res_message = newStr("");

  char* temp1 = (char*)malloc(64*sizeof(char));
  char* temp2 = (char*)malloc(64*sizeof(char));
  
  sprintf(temp1,"%llu", start_bin); // convert start_bin to string
  strcatr(res_message, temp1);
  strcatr(res_message, ":");

  sprintf(temp2,"%llu",end_bin);// convert end_bin to string
  strcatr(res_message, temp2);
  strcatr(res_message, ":");

  strcatr(res_message,formula_str->str); // concat formula
  strcatr(res_message, ":");

  dynam_str* lookup_str = vector_to_string(lookup_table);
  strcatr(res_message,lookup_str->str);

  free(temp1);
  free(temp2);
  free_dynam_str(lookup_str);
  return res_message;
}

void decode_message_master(dynam_str* mesg){
  message* mesg_obj = decode_message(mesg->str);
  printf("Decode Message Start: %llu\n", mesg_obj->start);
  printf("Decode Message End: %llu\n", mesg_obj->end);
  dynam_str* test = newStr("");
  encode(mesg_obj->f,test);
  printf("Decode Message Formula: %s\n", test->str);  
  free_dynam_str(test);
  dynam_str* lookup_str = vector_to_string(mesg_obj->lookup_table);
  printf("Decode Message Lookup Table: %s\n",lookup_str->str);
  free_dynam_str(lookup_str);
}

/*===================================================================================*/
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
      dynam_str* encode_formula_str = newStr("");
      encode(f,encode_formula_str);
      printf("encode_formula_str->str: %s\n",encode_formula_str->str);
      // assignment *a = make_assignment(f); // inital assignment struct
      // My Stuff
      ivec* lookup_table = generate_lookup_table(f);
      print_vector(lookup_table);
      int num_variables = lookup_table->size;
      printf("Num Variables: %d\n",num_variables);
      
      unsigned long long num_combs = 1;
      num_combs <<= num_variables;
      // size_t num_workers = (size_t)rank;
      size_t num_workers = 4;

      printf("Number of Combinations: %llu\n",num_combs);
      for(size_t worker_id = 1; worker_id < num_workers+1; ++worker_id){
        pair* worker_cases = distribute(num_combs,num_workers,worker_id);
        printf("For Worker %ld: [%lld, %lld]\n",worker_id,worker_cases->start,worker_cases->end);

        
        dynam_str* mesg = encode_message(encode_formula_str,worker_cases->start,worker_cases->end, lookup_table);
        printf("Worker Encoded Message: %s\n", mesg->str);

        message* msg_obj = decode_message(mesg->str);
        // printf("Decode Message Start: %llu\n", mesg_obj->start);
        // printf("Decode Message End: %llu\n", mesg_obj->end);
        // dynam_str* test = newStr("");
        // encode(mesg_obj->f,test);
        // printf("Decode Message Formula: %s\n", test->str);  
        // free_dynam_str(test);
        // dynam_str* lookup_str = vector_to_string(mesg_obj->lookup_table);
        // printf("Decode Message Lookup Table: %s\n",lookup_str->str);
        // free_dynam_str(lookup_str);
        assignment* a = make_assignment(f);
        int sol = solve(msg_obj->start, msg_obj->end, msg_obj->f, a, msg_obj->lookup_table);
        if(sol){
          printf("ANSWER:\n");
          print_assignment_map(a);
        }else{
          printf("Case Failed\n");
        }


        free_message(msg_obj);
        free_dynam_str(mesg);
        free_pair(worker_cases);
      }
      
      // free_assignment(a);
      free_dynam_str(encode_formula_str);
      free_formula(f);
      free_ivec(lookup_table);
    }
  }
  else{
    // For WORKERS ONLY
    printf("Worker Id: %d\n",rank);
    // dynam_str* msg = newStr("");
  
    // message* msg_obj = decode_message(msg->str);
    // free_dynam_str(msg);

    // int sol = solve(msg_obj->start,msg_obj->end,msg_obj->f,a,msg->lookup_table);  
    // MPI_Recv();
    /*assignment* a_sol = solve(worker_cases->start);*/
    // int sol = solve(worker_cases->start,worker_cases->end,f,a);
    //   if(sol){
    //     printf("ANSWER:\n");
    //     print_assignment_map(a);
    //   }

  }
  free_lib();
  MPI_Finalize();
  return 0;

}
