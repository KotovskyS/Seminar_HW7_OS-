#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

#define SHARED_MEM_NAME "/my_shared_memory"
#define BUFFER_SIZE 10

typedef struct {
    int numbers[BUFFER_SIZE];
    int head;
    int tail;
} shared_data;

int shared_mem_fd = -1;
shared_data* shared_memory = NULL;

void cleanup() {
    if (shared_memory) {
        munmap(shared_memory, sizeof(shared_data));
    }
    if (shared_mem_fd >= 0) {
        close(shared_mem_fd);
        shm_unlink(SHARED_MEM_NAME);
    }
}

void signal_handler(int signum) {
    cleanup();
    exit(0);
}

int main() {
    signal(SIGINT, signal_handler);

    shared_mem_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
    if (shared_mem_fd < 0) {
        perror("shm_open");
        return 1;
    }

    if (ftruncate(shared_mem_fd, sizeof(shared_data)) < 0) {
        perror("ftruncate");
        cleanup();
        return 1;
    }

    shared_memory = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        cleanup();
        return 1;
    }

    shared_memory->head = 0;
    shared_memory->tail = 0;

    while (1) {
        if (shared_memory->head != shared_memory->tail) {
            printf("Received number: %d\n", shared_memory->numbers[shared_memory->tail]);
            shared_memory->tail = (shared_memory->tail + 1) % BUFFER_SIZE;
        }
        usleep(100000);
    }

    cleanup();
    return 0;
}
