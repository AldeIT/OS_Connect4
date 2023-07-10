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

#define SHMINFO_KEY 1217 /* Shminfo. */
#define SEMSYNC_KEY 2711 /* Semsync. */
#define SEMMUTEX_KEY 7121 /* Semmutex. */
#define SECOND_SIGINT 1000
#define MATRIX_IS_FULL 2000
#define CLIENT_QUIT 3000
#define RIS_DEFAULT_VALUE 4000
#define SECONDS_BETWEEN_TURNS 1
#define MAX_SIGINT 2
#define MATRIX_DEFAULT_CHAR ' '

union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

/// @brief Prints the banner on the standard output
void print_banner();

/// @brief Converts a string into a integer
/// @param string The string to convert
/// @return The converted value
int string_to_int(char * string);

/// @brief Deletes all the ipcs
void delete_all();

/// @brief Prints informations and errors for the server
/// @param string the error that you want to print
void perror_exit_server(char *);

/// @brief Prints informations and errors for the client
/// @param string the error that you want to print
void perror_exit_client(char *);

/// @brief Given a matrix dimension and a matrix coordinates, it calculates the shifted position in an array
/// @param N Number of rows
/// @param M Number of colums
/// @param x The row of the position 
/// @param y The column of the position.
/// @return The shifted position.
int get_coordinates(int N, int M, int x, int y);


/// @brief Initializes the matrix
/// @param matrix The matrix to initialize
/// @param N The number of rows
/// @param M The number of colums
void matrix_init(int * matrix, int N, int M);

/// @brief Given a matrix and its dimensions, prints it
/// @param matrix The matrix
/// @param N The number of rows 
/// @param M The number of colums
void print_matrix(int * matrix, int N, int M);

/// @brief Try to insert the given symbol into the given colums of the matrix
/// @param matrix The matrix where to perform the insertion
/// @param N The number of rows
/// @param M The number of colums
/// @param col The colums where to insert the symbol
/// @param symbol The symbol to insert into the position
/// @return 1 if the move was successful, -1 oterwhise
int make_move(int * matrix, int N, int M, int col, char symbol);

///@brief Checks if there is a winner combination
///@param matrix The matrix to analyse
///@param n The number of rows
///@param m The number of colums
///@param symbol The symbol to look for
///@return 1 if winner, 0 otherwise
int check_winner(int *matrix, int n, int m, char symbol);

/// @brief Checks if a string is Numeric
/// @param string the string to check
/// @return 1 if numeric, 0 otherwise
int isNumeric(char* string);

/************************************
*VR471346, VR471414, VR471404
*Aldegheri Alessandro, Venturi Davide, Zerman Nicolò
*15/04/2023
*************************************/