#include "F4.h"

/* Global declarations. */
int * shm_info_attach;
char symbol;
int flag_turn_expired = 0;
int flag_isover = 0;
int * shm_matrix_attach;
int N, M;
int create_son = 0;
int play_automatic = 0;
int pid_son;
int server_pid;

/// @brief To handle the errors easier
/// @param string The string to put in perror
void perror_exit_client(char * string){
    perror(string);
    exit(-1);
}


/// @brief Handler for the SIGUSR1
/// @param sig The value of the signal
void sigusr1_handler(int sig){
    if (shm_info_attach != NULL){  //if the informations were loaded

        /* If the server receives its second CTRL+C. */
        if (shm_info_attach[9] == SECOND_SIGINT){
            if (!play_automatic)
                printf("End of the game by the server\n");
            exit(0);
        }
        /* If the game is over and the matrix is full. */
        if (shm_info_attach[9] == MATRIX_IS_FULL){
            if (!play_automatic)
                printf("The Matrix is full! Draw!\n");
            flag_isover = 1;
            return;
        }

        /* If the other player doesn't wanna play anymore. */
        if (shm_info_attach[9] == CLIENT_QUIT){
            if (!create_son)
                printf("The other player doesn't want to play anymore...\n");
            exit(0);
        }

        /* Checking the symbol of the winner. */
        if (shm_info_attach[9] == symbol){
            if (!play_automatic){
                print_matrix(shm_matrix_attach, N, M);
                printf("I have won!\n");
            }
            flag_isover = 1;
        }else{
            if(!play_automatic){
                print_matrix(shm_matrix_attach, N, M);
                printf("I have lost!\n");
            }
            flag_isover = 1;
        }
    }
    
}

/// @brief Handler for SIGUSR2, if the other player presses CTRL+C
/// @param sig The value of the signal
void sigusr2_handler(int sig){
    if(!play_automatic)
        printf("Game won!\n");
    exit(0);
}

/// @brief Handler for SIGINT
/// @param sig The value of the signal
void sigint_handler(int sig){
    if(shm_info_attach != NULL){
        if (shm_info_attach[3]!=0){ /* Checking for the server pid. */
            shm_info_attach[9] = symbol;
            kill(server_pid, SIGUSR1); /* Signaling the server. */
        }
    }
    printf("Game quitted...\n");
    exit(0);
}

/// @brief Handler for SIGALRM
/// @param sig The value of the signal
void sigalarm_handler(int sig){
    flag_turn_expired = 1;
    printf("Your time is expired.\n");
    kill(getpid(), SIGINT); /* temporary */
}


int main(int argc, char const *argv[])
{
    /*Cheking if the number of argument is correct.*/  
    if (argc>=4 || argc == 1){
        printf("Wrong argument error\n");
        return -1;
    }

    /* Checking if the third argument is valid. */    
    if (argc == 3){
        if (strlen(argv[2])==1 && argv[2][0] == '*')
            create_son = 1;
        else{
            printf("Can't recognize optional argument...\n");
            exit(-1);
        }
            
    }

    /* Creating the son. */
    if (create_son){
        if ((pid_son = fork()) == -1)
            perror_exit_client("Can't create my son...");

        if (!pid_son) /* Only the son will have play_automatic = 1. */
            play_automatic = 1;
    }


    /* Handling all the signals. */
    if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR)
        perror_exit_client("Error Handling SIGUSR1\n");
    
    if (signal(SIGALRM, sigalarm_handler) == SIG_ERR)
        perror_exit_client("Error Handling SIGALRM\n");

    if (signal(SIGUSR2, sigusr2_handler) == SIG_ERR)
        perror_exit_client("Error Handling SIGUSR2\n");

    if (signal(SIGINT, sigint_handler) == SIG_ERR || signal(SIGHUP, sigint_handler) == SIG_ERR)
        perror_exit_client("Error Handling SIGINT SIGHUP\n");

    
    /* Declarations. */
    int sem_mutex;
    int sem_sync;
    int shm_info;
    char colInsert[10];

    /* Semops for the mutex. */
    struct sembuf sops_mutex[3];
    /* Used for locking the mutex. */
    sops_mutex[0].sem_num = 0;
    sops_mutex[0].sem_op  = -1;
    sops_mutex[0].sem_flg = 0;
    /* Used for unlocking the mutex. */
    sops_mutex[1].sem_num = 0;
    sops_mutex[1].sem_op  = 1;
    sops_mutex[1].sem_flg = 0;

    int sem_mutex_key = ftok("key.txt", SEMMUTEX_KEY);
    /* Checking if the mutex semaphore exist and getting it. */
    if((sem_mutex = semget(sem_mutex_key, 0, 0)) == -1)
        perror_exit_client("There is no Server...");

    /* Getting mutex. */
    if((semop(sem_mutex, &sops_mutex[0], 1)) == -1)
        perror_exit_client("Error Mutex Entry...");
        
    /* start: cs */

    int shm_info_key = ftok("key.txt", SHMINFO_KEY);
    /* Getting the shm that contains all the info. */
    if ((shm_info = shmget(shm_info_key, sizeof(int) * 12, 0)) == -1)
        perror_exit_client("Info Shared Memory...");

    /* Attaching to the above shm. */
    if ((shm_info_attach = (int *) shmat(shm_info, NULL, 0)) == NULL)
        perror_exit_client("Attaching info Shared Memory...");

    /* Getting all the needed data from the shm. */
    int index = shm_info_attach[0]++; 
    if (index == 0) write(1, "Waiting for the other player to join...\n", strlen("Waiting for the other player to join...\n"));
    
    if (index == 1 && !play_automatic && create_son){
        write(1, "Can't play with the computer when another user is already in the game!\n", strlen("Can't play with the computer when another user is already in the game!\n"));
        shm_info_attach[0]--;
        kill(pid_son, SIGKILL);
        /* Releasing mutex. */
        if((semop(sem_mutex, &sops_mutex[1], 1)) == -1)
            perror_exit_client("Error Mutex Exit...");
        exit(-1);

    }

    if (index>=2){
        printf("There are already 2 players...\n");
        shm_info_attach[0] = 2;
        /* Releasing mutex. */
        if((semop(sem_mutex, &sops_mutex[1], 1)) == -1)
            perror_exit_client("Error Mutex Exit...");
        exit(-1);
    }

    symbol                     = shm_info_attach[index + 1]; /* My symbol. */
    server_pid                 = shm_info_attach[3];         /* The pid of the server. */
    shm_info_attach[index + 4] = getpid();                   /* My pid. */
    int shm_matrix             = shm_info_attach[6];         /* SHMID to the matrix. */
    N                          = shm_info_attach[7];         /* The number of rows of the matrix. */
    M                          = shm_info_attach[8];         /* The number of colums of the matrix. */
    int timer                  = shm_info_attach[10];        /* How much time i have for my moves (0 is ignored). */

    /* end: cs */

    
    /* Releasing mutex. */
    if((semop(sem_mutex, &sops_mutex[1], 1)) == -1)
        perror_exit_client("Error Mutex Exit...");

    /* Sops for the sync semaphores. */
    struct sembuf sops[2];
    /* For waking up server. */
    sops[0].sem_num = 2;
    sops[0].sem_op  = +1;
    sops[0].sem_flg = 0;
    /* For locking myself. */
    sops[1].sem_num = index;
    sops[1].sem_op  = -1;
    sops[1].sem_flg = 0;

    /* Attaching to the shm matrix. */
    if ((shm_matrix_attach = (int *)shmat(shm_matrix, NULL, 0)) == NULL)
        perror_exit_client("Attaching info Shared Memory...");       

    int sem_sync_key = ftok("key.txt", SEMSYNC_KEY);
    /* Getting the sync semaphore. */
    if((sem_sync = semget(sem_sync_key, 0, 0)) == -1)
        perror_exit_client("Error getting the sem_sync..."); 

    /* Unlocking the server. */
    if((semop(sem_sync, &sops[0], 1)) == -1)
        perror_exit_client("Error waking up the server...");
    
    

    /* Locking myself. */
    if((semop(sem_sync, &sops[1], 1)) == -1){
        if (errno == EIDRM){
            printf("The server closed the game...\n");
            exit(0);
        }
        perror_exit_client("Error blocking myself...");
    }
        

    /* Declarations. */
    int col;
    char choice;
    
    /* Only the son will perform this. */
    if (play_automatic) 
        srand(time(NULL));

    while(1){
        flag_isover = 0;
        while(1){
            
            print_matrix(shm_matrix_attach, N, M);
            
            /* Getting the move from the player. */
            do{

                if (play_automatic){ /* Only the son will generate his move. */
                    col = (rand() % M)+1;
                    printf("I have made my move...\n");
                    sleep(SECONDS_BETWEEN_TURNS); /* You can modify this in the F4.h. */
                }
                else{
                    flag_turn_expired = 0;
                    printf("Insert the column number: ");
                    alarm(timer);  /* Giving myself timer seconds to make the move (0 is ignored). */
                    
                    scanf("%s", colInsert);
                    if (!isNumeric(colInsert)){
                        col = -1;
                    }else col = atoi(colInsert);
                    
                    alarm(0); /* Deactivating the timer */
                }

                if (flag_turn_expired){
                    break;
                }
                
            }while(make_move(shm_matrix_attach, N, M, col, symbol) == -1); /* Checking if the move is valid. */

            /* Unlocking the server. */
            if((semop(sem_sync, &sops[0], 1)) == -1)
                perror_exit_client("Error waking up the server...");

            /* Locking myself. */
            if((semop(sem_sync, &sops[1], 1)) == -1){
                if (errno != EINTR){
                    perror_exit_client("Error blocking myself...");
                }
            }  

            if (flag_isover){ /* If this game is over then break. */
                if (!play_automatic) /* Only the father will print this, to avoid chaos in the STDOUT. */
                    printf("Game Over...\n");
                break;
            }
        }

        if (!play_automatic){ /* The father will decide for both himself and the son. */
            /* Asking players if they wanna play again. */
            do{
                printf("Wanna play again? y/n: ");

                if(scanf(" %c", &choice)){
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF) {
                        choice = 'z';
                    }

                    if (choice == 'z'){
                        printf("Your choice should either be a 'y' or a 'n' ...\n");
                    }
                }

            }while(choice != 'y' && choice != 'Y' && choice != 'n' && choice != 'N');

            if(choice == 'y' || choice == 'Y'){

                if (create_son){
                    shm_info_attach[11] += 2;
                } else {
                    shm_info_attach[11] += 1;
                }
                
            }    
        }
        
        /* Unlocking the server. */
        if((semop(sem_sync, &sops[0], 1)) == -1)
            perror_exit_client("Error waking up the server...");

        if (!play_automatic){
            /* If one player or the father doesn't want to play anymore. */
            if (choice == 'n' || choice == 'N'){
                printf("Bye Bye...\n");
                exit(0);
            }
        }      

        /* Locking myself. */
        if((semop(sem_sync, &sops[1], 1)) == -1)
            perror_exit_client("Error blocking myself...");

    }

    return 0;
}


/************************************
*VR471346, VR471414, VR471404
*Aldegheri Alessandro, Venturi Davide, Zerman Nicolò
*15/04/2023
*************************************/