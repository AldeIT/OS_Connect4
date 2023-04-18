#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SHMINFO_KEY 1217 //shminfo
#define SEMSYNC_KEY 2711 //semsync
#define SEMMUTEX_KEY 7121 //semmutex

union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

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

int makeMove(int * matrix, int N, int M, int col, char symbol){
    if (col>=M){
        printf("Warning! Argument out of range\n");
        return -1;
    }
    int i = 0;
    while(matrix[getCoordinates(N, M, i, col)]==' '){
        i++;
    }

    if (i == 0){
        printf("Warning! The selected column is already full\n");
        return -1;
    }

    matrix[getCoordinates(N, M, i-1, col)] = symbol;
    /*for (int i = col * M; i < N; i ++){
        matrix[getCoordinates(N, M, i, col)] = symbol;
    }*/
    return 1;
}

///@brief a funtcion to checks if there is a winner combination
///@param matrix the matrix to analyse
///@param n the number of rows
///@param m the number of colums
///@param symbol the symbol to look for
///@return 1 if winner, 0 otherwise
int check_winner(int *matrix, int n, int m, char symbol){
	int counter = 0;
    for(int i=0; i<n; i++){
        for(int y=0; y<m; y++){
            if(matrix[getCoordinates(n,m,i,y)]!=' ')
                counter++;
            else
                break;
        }
    }
    if(counter == m*n){
        //printf("Matrix full\n");
        return -1;
    }

	/* checking the horizontal lines */
    for (int y = 0; y<m-3 ; y++ ){  //-3 because checking the 3 elements after the current
        for (int i = 0; i<n; i++){
            if (matrix[getCoordinates(n, m, i, y)] == symbol && matrix[getCoordinates(n, m, i, y+1)] == symbol && matrix[getCoordinates(n, m, i, y+2)] == symbol && matrix[getCoordinates(n, m, i, y+3)] == symbol){
                return 1;
            }           
        }
    }

	/* checking the vertical lines */
    for (int i = 0; i<n-3 ; i++ ){	//well, same as above
        for (int y = 0; y<m; y++){
            if (matrix[getCoordinates(n, m, i, y)] == symbol && matrix[getCoordinates(n, m, i+1, y)] == symbol && matrix[getCoordinates(n, m, i+2, y)] == symbol && matrix[getCoordinates(n, m, i+3, y)] == symbol){
                return 1;
            }           
        }
    }

    /* checking the ascending diagonals (/) */
    for (int i=0; i<m-3; i++){	
        for (int y=3; y<n; y++){													
            if (matrix[getCoordinates(n, m, y, i)] == symbol && matrix[getCoordinates(n, m, y-1, i+1)] == symbol && matrix[getCoordinates(n, m, y-2, i+2)] == symbol && matrix[getCoordinates(n, m, y-3, i+3)] == symbol){
                return 1;
            }
        }
    }

    /* checking the descending diagonals (\) */
    for (int i=0; i<m-3; i++){	
        for (int y=0; y<n-3; y++){
            if (matrix[getCoordinates(n, m, i, y)] == symbol && matrix[getCoordinates(n, m, i+1, y+1)] == symbol && matrix[getCoordinates(n, m, i+2, y+2)] == symbol && matrix[getCoordinates(n, m, i+3, y+3)] == symbol)
                return 1;
        }
    }
	return 0;
}