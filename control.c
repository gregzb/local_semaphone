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

union semun {
  int              val;    /* Value for SETVAL */
  struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
  unsigned short  *array;  /* Array for GETALL, SETALL */
  struct seminfo  *__buf;  /* Buffer for IPC_INFO
                              (Linux-specific) */
};

int main(int argc, char** argv) {

  int semd, val, shmd, fd;

  for (int i = 0; i < argc; i++) {
    char* arg = argv[i];

    if (!strcmp(arg, "-c")) {

      semd = semget(KEY, 1, IPC_CREAT | IPC_EXCL | 0644);
      if (semd == -1) {
        //printf("error %d: %s\n", errno, strerror(errno)); ERRNO is EEXIST
        semd = semget(KEY, 1, 0);
        val = semctl(semd, 0, GETVAL, 0);
        printf("Semaphore already exists with value: %d\n", val);
      }
      else {
        union semun us;
        us.val = 1;
        val = semctl(semd, 0, SETVAL, us);
        printf("Created semaphore with value: %d\n", val);

        shmd = shmget(KEY, SEG_SIZE, IPC_CREAT | IPC_EXCL | 0644);
        if (shmd == -1) {
          //shmd = shmget(KEY, SEG_SIZE, 0);
          printf("Shared memory already exists\n");
        } else {
          printf("Shared memory created\n");
        }

        fd = open("story_file", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
          printf("Error opening file: %s\n", strerror(errno));
        } else {
          printf("Created story file\n");
          close(fd);
        }
      }

      return 0;
    } else if (!strcmp(arg, "-r")) {

      semd = semget(KEY, 1, 0);
      if (semd != -1) {

        struct sembuf sb;
        sb.sem_num = 0;
        //sb.sem_flg = SEM_UNDO;
        sb.sem_op = -1;

        semop(semd, &sb, 1);
        printf("Acquired semaphore!\n");
        // sb.sem_op = 1;
        // semop(semd, &sb, 1);
        // printf("Released semaphore!\n");

        semctl(semd, 0, IPC_RMID);
        printf("Deleted semaphore\n");

        shmd = shmget(KEY, SEG_SIZE, 0);
        if (shmd != -1) {
          shmctl(shmd, 0, IPC_RMID);
          printf("Deleted shared memory\n");
        }

        if (remove("story_file") == -1) {
          printf("Error deleting file: %s\n", strerror(errno));
        } else {
          printf("Deleted story file\n");
        }
      }

      return 0;
    } else if (!strcmp(arg, "-v")) {
      fd = open("story_file", O_RDONLY);
      if (fd == -1) {
        printf("Error reading file: %s\n", strerror(errno));
      } else {
        printf("The story so far: \n");

        char buf[1024];
        int read_bytes = 0;
        while ((read_bytes = read(fd, buf, 1024)) > 0) {
          printf("%s", buf);
        }
        close(fd);
      }
      return 0;
    }
  }

  return 0;
}
