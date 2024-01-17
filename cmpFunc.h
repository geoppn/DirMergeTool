#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

#define EXISTS    1
#define NOTEXISTS 0


typedef enum {
    FILE1 = 0,
    DIRECTORY,
    SOFT_LINK,
    HARD_LINK
}Type;

typedef struct{
    //char type; // l gia link, d gia directory, f gia file, WE COULD USE DIRENT D_TYPE
    char path[100];
    char relativepath[100];
    char name[20];
    int size;
    time_t lastedited;
    ino_t iID; // I-NODE ID FOR HARD LINKS
    char linkPointer[100]; // SOFT LINKS
    Type type;
}EntryInfo;

void read_directory(const char *dirInput, EntryInfo **dirInfo, int *i, int *capacity);
void compare_directories(EntryInfo* dir1Info, int size1, EntryInfo* dir2Info, int size2);
int compare_files(const char* file1, const char* file2);
int directory_exists(const char *path);
void merge_directories(EntryInfo* dir1Info, int size1, EntryInfo* dir2Info, int size2, const char* mergedDirName);