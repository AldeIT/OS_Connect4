#include "F4.h"

int main(int argc, char *argv[])
{
    /*Cheking if the number of argument is correct.*/
    if (argc != 5){
        printf("Errore nei parametri\n");
        return -1;
    }
    
    /*Getting the info from the command line.*/
    const int N = stringToInt(argv[1]);
    const int M = stringToInt(argv[2]);
    const char C1Symbol = argv[3][0];
    const char C2Symbol = argv[4][0];
    
    int shm_matrix_id = shmget(IPC_PRIVATE, sizeof(int *) * N, IPC_CREAT | 0777);
    int ** matrix_pointer = (int **) shmat(shm_matrix_id, NULL, 0);
    int columns_ids[N];
    for (int i = 0; i < N; i++){
        columns_ids[i] = shmget(IPC_PRIVATE, sizeof(int *) * M, IPC_CREAT | 0777);
        matrix_pointer[i] = (int *) shmat(columns_ids[i], NULL, 0);

    }
    for (int i = 0; i < N; i++){
        for (int y = 0; y < M; y++){
            matrix_pointer[i][y] = 0;
        }
    }

    for (int i = 0; i < N; i++){
        for (int y = 0; y < M; y++){
            printf("%d ", matrix_pointer[i][y]);
        }
        printf("\n");
    }


    //int shm_matrix_id = shmget(IPC_PRIVATE, sizeof(int) * N * M, 0777);
    //int * matrix_pointer = (int *) shmat(shm_matrix_id, NULL, 0);

    /*for (int i = 0; i < N; i++){
        for (int y=0; y < M; y++){
            matrix_pointer[getCoordinates(N, M, i, y)] = ' ';
        }
    }

    printMatrix(matrix_pointer, N, M);

    makeMove(matrix_pointer, N, M, 0, 'O');
    
    printMatrix(matrix_pointer, N, M);
    
    makeMove(matrix_pointer, N, M, 0, 'O');
    makeMove(matrix_pointer, N, M, 4, 'X');
    makeMove(matrix_pointer, N, M, 2, 'O');

    printMatrix(matrix_pointer, N, M);*/

    /*
    00 01 02 03 04 10 11 12 13 14
    ...
    
    */
   printf("%d\n", check_winner(matrix_pointer, N, M, "0"));
    

    return 0;
}
