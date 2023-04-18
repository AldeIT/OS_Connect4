#include "F4.h"


int shm_matrix_id; 
int shm_names;
int shm_info;
int sem_sync;
int sem_mutex;
int * matrix_pointer;
int * shm_info_attach;

void delete_all(){
    shmctl(shm_matrix_id, IPC_RMID, NULL);
    shmctl(shm_info, IPC_RMID, NULL);
    semctl(sem_sync, 0, IPC_RMID, NULL);
    semctl(sem_mutex, 0, IPC_RMID, NULL);
}


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
    
    /*int shm_matrix_id = shmget(IPC_PRIVATE, sizeof(int *) * N, IPC_CREAT | 0777);
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
    }*/
    struct sembuf sops[3];

    //sembuf -1 -> server
    sops[0].sem_num = 2;
    sops[0].sem_op = -2; // subtract 1 from server semaphore
    sops[0].sem_flg = 0;
    
    //sembuf +1 -> client1
    sops[1].sem_num = 0;
    sops[1].sem_op = +1; // add 1 from client1 semaphore
    sops[1].sem_flg = 0;

    //sembuf +1 -> client2
    sops[2].sem_num = 1;
    sops[2].sem_op = +1; // add 1 from client2 semaphore
    sops[2].sem_flg = 0;


    if ((shm_matrix_id = shmget(IPC_PRIVATE, sizeof(int) * N * M, IPC_CREAT | 0777)) == -1){
        perror("Matrix Shared Memory...");
        delete_all();
        exit(-1);
    }

    if ((matrix_pointer = (int *) shmat(shm_matrix_id, NULL, 0)) == NULL){
        perror("Matrix Shared Memory Attach...");
        delete_all();
        exit(-1);
    }

    for (int i = 0; i < N; i++){
        for (int y=0; y < M; y++){
            matrix_pointer[getCoordinates(N, M, i, y)] = ' ';
        }
    }


    printf("Inizializzata la matrice...\n");

    if ((shm_info = shmget(SHMINFO_KEY, sizeof(int) * 10, IPC_CREAT | 0777)) == -1){
        perror("Info Shared Memory...");
        delete_all();
        exit(-1);
    }

    if ((shm_info_attach = (int *) shmat(shm_info, NULL, 0)) == NULL){
        perror("Info Shared Memory Attach...");
        delete_all();
        exit(-1);
    }

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

    printf("Inizializzata le info...\n");

    if ((sem_sync = semget(SEMSYNC_KEY, 3, IPC_CREAT | IPC_EXCL | 0777))== -1){
        perror("Semaphore Sync...");
        delete_all();
        exit(-1);
    }

    union semun arg;
    short values[3] = {0, 0, 0};
    arg.array = values;
    if ((semctl(sem_sync, 0, SETALL, arg)) == -1){
        perror("Assigning Sem Sync...");
        delete_all();
        exit(-1);
    }

    printf("Settato il semsync...\n");

    if ((sem_mutex = semget(SEMMUTEX_KEY, 1, IPC_CREAT | IPC_EXCL | 0777))== -1){
        perror("Semaphore Mutex...");
        delete_all();
        exit(-1);
    }

    arg.val = 1;
    if ((semctl(sem_mutex, 0, SETVAL, arg)) == -1){
        perror("Assigning Sem Mutex...");
        delete_all();
        exit(-1);
    }

    printf("Settato il semmutex...\n");
    printf("Mi blocco...\n");

    if ((semop(sem_sync, &sops[0], 1)) == -1){
        perror("Blocking server...");
        delete_all();
        exit(-1);
    }
    
    printf("Mi sono sbloccato, dovrebbero essere arrivati i client: %d %d %d\n", getpid(), shm_info_attach[4], shm_info_attach[5]);
    
    /*makeMove(matrix_pointer, N, M, 0, 'O');
    makeMove(matrix_pointer, N, M, 4, 'X');
    makeMove(matrix_pointer, N, M, 2, 'O');

    printMatrix(matrix_pointer, N, M);*/
    
    int turn = 0;

    sops[0].sem_op = -1;

    while(1){

        printf("Vai Client %d\n", (turn%2) + 1);
        if (semop(sem_sync, &sops[(turn % 2) + 1], 1) == -1){
            perror("Error waking the current client...");
            exit(-1);
        }


        if (semop(sem_sync, &sops[0], 1) == -1){
            perror("Error blocking myself...");
            exit(-1);
        }

        if (!(turn%2)){  // client 0
            if (check_winner(matrix_pointer, N, M, C1Symbol) == 1){
                printf("Partita terminata! Vince: %c\n", C1Symbol);
                break;
            }else{
                printf("Matrice Piena\n");
                break;
            }
        }else {  //client 1
            if (check_winner(matrix_pointer, N, M, C2Symbol)){
                printf("Partita terminata! Vince: %c\n", C2Symbol);
                break;
            }else{
                printf("Matrice Piena\n");
                break;
            }
        }

        
        turn ++;
    }

    delete_all();
    return 0;
}
