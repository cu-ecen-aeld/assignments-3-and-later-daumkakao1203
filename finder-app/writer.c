#include <stdio.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
    openlog(NULL, 0, LOG_USER);

    if (argc != 3) {
        syslog(LOG_ERR, "Invalid Number of arguments: %d", argc);
        return 1;
    }

    const char *writefile = argv[1];
    const char *writestr = argv[2];

    FILE *file = fopen(writefile, "w");
    if (file == NULL) {
        syslog(LOG_ERR, "Error opening file: %s", writefile);
        return 1;
    }

    fprintf(file, "%s", writestr);
    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);

    fclose(file);
    closelog();

    return 0;
}
