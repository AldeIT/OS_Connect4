#include "F4.h"


int * shm_info_attach;
char symbol;
int flag_turn_expired = 0;

void perror_exit_client(char * string){
    perror(string);
    exit(-1);
}

void sigusr1_handler(int sig){
    if (shm_info_attach != NULL){
        if (shm_info_attach[9] == 'C'){
            printf("End of the game by the server\n");
            exit(0);
        }

        if (shm_info_attach[9] == 'P'){
            printf("The Matrix is full! Draw!\n");
            exit(0);
        }

        if (shm_info_attach[9] == symbol){
            printf("I have won!\n");
            exit(0);
        }else{
            printf("I have lost!\n");
            exit(0);
        }
    }
    
}

void sigusr2_handler(int sig){
    printf("Game won!\n");
    exit(0);
}

void sigint_handler(int sig){
    if(shm_info_attach != NULL){
        if (shm_info_attach[3]!=0){
            shm_info_attach[9] = symbol;
            kill(shm_info_attach[3], SIGUSR1);
        }
    }
    printf("Game quitted...\n");
    exit(0);
}


void sigalrm_handler(int sig){
    flag_turn_expired = 1;
    printf("Your time is expired.\n");
    kill(getpid(), SIGINT); /*temporary*/
}


int main(int argc, char const *argv[])
{
    /*Cheking if the number of argument is correct.*/
    if (argc != 2){
        printf("Wrong argument error\n");
        return -1;
    }

    if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR)
        perror_exit_client("Error Handling SIGUSR1\n");
    
    if (signal(SIGALRM, sigalrm_handler) == SIG_ERR)
        perror_exit_client("Error Handling SIGALRM\n");

    if (signal(SIGUSR2, sigusr2_handler) == SIG_ERR)
        perror_exit_client("Error Handling SIGUSR2\n");

    if (signal(SIGINT, sigint_handler) == SIG_ERR || signal(SIGHUP, sigint_handler) == SIG_ERR)
        perror_exit_client("Error Handling SIGINT SIGHUP\n");

    

    int sem_mutex;
    int sem_sync;
    int shm_info;
    
    int * shm_matrix_attach;
    int shm_matrix_id;

    /* Semops for the mutex. */
    struct sembuf sops_mutex[3];
    /* For blocking the mutex. */
    sops_mutex[0].sem_num = 0;
    sops_mutex[0].sem_op = -1;
    sops_mutex[0].sem_flg = 0;
    /* For blocking the mutex. */
    sops_mutex[1].sem_num = 0;
    sops_mutex[1].sem_op = 1;
    sops_mutex[1].sem_flg = 0;

    /* Checking if the mutex semaphore exist and getting it. */
    if((sem_mutex = semget(SEMMUTEX_KEY, 0, 0)) == -1)
        perror_exit_client("Error getting the mutex");

    printf("Voglio Entrare in Sezione Critica...\n");
    /* Getting mutex. */
    if((semop(sem_mutex, &sops_mutex[0], 1)) == -1)
        perror_exit_client("Error Mutex Entry...");

    printf("Sono in sezione critica...\n");
    /* start: cs */

    if ((shm_info = shmget(SHMINFO_KEY, sizeof(int) * 11, 0)) == -1)
        perror_exit_client("Info Shared Memory...");

    
    if ((shm_info_attach = (int *) shmat(shm_info, NULL, 0)) == NULL)
        perror_exit_client("Attaching info Shared Memory...");

    /* Getting all the needed data from the shm. */
    int index = shm_info_attach[0]++; 
    symbol = shm_info_attach[index + 1];
    int server_pid = shm_info_attach[3];
    shm_info_attach[index + 4] = getpid();
    int shm_matrix = shm_info_attach[6];
    int N = shm_info_attach[7];
    int M = shm_info_attach[8];
    
    int timer = shm_info_attach[10];

    /* end: cs */

    /* Releasing mutex. */
    if((semop(sem_mutex, &sops_mutex[1], 1)) == -1)
        perror_exit_client("Error Mutex Exit...");

    /* Sops for the sync semaphores. */
    struct sembuf sops[2];
    /* For waking up server. */
    sops[0].sem_num = 2;
    sops[0].sem_op = +1;
    sops[0].sem_flg = 0;
    /* For blocking myself. */
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
    /* Attaching. */
    if ((shm_matrix_attach = (int *)shmat(shm_matrix, NULL, 0)) == NULL)
        perror_exit_client("Attaching info Shared Memory...");       

    /* Getting the sync semaphore. */
    if((sem_sync = semget(SEMSYNC_KEY, 0, 0)) == -1)
        perror_exit_client("Error getting the sem_sync..."); 

    /* Unblocking the server. */
    if((semop(sem_sync, &sops[0], 1)) == -1)
        perror_exit_client("Error waking up the server...");

    /* Blocking myself. */
    if((semop(sem_sync, &sops[1], 1)) == -1)
        perror_exit_client("Error blocking myself...");

    int col;
    /*sleep(2);
    srand(time(NULL));*/
    int move;
    while(1){
        
        printMatrix(shm_matrix_attach, N, M);
        
        do{
            flag_turn_expired = 0;
            printf("Inserisci la colonna: ");
            alarm(timer);
            //read(1, &col, 1);
            scanf("%d", &col);
            alarm(0);

            if (flag_turn_expired){
                printf("DEVO USCIRE\n");
                break;
            }
             
        }while(makeMove(shm_matrix_attach, N, M, col, symbol) == -1);

        //sleep(3);
        /*do{
            sleep(2);
            col = rand() % M;
        }while(makeMove(shm_matrix_attach, N, M, col, symbol) == -1);*/
        
        printMatrix(shm_matrix_attach, N, M);

        /* Unblocking the server. */
        if((semop(sem_sync, &sops[0], 1)) == -1)
            perror_exit_client("Error waking up the server...");

        /* Blocking myself. */
        if((semop(sem_sync, &sops[1], 1)) == -1)
            perror_exit_client("Error blocking myself...");
    }

    return 0;
}
