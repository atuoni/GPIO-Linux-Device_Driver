#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE "/dev/mygpio"

/* deve bater com o driver */
#define MYGPIO_READ   0
#define MYGPIO_WRITE  1

struct mygpio_ioctl {
    unsigned int index;
    unsigned int value;
};

static void usage(const char *p)
{
    fprintf(stderr,
        "Uso:\n"
        "  %s read  <index>\n"
        "  %s write <index> <0|1>\n\n"
        "Exemplo:\n"
        "  %s write 0 1\n"
        "  %s read  2\n",
        p, p, p, p);
    exit(1);
}

int main(int argc, char **argv)
{
    int fd;
    struct mygpio_ioctl data;

    if (argc < 3)
        usage(argv[0]);

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (!strcmp(argv[1], "read")) {

        if (argc != 3)
            usage(argv[0]);

        data.index = atoi(argv[2]);
        data.value = 0;

        if (ioctl(fd, MYGPIO_READ, &data) < 0) {
            perror("ioctl(MYGPIO_READ)");
            close(fd);
            return 1;
        }

        printf("GPIO[%u] = %u\n", data.index, data.value);

    } else if (!strcmp(argv[1], "write")) {

        if (argc != 4)
            usage(argv[0]);

        data.index = atoi(argv[2]);
        data.value = atoi(argv[3]) ? 1 : 0;

        if (ioctl(fd, MYGPIO_WRITE, &data) < 0) {
            perror("ioctl(MYGPIO_WRITE)");
            close(fd);
            return 1;
        }

    } else {
        usage(argv[0]);
    }

    close(fd);
    return 0;
}
