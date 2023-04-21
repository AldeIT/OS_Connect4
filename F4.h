#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#define SHMINFO_KEY 1217 //shminfo
#define SEMSYNC_KEY 2711 //semsync
#define SEMMUTEX_KEY 7121 //semmutex
#define SECOND_SIGINT 1000
#define MATRIX_IS_FULL 2000
#define CLIENT_QUIT 3000
#define RIS_DEFAULT_VALUE 4000
#define MATRIX_DEFAULT_CHAR ' '

union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

/// @brief prints the banner on the standard output
void print_banner(){
    printf("______                    ___ \n");
    printf("|  ___|                  /   |\n");
    printf("| |_ ___  _ __ ______ _ / /| |\n");
    printf("|  _/ _ \\| '__|_  / _` / /_| |\n");
    printf("| || (_) | |   / / (_| \\___  |\n");
    printf("\\_| \\___/|_|  /___\\__,_|   |_/\n");
    printf("\n");
}

/// @brief Converts a string into a integer
/// @param string The string to convert
/// @return The converted value
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

/// @brief Deletes all the ipcs
void delete_all();

/* Can we just put these funtions here and detele them from the other files??? NO!*/
void perror_exit_server(char *);

void perror_exit_client(char *);

/// @brief Given a matrix dimension and a matrix position, it calculates the shifted position in an array
/// @param N Number of rows
/// @param M Number of colums
/// @param x The row of the position 
/// @param y The column of the position.
/// @return The shifted position.
int getCoordinates(int N, int M, int x, int y){
    return M*x + y;
}


/// @brief Initializes the matrix
/// @param matrix The matrix to initialize
/// @param N The number of rows
/// @param M The number of colums
void matrix_init(int * matrix, int N, int M){
    for (int i = 0; i < N; i++){
        for (int y = 0; y < M; y++){
            matrix[getCoordinates(N, M, i, y)] = MATRIX_DEFAULT_CHAR;
        }
    }
}

/// @brief Given a matrix and its dimensions, prints it
/// @param matrix The matrix
/// @param N The number of rows 
/// @param M The number of colums
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

/// @brief Try to insert the given symbol into the given colums of the matrix
/// @param matrix The matrix where to perform the insertion
/// @param N The number of rows
/// @param M The number of colums
/// @param col The colums where to insert the symbol
/// @param symbol The symbol to insert into the position
/// @return 1 if the move was successful, -1 oterwhise
int makeMove(int * matrix, int N, int M, int col, char symbol){
    
    col--;
    
    if (col>=M || col < 0){
        printf("Warning! Argument out of range\n");
        return -1;
    }
    int i = 0;
    while(matrix[getCoordinates(N, M, i, col)]==MATRIX_DEFAULT_CHAR){
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

///@brief Checks if there is a winner combination
///@param matrix The matrix to analyse
///@param n The number of rows
///@param m The number of colums
///@param symbol The symbol to look for
///@return 1 if winner, 0 otherwise
int check_winner(int *matrix, int n, int m, char symbol){
	/* Checking if the matrix is full. */
    int counter = 0;
    for(int i=0; i<n; i++){
        for(int y=0; y<m; y++){
            if(matrix[getCoordinates(n,m,i,y)]!=MATRIX_DEFAULT_CHAR)
                counter++;
            else
                break;
        }
    }
    if(counter == m*n)
        return -1;

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