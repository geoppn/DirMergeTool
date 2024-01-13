#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include "cmpFunc.h"

#define INITIAL_SIZE 15 // INITIAL SIZE OF DYNAMIC ARRAY

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


    EntryInfo* dir1Info = malloc(INITIAL_SIZE*sizeof(EntryInfo)); 
    EntryInfo* dir2Info = malloc(INITIAL_SIZE*sizeof(EntryInfo));

    int capacity1 = INITIAL_SIZE;
    int capacity2 = INITIAL_SIZE;

    int size1 = 0;
    read_directory(dirInput1,&dir1Info,&size1,&capacity1);
    
    int size2 = 0;
    read_directory(dirInput2,&dir2Info,&size2,&capacity2);

    printf("Directory 1:\n");
    for (int j = 0; j < size1; j++) {
        printf("Path: %s, Name: %s, Size: %d, Last edited: %s INO: %lu, SL: %s\n", dir1Info[j].path, dir1Info[j].name, dir1Info[j].size, ctime(&dir1Info[j].lastedited), dir1Info[j].iID, dir1Info[j].linkPointer);
        printf("RL : %s\n", dir1Info[j].relativepath);
    }

    printf("Directory 2:\n");
    for (int j = 0; j < size2; j++) {
        printf("Path: %s, Name: %s, Size: %d, Last edited: %s INO: %lu, SL: %s\n", dir2Info[j].path, dir2Info[j].name, dir2Info[j].size, ctime(&dir2Info[j].lastedited), dir1Info[j].iID, dir1Info[j].linkPointer);
        printf("RL : %s\n", dir2Info[j].relativepath);
    }

    compare_directories(dir1Info, size1, dir2Info, size2);

    free(dir1Info);
    free(dir2Info);
}