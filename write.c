#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>

#define KEY 23749
#define SEG_SIZE 8

int main() {
  printf("Trying to acquire semaphore\n");

  int semd = semget(KEY, 1, 0);
  if (semd != -1) {
    struct sembuf sb;
    sb.sem_num = 0;
    //sb.sem_flg = SEM_UNDO;
    sb.sem_op = -1;
    semop(semd, &sb, 1);
    printf("Acquired semaphore!\n");

    int shmd = shmget(KEY, SEG_SIZE, 0);
    if (shmd == -1) {
      printf("Shared memory brooks oopsie: %s\n", strerror(errno));
    }

    int* dat = shmat(shmd, 0, 0);
    int num_bytes = *dat;

    int fd = open("story_file", O_RDWR);
    if (fd == -1) {
      printf("Error reading file: %s\n", strerror(errno));
    }

    lseek(fd, -num_bytes, SEEK_END);
    char * buf = malloc(sizeof(char) * num_bytes);
    read(fd, buf, num_bytes);
    printf("Last addition: %s\n", buf);
    free(buf);
    printf("Your addition: ");
    fflush(stdout);

    char input[2048];
    fgets(input, 2048, stdin);
    *dat = strlen(input);

    write(fd, input, *dat);

    sb.sem_op = 1;
    semop(semd, &sb, 1);
    printf("Released semaphore!\n");

    close(fd);
  } else {
    printf("Could not acquire semaphore: %s\n", strerror(errno));
  }

  return 0;
}
