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
char symbol[2] = {MATRIX_DEFAULT_CHAR, MATRIX_DEFAULT_CHAR};


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
    //delete_all();
    exit(-1);
}

/// @brief Handler for the SIGUSR1
/// @param sig The value of the signal
void sigusr1_handler(int sig){
    if (shm_info_attach[0]==2){
        if (shm_info_attach[9] == symbol[0]){  
            printf("%c is out of the game!\n", symbol[0]);  
            printf("%c is the WINNER!\n", symbol[1]);  
            kill(shm_info_attach[5], SIGUSR2);
        }
        else{
            printf("%c is out of the game!\n", symbol[1]);  
            printf("%c is the WINNER!\n", symbol[0]); 
            kill(shm_info_attach[4], SIGUSR2); 
        }
    }
    else{
        printf("The player closed the game, please restart the server...\n");
    }
    exit(0);
}

/// @brief Handler for the SIGINTs
/// @param sig The value of the signal
void sigint_handler(int sig){
    if (++count_sigint == MAX_SIGINT){ /* End of game. */
        printf("End of Game\n");
        if (shm_info_attach != NULL){
            shm_info_attach[9] = SECOND_SIGINT;
            if (shm_info_attach[4] != 0 && shm_info_attach[5]!=0){
                kill(shm_info_attach[4], SIGUSR1);
                kill(shm_info_attach[5], SIGUSR1);
            }
            
        }
        exit(-1);
    }else if (count_sigint == 1){ /* Warning. */
        printf("Next CTRL+C will kill the game!\n");
    }
}


/// @brief Handler for the SIGHUP
/// @param sig The value of the signal
void sighup_handler(int sig){
    count_sigint = MAX_SIGINT - 1;
    kill(getpid(), SIGINT);
    exit(-1);
}

/// @brief Initializes the info matrix
/// @param shm_info_attach the pointer to the matrix
/// @param N the number of rows
/// @param M the number of columns
/// @param timer the value of the timer
void shm_info_init(int * shm_info_attach, int N, int M, int timer){
    shm_info_attach[0]  = 0;
    shm_info_attach[1]  = symbol[0];
    shm_info_attach[2]  = symbol[1];
    shm_info_attach[3]  = getpid();
    shm_info_attach[4]  = 0;
    shm_info_attach[5]  = 0;
    shm_info_attach[6]  = shm_matrix_id;
    shm_info_attach[7]  = N;
    shm_info_attach[8]  = M;
    shm_info_attach[9]  = RIS_DEFAULT_VALUE;
    shm_info_attach[10] = timer;
    shm_info_attach[11] = 0;
}

/// @brief Checks if a string is Numeric
/// @param string the string to check
/// @return 1 if numeric, 0 otherwise
int isNumeric(char* string){
    int count = 0;
    while (string[count]!='\0'){
        if (string[count] <'0' || string[count] > '9')return 0;
        count++;
    }
    return 1;
}



int main(int argc, char *argv[])
{
    /* Cheking if the number of argument is correct. */
    if (argc != 5){
        printf("Parameters Error.\n");
        printf("The corretct sinxtax should be: ./F4Server n_rows n_colums symbol1 symbol2 \n");
        return -1;
    }

    

    /* Getting the info from the command line and checking if they are acceptable. */
    if(!isNumeric(argv[1])){
        printf("The number of Rows is not a number...\n");
        exit(-1);
    }
    const int N = string_to_int(argv[1]);
    if (N < 5){
        printf("Number of Rows not acceptable...\n");
        exit(-1);
    }

    if(!isNumeric(argv[2])){
        printf("The number of Columns is not a number...\n");
        exit(-1);
    }
    const int M = string_to_int(argv[2]);
    if (M < 5){
        printf("Number of Columns not acceptable...\n");
        exit(-1);
    }
    
    /* Checking if the symbols are valid. */
    if((strlen(argv[3])==1 && strlen(argv[4])==1) && (argv[3][0]!=argv[4][0])){
        symbol[0] = argv[3][0];
        symbol[1] = argv[4][0];
    }
    else{
        printf("The symbols must be 1 char each and different!\n");
        exit(-1);
    }

    /* Forza4 Logo. */
    print_banner();

    /* Setting the exit function to delete all the ipcs */
    if (atexit(delete_all) == -1){
        perror("Error setting delete_all up...");
        exit(-1);
    }

    /* Handling all the signals. */
    if (signal(SIGINT, sigint_handler) == SIG_ERR){
        perror("Error Handling CTRL+C!\n");
        exit(-1);
    }

    if (signal(SIGHUP, sighup_handler) == SIG_ERR){
        perror("Error Handling SIGHUP signal!\n");
        exit(-1);
    }

    if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR){
        perror("Error Handling SIGUSR1!\n");
        exit(-1);
    }

    /* Setting the timer for the turns. */
    int timer;
    int checkValidTimer;
    do{
        checkValidTimer = 1;
        printf("Insert the number of seconds for each turn (0 for no timer): ");


        if(!scanf(" %d", &timer)){ /*If the user inserts a string*/
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            checkValidTimer = 0;
            printf("The timer should be a number...\n");
        }

        if (timer<0){
            printf("The timer cannot be negative...\n");
            checkValidTimer = 0;
        }

    }while(!checkValidTimer);

    
    
    /* Sops for the sync semaphores. */
    struct sembuf sops[3];
    /* Used for locking operations on myself. */
    sops[0].sem_num = 2;
    sops[0].sem_op  = -2; /* Subtract 1 from server semaphore. */
    sops[0].sem_flg = 0;
    /* Used for unlocking operations on the first client. */
    sops[1].sem_num = 0;
    sops[1].sem_op  = +1; /* Add 1 from client1 semaphore. */
    sops[1].sem_flg = 0;
    /* Used for unlocking operations on the second client. */
    sops[2].sem_num = 1;
    sops[2].sem_op  = +1; /* Add 1 from client2 semaphore. */
    sops[2].sem_flg = 0;

    /* Creating the shm that store the matrix. */
    if ((shm_matrix_id = shmget(IPC_PRIVATE, sizeof(int) * N * M, IPC_CREAT | 0777 | IPC_EXCL)) == -1){
        perror_exit_server("Matrix Shared Memory...");
    }

    /* Attaching to the shm that store the matrix. */
    if ((matrix_pointer = (int *) shmat(shm_matrix_id, NULL, 0)) == NULL){
        perror_exit_server("Matrix Shared Memory Attach...");
    }

    int shm_info_key = ftok("key.txt", SHMINFO_KEY);

    /* Creating the shm that store various information. */
    if ((shm_info = shmget(shm_info_key, sizeof(int) * 12, IPC_CREAT | 0777 | IPC_EXCL)) == -1){
        perror_exit_server("A server already exists...");
    }
    
    /* Initializing the matrix. */
    matrix_init(matrix_pointer, N, M);

    printf("Initializing matrix...\n");

    

    /* Attaching to the previous shm. */
    if ((shm_info_attach = (int *) shmat(shm_info, NULL, 0)) == NULL){
        perror_exit_server("Info Shared Memory Attach...");
    }

    /* Putting info into the shm. */
    shm_info_init(shm_info_attach, N, M, timer);

    printf("Initializing infos...\n");

    int sem_sync_key = ftok("key.txt", SEMSYNC_KEY);

    /* Creating the sem sync semaphore. */
    if ((sem_sync = semget(sem_sync_key, 3, IPC_CREAT | IPC_EXCL | 0777))== -1){
        perror_exit_server("Semaphore Sync...");
    }

    /* Union for setting all the semaphores previosly created. */
    union semun arg;
    unsigned short values[3] = {0, 0, 0};
    arg.array = values;

    /* Setting all the semsync semaphores. */
    if ((semctl(sem_sync, 0, SETALL, arg)) == -1){
        perror_exit_server("Assigning Sem Sync...");
    }

    printf("Setting semsync...\n");

    int sem_mutex_key = ftok("key.txt", SEMMUTEX_KEY);

    /* Setting the mutex semaphore. */
    if ((sem_mutex = semget(sem_mutex_key, 1, IPC_CREAT | IPC_EXCL | 0777))== -1)
        perror_exit_server("Semaphore Mutex...");

    arg.val = 1;
    if ((semctl(sem_mutex, 0, SETVAL, arg)) == -1){
        perror_exit_server("Assigning Sem Mutex...");
    }

    printf("Setting semmutex...\n");
    printf("Waiting for players...\n");

    if ((semop(sem_sync, &sops[0], 1)) == -1){
        if (errno == EINTR){
                    if (semop(sem_sync, &sops[0], 1) == -1)
                        perror_exit_server("Error locking myself...");
        }else perror_exit_server("Error locking myself...");
                    
    }
    
    /* Declarations. */
    int turn;
    int win;
    
    while(1){
        turn = 0;
        sops[0].sem_op = -1;
        shm_info_attach[11] = 0;
        while(1){
            /* Unlocking the client corresponding to the current turn. */
            if (semop(sem_sync, &sops[(turn % 2) + 1], 1) == -1)
                perror_exit_server("Error waking the current client...");

            /* Locking myself and handling if a signal make the semop fails. */
            if (semop(sem_sync, &sops[0], 1) == -1){
                if (errno == EINTR){
                    if (semop(sem_sync, &sops[0], 1) == -1)
                        break;
                }
                else
                    perror_exit_server("Error locking myself...");
            }
            /* Checking if either client has won. */
            win = check_winner(matrix_pointer, N, M, symbol[turn%2]);
            if ( win == 1){
                printf("Game over! The winner is: %c\n", symbol[turn%2]);
                break;
            }else if(win == -1){
                printf("The matrix is full!\n");
                break;
            }
            
            turn ++;
        }

        if (win == -1)
            shm_info_attach[9] = MATRIX_IS_FULL;
        else
            shm_info_attach[9] = symbol[turn%2];
        
        kill(shm_info_attach[4], SIGUSR1);
        kill(shm_info_attach[5], SIGUSR1);

        printf("Waiting for clients' decision...\n");

        sops[0].sem_op = -2;

        /* Locking myself. */
        if ((semop(sem_sync, &sops[0], 1)) == -1){

            if (errno == EINTR){

                if (semop(sem_sync, &sops[0], 1) == -1){
                    perror_exit_server("Locking Server...\n");
                }

            }else{
                perror_exit_server("Locking Server...");
            }
            
        }
        
        /* Handling if one player doesn't wanna play anymore. */
        if(shm_info_attach[11] != 2){
            shm_info_attach[9] = CLIENT_QUIT;
            kill(shm_info_attach[4], SIGUSR1);
            kill(shm_info_attach[5], SIGUSR1);
            break;
        }

        /* Initializing the matrix. */
        matrix_init(matrix_pointer, N, M);
        
    }

    printf("Game Over!\n");
    
    //delete_all();
    return 0;
}


/************************************
*VR471346, VR471414, VR471404
*Aldegheri Alessandro, Venturi Davide, Zerman Nicolò
*15/04/2023
*************************************/