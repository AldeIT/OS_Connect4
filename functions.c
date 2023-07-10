#include "F4.h"

void print_banner(){
    printf("______                    ___ \n");
    printf("|  ___|                  /   |\n");
    printf("| |_ ___  _ __ ______ _ / /| |\n");
    printf("|  _/ _ \\| '__|_  / _` / /_| |\n");
    printf("| || (_) | |   / / (_| \\___  |\n");
    printf("\\_| \\___/|_|  /___\\__,_|   |_/\n");
    printf("\n");
}

int string_to_int(char * string){
    int index = 0;
    int value = 0;
    if (string[0]=='-')return -1;
    while (string[index]!='\0')
    {
        value *= 10;
        value += string[index] - 48;
        index ++;
    }

    return value;
    
}

int get_coordinates(int N, int M, int x, int y){
    return M*x + y;
}

void matrix_init(int * matrix, int N, int M){
    for (int i = 0; i < N; i++){
        for (int y = 0; y < M; y++){
            matrix[get_coordinates(N, M, i, y)] = MATRIX_DEFAULT_CHAR;
        }
    }
}

void print_matrix(int * matrix, int N, int M){

    for (int x = 0; x < M*4+1; x++){
        printf("_");
    }
    printf("\n\n");

    for (int i = 0; i < N; i++){
        for (int y = 0; y < M; y++){
            if (y == M - 1){
                printf("%c | \n", matrix[get_coordinates(N, M, i, y)]);
            }else if(y == 0){
                printf("| %c | ", matrix[get_coordinates(N, M, i, y)]);
            }else{
                printf("%c | ", matrix[get_coordinates(N, M, i, y)]);
            }
            
        }
        for (int x = 0; x < M*4+1; x++){
            printf("_");
        }
        printf("\n\n");
    }
}

int make_move(int * matrix, int N, int M, int col, char symbol){
    
    col--;
    
    if (col>=M || col < 0){
        printf("Warning! Value not acceptable\n");
        return -1;
    }
    int i = 0;
    while(matrix[get_coordinates(N, M, i, col)]==MATRIX_DEFAULT_CHAR){
        i++;
    }

    if (i == 0){
        printf("Warning! The selected column is already full\n");
        return -1;
    }

    matrix[get_coordinates(N, M, i-1, col)] = symbol;

    return 1;
}

int check_winner(int *matrix, int n, int m, char symbol){
	/* Checking if the matrix is full. */
    int counter = 0;
    for(int i=0; i<n; i++){
        for(int y=0; y<m; y++){
            if(matrix[get_coordinates(n,m,i,y)]!=MATRIX_DEFAULT_CHAR)
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
            if (matrix[get_coordinates(n, m, i, y)] == symbol && matrix[get_coordinates(n, m, i, y+1)] == symbol && matrix[get_coordinates(n, m, i, y+2)] == symbol && matrix[get_coordinates(n, m, i, y+3)] == symbol){
                return 1;
            }           
        }
    }

	/* checking the vertical lines */
    for (int i = 0; i<n-3 ; i++ ){	//well, same as above
        for (int y = 0; y<m; y++){
            if (matrix[get_coordinates(n, m, i, y)] == symbol && matrix[get_coordinates(n, m, i+1, y)] == symbol && matrix[get_coordinates(n, m, i+2, y)] == symbol && matrix[get_coordinates(n, m, i+3, y)] == symbol){
                return 1;
            }           
        }
    }

    /* checking the ascending diagonals (/) */
    for (int i=0; i<m-3; i++){	
        for (int y=3; y<n; y++){													
            if (matrix[get_coordinates(n, m, y, i)] == symbol && matrix[get_coordinates(n, m, y-1, i+1)] == symbol && matrix[get_coordinates(n, m, y-2, i+2)] == symbol && matrix[get_coordinates(n, m, y-3, i+3)] == symbol){
                return 1;
            }
        }
    }

    /* checking the descending diagonals (\) */
    for (int i=0; i<m-3; i++){	
        for (int y=0; y<n-3; y++){
            if (matrix[get_coordinates(n, m, i, y)] == symbol && matrix[get_coordinates(n, m, i+1, y+1)] == symbol && matrix[get_coordinates(n, m, i+2, y+2)] == symbol && matrix[get_coordinates(n, m, i+3, y+3)] == symbol)
                return 1;
        }
    }
	return 0;
}

int isNumeric(char* string){
    int count = 0;
    while (string[count]!='\0'){
        if (string[count] <'0' || string[count] > '9')return 0;
        count++;
    }
    return 1;
}

/************************************
*VR471346, VR471414, VR471404
*Aldegheri Alessandro, Venturi Davide, Zerman Nicolò
*15/04/2023
*************************************/