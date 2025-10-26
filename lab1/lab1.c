#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  char *buffer = NULL;
  size_t size = 0;
  char *ret;
  char *saveptr;

  if (getline(&buffer, &size, stdin) == -1) {
    perror("getline failed");
    return 1;
  }

  if ((ret = strtok_r(buffer, " ", &saveptr)) != NULL) {
    printf("Tokens:\n");
    printf("\t%s\n", ret);
  }

  while ((ret = strtok_r(NULL, " ", &saveptr)) != NULL) {
    printf("\t%s\n", ret);
  }

  free(buffer);
  return 0;
}
