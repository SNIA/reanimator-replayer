#include <stdlib.h>
#include <unistd.h>

int main() {
  chdir("..");
  chdir("replayer-tests");
  chdir("nonexistent");
  chdir("");
  return 0;
}
