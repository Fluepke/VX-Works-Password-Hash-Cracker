#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_PASSWORD_LEN 100
#define QUEUE_SIZE 100

// VxWorks Password Hashing Function
int loginDefaultEncrypt(char *plaintextPwIn, char *encryptedPwOut) {
    char *pcVar1;
    size_t inputPwLen;
    unsigned int i;
    int iVar2;

    iVar2 = 0;
    inputPwLen = strlen(plaintextPwIn);
    
    if ((7 < inputPwLen) && (inputPwLen = strlen(plaintextPwIn), inputPwLen < 41)) {
        i = 0;
        while (inputPwLen = strlen(plaintextPwIn), i < inputPwLen) {
            pcVar1 = plaintextPwIn + i;
            i = i + 1;
            iVar2 = iVar2 + ((int)*pcVar1 * i ^ i);
        }
        
        sprintf(encryptedPwOut, "%u", iVar2 * 0x1e3a1d5);
        
        for (i = 0; inputPwLen = strlen(encryptedPwOut), i < inputPwLen; i = i + 1) {
            if (encryptedPwOut[i] < '3') {
                encryptedPwOut[i] = encryptedPwOut[i] + '!';
            }
            if (encryptedPwOut[i] < '7') {
                encryptedPwOut[i] = encryptedPwOut[i] + '/';
            }
            if (encryptedPwOut[i] < '9') {
                encryptedPwOut[i] = encryptedPwOut[i] + 'B';
            }
        }
        
        return 0;
    }

    return -1;
}

// Thread data structure
typedef struct {
    char encryptedPassword[MAX_PASSWORD_LEN];
    char dictionaryFileName[MAX_PASSWORD_LEN];
} ThreadData;

// Shared data structure for the queue
typedef struct {
    char passwords[QUEUE_SIZE][MAX_PASSWORD_LEN];
    int front;
    int rear;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} PasswordQueue;

// Global variables
PasswordQueue passwordQueue;

// Function to add a password to the queue
void enqueue(const char *password) {
    pthread_mutex_lock(&passwordQueue.mutex);

    strcpy(passwordQueue.passwords[passwordQueue.rear], password);
    passwordQueue.rear = (passwordQueue.rear + 1) % QUEUE_SIZE;

    pthread_cond_signal(&passwordQueue.cond);
    pthread_mutex_unlock(&passwordQueue.mutex);
}

// Function to get a password from the queue
void dequeue(char *password) {
    pthread_mutex_lock(&passwordQueue.mutex);

    while (passwordQueue.front == passwordQueue.rear) {
        pthread_cond_wait(&passwordQueue.cond, &passwordQueue.mutex);
    }

    strcpy(password, passwordQueue.passwords[passwordQueue.front]);
    passwordQueue.front = (passwordQueue.front + 1) % QUEUE_SIZE;

    pthread_mutex_unlock(&passwordQueue.mutex);
}

// Function to decrypt and check password
void* tryPassword(void *arg) {
    ThreadData *data = (ThreadData*)arg;
    char line[MAX_PASSWORD_LEN];

    while (1) {
        // Get a password from the queue
        dequeue(line);

        // Try to encrypt and compare with the provided encrypted password
        char encrypted[MAX_PASSWORD_LEN];
        if (loginDefaultEncrypt(line, encrypted) == 0 && strcmp(data->encryptedPassword, encrypted) == 0) {
            printf("Password found: %s\n", line);
            pthread_exit(NULL);
        }
    }

    pthread_exit(NULL);
}

// Function for the reader thread
void* readPasswords(void *arg) {
    ThreadData *data = (ThreadData*)arg;
    char line[MAX_PASSWORD_LEN];

    FILE *file = fopen(data->dictionaryFileName, "r");
    if (file == NULL) {
        perror("Error opening dictionary file");
        pthread_exit(NULL);
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove newline character if present
        size_t len = strlen(line);
        if (line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Add the password to the queue
        enqueue(line);
    }

    fclose(file);

    // Signal worker threads that there are no more passwords
    pthread_mutex_lock(&passwordQueue.mutex);
    passwordQueue.front = passwordQueue.rear;
    pthread_cond_broadcast(&passwordQueue.cond);
    pthread_mutex_unlock(&passwordQueue.mutex);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <dictionary_file> <encrypted_password>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *dictionaryFileName = argv[1];
    char *encryptedPassword = argv[2];

    // Get the number of CPUs
    int numCPUs = sysconf(_SC_NPROCESSORS_ONLN);
    fprintf(stderr, "Number of CPUs: %d\n", numCPUs);

    pthread_t readerThread, workerThreads[numCPUs];
    ThreadData data;
    strcpy(data.dictionaryFileName, dictionaryFileName);
    strcpy(data.encryptedPassword, encryptedPassword);

    // Initialize the shared data structure
    passwordQueue.front = 0;
    passwordQueue.rear = 0;
    pthread_mutex_init(&passwordQueue.mutex, NULL);
    pthread_cond_init(&passwordQueue.cond, NULL);

    // Create the reader thread
    if (pthread_create(&readerThread, NULL, readPasswords, (void*)&data) != 0) {
        perror("Error creating reader thread");
        exit(EXIT_FAILURE);
    }

    // Create worker threads
    for (int i = 0; i < numCPUs; i++) {
        if (pthread_create(&workerThreads[i], NULL, tryPassword, (void*)&data) != 0) {
            perror("Error creating worker thread");
            exit(EXIT_FAILURE);
        }
    }

    // Join the reader thread
    if (pthread_join(readerThread, NULL) != 0) {
        perror("Error joining reader thread");
        exit(EXIT_FAILURE);
    }

    // Join worker threads
    for (int i = 0; i < numCPUs; i++) {
        if (pthread_join(workerThreads[i], NULL) != 0) {
            perror("Error joining worker thread");
            exit(EXIT_FAILURE);
        }
    }

    // Destroy the shared data structure
    pthread_mutex_destroy(&passwordQueue.mutex);
    pthread_cond_destroy(&passwordQueue.cond);

    return 0;
}

