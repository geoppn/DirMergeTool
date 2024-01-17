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


typedef enum { // ENUM FOR TYPE OF ENTRIES
    FILE1 = 0,
    DIRECTORY,
    SOFT_LINK,
    HARD_LINK
}Type;

typedef struct{ // STRUCT FOR EACH ENTRY
    char path[100]; // ABSOLUTE/FULL PATH
    char relativepath[100]; // RELATIVE PATH
    char name[20]; // NAME OF ENTRY [NO PATH]
    int size; // SIZE OF ENTRY 
    time_t lastedited; // LAST EDITED TIME
    ino_t iID; // I-NODE ID FOR HARD LINKS
    char linkPointer[100]; // SOFT LINK POINTER
    Type type; // TYPE OF ENTRY
}EntryInfo;

void read_directory(const char *, EntryInfo **, int *i, int *);
void compare_directories(EntryInfo* , int , EntryInfo* , int );
int compare_files(const char* , const char* );
int directory_exists(const char *);
void copy_directory(const char* , const char* ) ;
void copy_file(const char* , const char* );
void merge_directories(EntryInfo* , int , EntryInfo* , int , const char* );