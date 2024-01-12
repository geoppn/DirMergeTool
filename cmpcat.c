#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

int main(int argc, char *argv[]){
    char *dirInput1 = NULL;
    char *dirInput2 = NULL;
    char *dirOutput = NULL;
    // ARGUMENT PARSING
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 2 < argc) {
            dirInput1 = argv[i + 1];
            dirInput2 = argv[i + 2];
            i += 2;
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            dirOutput = argv[i + 1];
            i++;
        }
    }

    // ARGUMENT VALIDATION
    if (dirInput1 == NULL || dirInput2 == NULL) {
        fprintf(stderr, "Invalid command line argument: -d is required\n");
        fprintf(stderr, "Usage: ./cmpcat -d DIRA DIRB [-s DIRC]\n");
        return 1;
    }
    if (dirOutput != NULL && (dirInput1 == NULL || dirInput2 == NULL)) {
        fprintf(stderr, "Invalid command line argument: -s cannot exist without -d\n");
        fprintf(stderr, "Usage: ./cmpcat -d DIRA DIRB [-s DIRC]\n");
        return 1;
    }


}