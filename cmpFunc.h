#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>



typedef struct{
    //char type; // l gia link, d gia directory, f gia file, WE COULD USE DIRENT D_TYPE
    char path[100];
    char name[20];
    int size;
    time_t lastedited;
    // LINKS POINTER INFO
    // FILE CONTENTS
}EntryInfo;

void read_directory(const char *dirInput, EntryInfo **dirInfo, int *i, int *capacity);