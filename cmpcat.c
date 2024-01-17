#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include "cmpFunc.h"

#define INITIAL_SIZE 15 // INITIAL SIZE OF DYNAMIC ARRAY

int main(int argc, char *argv[]){
    char *dirInput1 = NULL;
    char *dirInput2 = NULL;
    char *dirOutput = NULL;  // INITIALIZE INPUT VARIABLES

    int DashS = -1; // DENOTES THAT THE -s FLAG DOESNT EXIST [WE DONT USE ==NULL CHECKS, SO WE CAN PROVIDE EASE OF USE FOR THE USER IN THE FUTURE]

    // ARGUMENT PARSING WITH FLAGS IN ANY ORDER
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 2 < argc) {
            dirInput1 = argv[i + 1];
            dirInput2 = argv[i + 2];
            i += 2;
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) { // IF -s FLAG EXISTS, WE NEED TO CHECK IF THE NEXT ARGUMENT IS A DIRECTORY NAME
                dirOutput = argv[i + 1]; 
                i++;
                DashS = 1; // IF THERE IS A DIRECTORY NAME, WE SET THE FLAG TO 1
            } else {
                DashS = 0; // OTHERWISE WE SET IT TO 0
            }
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

    // MEMORY ALLOCATION FOR DYNAMIC ARRAYS TO HOLD THE ENTRIES
    EntryInfo* dir1Info = malloc(INITIAL_SIZE*sizeof(EntryInfo)); 
    EntryInfo* dir2Info = malloc(INITIAL_SIZE*sizeof(EntryInfo)); 

    int capacity1 = INITIAL_SIZE;
    int capacity2 = INITIAL_SIZE; // INITIAL CAPACITY OF DYNAMIC ARRAYS

    int size1 = 0; 
    read_directory(dirInput1,&dir1Info,&size1,&capacity1); // READ DIRECTORY 1 AND UPDATE THE ACCORDING DYNAMIC ARRAY AND OTHER VARIABLES
    
    int size2 = 0;
    read_directory(dirInput2,&dir2Info,&size2,&capacity2);

    // printf("-----------------\n");
    // printf("Directory 1:\n\n");
    // for (int j = 0; j < size1; j++) {
    //     printf("Path: %s, Name: %s, Size: %d, Last edited: %s I-Node: %lu, Soft Links: %s", dir2Info[j].path, dir2Info[j].name, dir2Info[j].size, ctime(&dir2Info[j].lastedited), dir1Info[j].iID, dir1Info[j].linkPointer);
    //     printf("Relative Path : %s\n\n", dir2Info[j].relativepath);
    // }

    // printf("-----------------\n");
    // printf("Directory 2:\n\n");
    // for (int j = 0; j < size2; j++) {
    //     printf("Path: %s, Name: %s, Size: %d, Last edited: %s I-Node: %lu, Soft Links: %s", dir2Info[j].path, dir2Info[j].name, dir2Info[j].size, ctime(&dir2Info[j].lastedited), dir1Info[j].iID, dir1Info[j].linkPointer);
    //     printf("Relative Path : %s\n\n", dir2Info[j].relativepath);
    // }

    compare_directories(dir1Info, size1, dir2Info, size2); // COMPARE THE TWO DIRECTORIES AND PRINT THE DIFFERENCES

    if (DashS == 0) // IF THE -s FLAG EXISTS BUT NO DIRECTORY NAME WAS PROVIDED, WE PROVIDE A DEFAULT NAME "DIR3"
    {
        merge_directories(dir1Info, size1, dir2Info, size2, "DIR3");
    } else if (DashS == 1) {
        merge_directories(dir1Info, size1, dir2Info, size2, dirOutput);
    }

    free(dir1Info);
    free(dir2Info); // FREE THE DYNAMIC ARRAYS
}