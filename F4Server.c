#include "F4.h"

/* Global declarations. */
int shm_matrix_id; 
int shm_names;
int shm_info;
int sem_sync;
int sem_mutex;
int * matrix_pointer;
int * shm_info_attach;
int count_sigint = 0;
char C1Symbol = ' ';
char C2Symbol = ' ';

void delete_all(){
    shmctl(shm_matrix_id, IPC_RMID, NULL);
    shmctl(shm_info, IPC_RMID, NULL);
    semctl(sem_sync, 0, IPC_RMID, NULL);
    semctl(sem_mutex, 0, IPC_RMID, NULL);
}

/// @brief To handle the errors easier
/// @param string The string to put in perror
void perror_exit_server(char * string){
    perror(string);
    delete_all();
    exit(-1);
}

/// @brief Handler for the SIGUSR1
/// @param sig The value of the signal
void sigusr1_handler(int sig){
    if (shm_info_attach[9] == C1Symbol){
        printf("%c is out of the game!\n", C1Symbol);
        printf("%c is the WINNER!", C2Symbol);
        kill(shm_info_attach[5], SIGUSR2);
    }else{
        printf("%c is out of the game!\n", C2Symbol);
        printf("%c is the WINNER!", C1Symbol);
        kill(shm_info_attach[4], SIGUSR2);
    }
    delete_all();
    exit(0);
}

/// @brief Handler for the SIGINTs
/// @param sig The value of the signal
void sigint_handler(int sig){
    if (++count_sigint == 2){ //end of game
        printf("End of Game\n");
        if (shm_info_attach != NULL){
            shm_info_attach[9] = 'C';
            if (shm_info_attach[4] != 0 && shm_info_attach[5]!=0){
                kill(shm_info_attach[4], SIGUSR1);
                kill(shm_info_attach[5], SIGUSR1);
            }
            
        }
        delete_all();
        exit(-1);
    }else if (count_sigint == 1){ //warning
        printf("Next CTRL+C will kill the game!\n");
    }
}




int main(int argc, char *argv[])
{
    /* Cheking if the number of argument is correct. */
    if (argc != 5){
        printf("Parameters Error.\n");
        printf("The corretct sinxtax should be: ./F4Server n_rows n_colums symbol1 symbol2 \n");
        return -1;
    }

    /* Getting the timer for the clients, 0 no timer. */
    int timer;
    printf("Insert the number of seconds for each turn (0 for no timer): ");
    scanf("%d", &timer);
    
    /* Getting the info from the command line and checking if they are acceptable. */
    const int N = stringToInt(argv[1]);
    if (N < 5){
        printf("Number of Rows not acceptable...\n");
        exit(-1);
    }
    const int M = stringToInt(argv[2]);
    if (M < 5){
        printf("Number of Columns not acceptable...\n");
        exit(-1);
    }

    C1Symbol = argv[3][0];
    C2Symbol = argv[4][0];

    if (signal(SIGINT, sigint_handler) == SIG_ERR){
        perror("Error Handling CTRL+C!\n");
        exit(-1);
    }

    if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR){
        perror("Error Handling SIGUSR1!\n");
        exit(-1);
    }
    

    /* Sops for the sync semaphores. */
    struct sembuf sops[3];
    /* For blocking myself. */
    sops[0].sem_num = 2;
    sops[0].sem_op = -2; // subtract 1 from server semaphore
    sops[0].sem_flg = 0;
    /* For unblocking the first client. */
    sops[1].sem_num = 0;
    sops[1].sem_op = +1; // add 1 from client1 semaphore
    sops[1].sem_flg = 0;
    /* For unblocking the second client. */
    sops[2].sem_num = 1;
    sops[2].sem_op = +1; // add 1 from client2 semaphore
    sops[2].sem_flg = 0;

    /* Creating the shm that store the matrix. */
    if ((shm_matrix_id = shmget(IPC_PRIVATE, sizeof(int) * N * M, IPC_CREAT | 0777)) == -1){
        /*perror("Matrix Shared Memory...");
        delete_all();
        exit(-1);*/
        perror_exit_server("Matrix Shared Memory...");
    }

    /* Attaching to the shm that store the matrix. */
    if ((matrix_pointer = (int *) shmat(shm_matrix_id, NULL, 0)) == NULL){
        /*perror("Matrix Shared Memory Attach...");
        delete_all();
        exit(-1);*/
        perror_exit_server("Matrix Shared Memory Attach...");
    }
    
    /* Initializing the matrix. */
    for (int i = 0; i < N; i++){
        for (int y=0; y < M; y++)
            matrix_pointer[getCoordinates(N, M, i, y)] = ' ';
    }

    printf("Inizializzata la matrice...\n");

    /* Creating the shm that store various information. */
    if ((shm_info = shmget(SHMINFO_KEY, sizeof(int) * 12, IPC_CREAT | 0777)) == -1){
        /*perror("Info Shared Memory...");
        delete_all();
        exit(-1);*/
        perror_exit_server("Info Shared Memory...");
    }

    /* Attaching to the previous shm. */
    if ((shm_info_attach = (int *) shmat(shm_info, NULL, 0)) == NULL){
        /*perror("Info Shared Memory Attach...");
        delete_all();
        exit(-1);*/
        perror_exit_server("Info Shared Memory Attach...");
    }

    /* Putting info into the shm. */
    shm_info_attach[0] = 0;
    shm_info_attach[1] = C1Symbol;
    shm_info_attach[2] = C2Symbol;
    shm_info_attach[3] = getpid();
    shm_info_attach[4] = 0;
    shm_info_attach[5] = 0;
    shm_info_attach[6] = shm_matrix_id;
    shm_info_attach[7] = N;
    shm_info_attach[8] = M;
    shm_info_attach[9] = 'D';
    shm_info_attach[10] = timer;
    shm_info_attach[11] = 0;

    printf("Inizializzata le info...\n");

    /* Creating the sem sync semaphore. */
    if ((sem_sync = semget(SEMSYNC_KEY, 3, IPC_CREAT | IPC_EXCL | 0777))== -1){
        /*perror("Semaphore Sync...");
        delete_all();
        exit(-1);*/
        perror_exit_server("Semaphore Sync...");
    }

    /* Union for setting al the semaphores previosly created. */
    union semun arg;
    short values[3] = {0, 0, 0};
    arg.array = values;

    /* Setting all the semsync semaphores. */
    if ((semctl(sem_sync, 0, SETALL, arg)) == -1){
        /*perror("Assigning Sem Sync...");
        delete_all();
        exit(-1);*/
        perror_exit_server("Assigning Sem Sync...");
    }

    printf("Settato il semsync...\n");

    /* Setting the mutex semaphore. */
    if ((sem_mutex = semget(SEMMUTEX_KEY, 1, IPC_CREAT | IPC_EXCL | 0777))== -1)
        perror_exit_server("Semaphore Mutex...");

    arg.val = 1;
    if ((semctl(sem_mutex, 0, SETVAL, arg)) == -1){
        /*perror("Assigning Sem Mutex...");
        delete_all();
        exit(-1);*/
        perror_exit_server("Assigning Sem Mutex...");
    }

    printf("Settato il semmutex...\n");
    printf("Mi blocco...\n");

    if ((semop(sem_sync, &sops[0], 1)) == -1){
        /*perror("Blocking server...");
        delete_all();
        exit(-1);*/
        perror_exit_server("Blocking Server...");
    }
    
    printf("Mi sono sbloccato, dovrebbero essere arrivati i client: %d %d %d\n", getpid(), shm_info_attach[4], shm_info_attach[5]);
    int turn;
    int win;
    while(1){
        turn = 0;
        sops[0].sem_op = -1;
        shm_info_attach[11] = 0;
        while(1){

            printf("Vai Client %d\n", (turn%2) + 1);
            if (semop(sem_sync, &sops[(turn % 2) + 1], 1) == -1){
                /*perror("Error waking the current client...");
                exit(-1);*/
                perror_exit_server("Error waking the current client...");
            }


            if (semop(sem_sync, &sops[0], 1) == -1){
                if (errno = EINTR){
                    if (semop(sem_sync, &sops[0], 1) == -1){
                        //perror("Reblocking myself...\n");
                        break;
                    }
                }
                else{
                    /*perror("Error blocking myself...");
                    exit(-1);*/
                    perror_exit_server("Error blocking myself...");
                }
                
            }

            if (!(turn%2)){  // client 0
                win = check_winner(matrix_pointer, N, M, C1Symbol);
                if ( win == 1){
                    printf("Partita terminata! Vince: %c\n", C1Symbol);
                    break;
                }else if(win == -1){
                    printf("Matrice Piena\n");
                    break;
                }
            }else {  //client 1
                win = check_winner(matrix_pointer, N, M, C2Symbol);
                if (win == 1){
                    printf("Partita terminata! Vince: %c\n", C2Symbol);
                    break;
                }else if(win == -1){
                    printf("Matrice Piena\n");
                    break;
                }
            }
            turn ++;
        }

        if (win == -1){
            shm_info_attach[9] = 'P';
        }
        else{
            if (!(turn%2))
                shm_info_attach[9] = C1Symbol;
            else 
                shm_info_attach[9] = C2Symbol;
            
        }
        kill(shm_info_attach[4], SIGUSR1);
        kill(shm_info_attach[5], SIGUSR1);

        printf("Waiting for clients...\n");

        sops[0].sem_op = -2;

        /* Blocking myself. */
        if ((semop(sem_sync, &sops[0], 1)) == -1){
            /*perror("Blocking server...");
            delete_all();
            exit(-1);*/
            perror_exit_server("Blocking Server...");
        }
        
        if(shm_info_attach[11] != 2){
            shm_info_attach[9] = 'Q';
            kill(shm_info_attach[4], SIGUSR1);
            kill(shm_info_attach[5], SIGUSR1);
            break;
        }

        /* Initializing the matrix. */
        for (int i = 0; i < N; i++){
            for (int y=0; y < M; y++)
                matrix_pointer[getCoordinates(N, M, i, y)] = ' ';
        }
        
    }

    printf("Game Over!\n");
    
    

    /*while(1){

        printf("Vai Client %d\n", (turn%2) + 1);
        if (semop(sem_sync, &sops[(turn % 2) + 1], 1) == -1){
            
            perror_exit_server("Error waking the current client...");
        }


        if (semop(sem_sync, &sops[0], 1) == -1){
            if (errno = EINTR){
                if (semop(sem_sync, &sops[0], 1) == -1){
                    //perror("Reblocking myself...\n");
                    break;
                }
            }
            else{
                perror_exit_server("Error blocking myself...");
            }
            
        }

        if (!(turn%2)){  // client 0
            win = check_winner(matrix_pointer, N, M, C1Symbol);
            if ( win == 1){
                printf("Partita terminata! Vince: %c\n", C1Symbol);
                break;
            }else if(win == -1){
                printf("Matrice Piena\n");
                break;
            }
        }else {  //client 1
            win = check_winner(matrix_pointer, N, M, C2Symbol);
            if (win == 1){
                printf("Partita terminata! Vince: %c\n", C2Symbol);
                break;
            }else if(win == -1){
                printf("Matrice Piena\n");
                break;
            }
        }

        
        turn ++;
    }*/
    
    
    


    delete_all();
    return 0;
}
