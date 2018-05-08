#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>


int main(int argc, char* argv[])
{

 if(argc !=2){
    printf("Not a suitable number of program parameters\n");
    exit(1);
 }

 int toChildFD[2];
 int toParentFD[2];

 pipe(toChildFD);
 pipe(toParentFD);

 int val1,val2,val3 = 0;

 pid_t pid;

    if((pid = fork()) == 0) {
        close(toChildFD[1]);
        close(toParentFD[0]);
        read(toChildFD[0], &val2, sizeof(int));
    //odczytaj z potoku nienazwanego wartosc przekazana przez proces macierzysty i zapisz w zmiennej val2
        val2 = val2 * val2;
        write(toParentFD[1], &val2, sizeof(int));
    //wyslij potokiem nienazwanym val2 do procesu macierzysego

 } else {
        close(toParentFD[1]);
        close(toChildFD[0]);
        val1 = atoi(argv[1]);
        write(toChildFD[1], &val1, sizeof(int));
    //wyslij val1 potokiem nienazwanym do priocesu potomnego
 
     sleep(1);
        read(toParentFD[0], &val3, sizeof(int));
    //odczytaj z potoku nienazwanego wartosc przekazana przez proces potomny i zapisz w zmiennej val3
    
     printf("%d square is: %d\n",val1, val3);
 }
 return 0;
}
