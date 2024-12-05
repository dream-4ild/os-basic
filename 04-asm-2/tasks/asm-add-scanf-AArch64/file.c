#include <stdio.h>

extern long long add_scanf(); // Declaration of the assembly function

int main() {
    long long result = add_scanf(); // Call the assembly function
    printf("The sum is: %lld\n", result); // Output the result
    return 0;
}
