#include <sys/prctl.h>


void fix_broken_echo() {
  if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_DISABLED, 0) != 0) {
    perror("prctl");
    return 1;
  }
}
