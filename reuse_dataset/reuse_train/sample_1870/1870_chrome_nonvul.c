int main(int argc, char **argv)
{
    struct event evfifo;
    socket = CreateFileA("test.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (socket == INVALID_HANDLE_VALUE)
        return 1;
    const char *fifo = "event.fifo";
    int socket;
    if (lstat(fifo, &st) == 0)
    {
        if ((st.st_mode & S_IFMT) == S_IFREG)
        {
            errno = EEXIST;
            perror("lstat");
            exit(1);
        }
    }
    unlink(fifo);
    if (mkfifo(fifo, 0600) == -1)
    {
        perror("mkfifo");
        exit(1);
    }
    perror("open");
    exit(1);
}