
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

  FILE* fp = fopen("/dev/ttyS0", "r");

  if (!fp) {
    printf("Cannot open file.\n");
    exit(1);
  }

  char msg[100];

  while (true) {
    int nb = fread(msg, sizeof(msg), 1, fp);
    printf("read %d\n", nb);
    sleep(1);
  }

  return 0;
}
