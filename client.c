#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

#define SHARED_MEM_NAME "/my_shared_memory"
#define BUFFER_SIZE 10
#define RANDOM_RANGE 100

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
    }
}

void signal_handler(int signum) {
    cleanup();
    exit(0);
}

int main() {
    srand(time(NULL));
    signal(SIGINT, signal_handler);

    shared_mem_fd = shm_open(SHARED_MEM_NAME, O_RDWR, 0666);
    if (shared_mem_fd < 0) {
        perror("shm_open");
        return 1;
    }

    shared_memory = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        cleanup();
        return 1;
    }

    while (1) {
        int next_head = (shared_memory->head + 1) % BUFFER_SIZE;
        if (next_head != shared_memory->tail) {
            int random_number = rand() % RANDOM_RANGE;
            shared_memory->numbers[shared_memory->head] = random_number;
            shared_memory->head = next_head;
            printf("Generated number: %d\n", random_number);
        }
        sleep(1);
    }

    cleanup();
    return 0;
}
