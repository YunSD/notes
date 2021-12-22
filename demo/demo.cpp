#include <stdio.h>

main(){
    int fahr;
    printf('A' == 65?"t":"f");
    for (fahr = 0; fahr <= 300; fahr = fahr + 20)
    {
        /* code */
        printf("%3d %6.1f\n", fahr, (5.0/9.0)*(fahr-32));
    }
    return -1; 
}
