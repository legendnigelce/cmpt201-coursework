#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  char *line[5];
  size_t size = 0;
  for (int i = 0; i < 5; i++) {
    line[i] = NULL;
  }

  int loop = 0;

  printf("Enter input: ");

  while (getline(&line[loop], &size, stdin) != -1) {
    line[loop][strcspn(line[loop], "\n")] = 0;
    if (strcmp(line[loop], "print") == 0) {
      if (loop == 4) {
        loop = 0;
      } else {
        loop++;
      }

      for (int i = 0; i < 5; i++) {
        if (line[loop] != NULL) {
          printf("%s\n", line[loop]);
        }

        if (loop == 4) {
          loop = 0;
        } else {
          loop++;
        }
      }

      if (loop == 0) {
        loop = 4;
      } else {
        loop--;
      }
    }

    if (loop == 4) {
      loop = 0;
    } else {
      loop++;
    }

    printf("Enter input: ");
  }

  for (int i = 0; i < 5; i++) {
    free(line[i]);
  }

  return 0;
}
