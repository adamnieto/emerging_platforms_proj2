#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "sat.h"
#include "util.h"

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

		// do your thing

    free_assignment(a);
    free_formula(f);
  }

	free_lib();
  
  return 0;
}
