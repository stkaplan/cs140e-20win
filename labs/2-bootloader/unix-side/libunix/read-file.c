#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// read entire file into buffer.  return it.   zero pads to a 
// multiple of 4.
//
// make sure to cleanup!
uint8_t *read_file(unsigned *size, const char *name) {
    struct stat st;
    if (stat(name, &st) == -1) {
        perror("read_file: stat failed");
        return NULL;
    }

    *size = st.st_size;
    unsigned size_padding = (4 - (st.st_size % 4)) % 4;

    uint8_t *buf = malloc(st.st_size + size_padding);
    if (buf == NULL) {
        perror("read_file: malloc failed");
        return NULL;
    }

    int fd;
    if ((fd = open(name, O_RDONLY)) == -1) {
        perror("read_file: open failed");
        return NULL;
    }

    if (read(fd, buf, st.st_size) == -1) {
        close(fd);
        perror("read_file: read failed");
        return NULL;
    }
    close(fd);

    // Overwrite padding bits at end.
    for (int i = 0; i < size_padding; i++) {
        buf[st.st_size + i] = 0;
    }

    return buf;
}
