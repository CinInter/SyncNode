#include <stdio.h>

int main() {
  #ifdef __arm__
  printf("Why yes it is, thank you\n");
  #endif
  printf("lol\n");
  return 0;
}