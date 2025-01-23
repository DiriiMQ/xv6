#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char*
fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}

void
find(char* path, char* name)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
    case T_DEVICE:
    case T_FILE:
        printf("find: DEBUG: %s %d %d %d\n", fmtname(path), st.type, st.ino, (int)st.size);
        if (strcmp(fmtname(path), name) == 0) {
            printf("%s\n", path);
            return;
        }
        break;

    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
            printf("find: path too long\n");
            close(fd);
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
                continue;
            }
            char new_path[512];
            strcpy(new_path, buf);

            // Check if new_path is a directory
            struct stat st;
            if (stat(new_path, &st) < 0) {
                printf("find: cannot stat %s\n", new_path);
                continue;
            }
            if (st.type == T_DIR) {
                int new_fd = open(new_path, 0);
                if (new_fd < 0) {
                    printf("find: cannot open %s\n", new_path);
                    continue;
                }
                find(new_path, name);
                close(new_fd);
            } else if (st.type == T_FILE) {
                if (strcmp(de.name, name) == 0) {
                    printf("%s\n", new_path);
                }
            }
        }
        close(fd);
        break;
    }
}

int
main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(2, "Usage: find <path> <name>\n");
        exit(1);
    }

    find(argv[1], argv[2]);
}