#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define SHMINFO_KEY 1217 //shminfo

int stringToInt(char * string){
    int index = 0;
    int value = 0;
    while (string[index]!='\0')
    {
        value *= 10;
        value += string[index] - 48;
        index ++;
    }

    return value;
    
}

int getCoordinates(int N, int M, int x, int y){
    return M*x + y;
}

void printMatrix(int * matrix, int N, int M){

    for (int x = 0; x < M*4+1; x++){
        printf("_");
    }
    printf("\n\n");

    for (int i = 0; i < N; i++){
        for (int y = 0; y < M; y++){
            if (y == M - 1){
                printf("%c | \n", matrix[getCoordinates(N, M, i, y)]);
            }else if(y == 0){
                printf("| %c | ", matrix[getCoordinates(N, M, i, y)]);
            }else{
                printf("%c | ", matrix[getCoordinates(N, M, i, y)]);
            }
            
        }
        for (int x = 0; x < M*4+1; x++){
            printf("_");
        }
        printf("\n\n");
    }
}