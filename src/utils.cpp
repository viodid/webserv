#include <sys/socket.h>
#include <unistd.h>
#include <vector>

// Takes a file descriptor and a buffer (as a vector)
// and returns the number of bytes read or -1 in case of error
size_t readFromFile(int fd, std::vector<char>& buf)
{
    const size_t BUFFER_SIZE = 1 << 12; // 4KiB
    char buf_chunk[BUFFER_SIZE];
    size_t total = 0;
    int bytes_read;
    while ((bytes_read = read(fd, buf_chunk, BUFFER_SIZE)) > 0) {
        buf.insert(buf.end(), buf_chunk, buf_chunk + bytes_read);
        total += bytes_read;
    }
    if (bytes_read == -1)
        return -1;
    return total;
}
