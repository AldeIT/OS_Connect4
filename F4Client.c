#include "F4.h"

int main(int argc, char const *argv[])
{

    /*Cheking if the number of argument is correct.*/
    if (argc != 2){
        printf("Wrong argument error\n");
        return -1;
    }
    int sem_mutex;
    int sem_sync;
    int shm_info;
    int * shm_info_attach;
    int * shm_matrix_attach;
    int shm_matrix_id;
    /* semops for the mutex */
    struct sembuf sops_mutex[3];
    /* for blocking the mutex */
    sops_mutex[0].sem_num = 0;
    sops_mutex[0].sem_op = -1;
    sops_mutex[0].sem_flg = 0;
    /* for blocking the mutex */
    sops_mutex[1].sem_num = 0;
    sops_mutex[1].sem_op = 1;
    sops_mutex[1].sem_flg = 0;

    /* checking if the mutex semaphore exist and getting it */
    if((sem_mutex = semget(SEMMUTEX_KEY, 0, 0)) == -1){
        perror("Error getting the mutex");
        exit(-1);
    }
    printf("Voglio Entrare in Sezione Critica...\n");
    /* getting mutex */
    if((semop(sem_mutex, &sops_mutex[0], 1)) == -1){
        perror("Error Mutex Entry...");
        exit(-1);
    }
    printf("Sono in sezione critica...\n");
    /* start: cs */

    if ((shm_info = shmget(SHMINFO_KEY, sizeof(int) * 10, 0)) == -1){
        perror("Info Shared Memory...");
        exit(-1);
    }
    
    if ((shm_info_attach = (int *) shmat(shm_info, NULL, 0)) == NULL){
        perror("Attaching info Shared Memory...");
        exit(-1);        
    }
    /* getting all the needed data from the shm */
    int index = shm_info_attach[0]++; 
    char symbol = shm_info_attach[index + 1];
    int server_pid = shm_info_attach[3];
    shm_info_attach[index + 4] = getpid();
    int shm_matrix = shm_info_attach[6];
    int N = shm_info_attach[7];
    int M = shm_info_attach[8];

    /* end: cs*/

    /* releasing mutex */
    if((semop(sem_mutex, &sops_mutex[1], 1)) == -1){
        perror("Error Mutex Exit...");
        exit(-1);
    }

    struct sembuf sops[2];
    /* for waking up server */
    sops[0].sem_num = 2;
    sops[0].sem_op = +1;
    sops[0].sem_flg = 0;
    /* for blocking myself */
    sops[1].sem_num = index;
    sops[1].sem_op = -1;
    sops[1].sem_flg = 0;

    printf("Mio indice: %d, Simbolo: %c, Server: %d\n", index, symbol, server_pid);
    printf("Sblocco il server...\n");

    /* getting the matrix shm
    if ((shm_matrix_id = shmget(shm_matrix, sizeof(int) * N * M, 0)) == -1){
        perror("Matrix Shared Memory...");
        exit(-1);
    }*/
    /* attaching */
    if ((shm_matrix_attach = (int *)shmat(shm_matrix, NULL, 0)) == NULL){
        perror("Attaching info Shared Memory...");
        exit(-1);        
    }

    /* getting the sync semaphore */
    if((sem_sync = semget(SEMSYNC_KEY, 0, 0)) == -1){
        perror("Error getting the sem_sync");
        exit(-1);
    }

    /* unblocking the server */
    if((semop(sem_sync, &sops[0], 1)) == -1){
        perror("Error waking up the server...");
        exit(-1);
    }

    /* blocking myself */
    if((semop(sem_sync, &sops[1], 1)) == -1){
        perror("Error blocking myself...");
        exit(-1);
    }

    int col;
    srand(time(NULL));
    int move;
    while(1){
        
        printMatrix(shm_matrix_attach, N, M);
        
        /*do{
            printf("Inserisci la colonna: ");
            scanf("%d", &col);
             
        }while(makeMove(shm_matrix_attach, N, M, col, symbol) == -1);*/
        //sleep(3);
        do{

            col = rand() % M;
        }while(makeMove(shm_matrix_attach, N, M, col, symbol) == -1);
        
        
        
        printMatrix(shm_matrix_attach, N, M);

        /* unblocking the server */
        if((semop(sem_sync, &sops[0], 1)) == -1){
            perror("Error waking up the server...");
            exit(-1);
        }

        /* blocking myself */
        if((semop(sem_sync, &sops[1], 1)) == -1){
            perror("Error blocking myself...");
            exit(-1);
        }
    }

    return 0;
}
