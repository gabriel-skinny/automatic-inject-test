#include <stdio.h>

int main(int argc, char *argv[]) {

  if (argc > 2) {
    fprintf(stderr, "At least one arguments has to be provided");
  }

  return 0;
}