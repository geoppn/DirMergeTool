#include "cmpFunc.h"
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

void read_directory(const char *dirInput, EntryInfo **dirInfo, int *i, int *capacity) {
    DIR *dir = opendir(dirInput);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct stat fileStat;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            // SKIP . AND .. ENTRIES DUE TO LINUX ENVIROMENT
            continue;
        }

        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", dirInput, entry->d_name); // GET FULL PATH FOR I-NODE RETRIEVAL

        if (stat(filePath, &fileStat) == -1) { // USE STAT TO GET I-NODE INFO
            perror("stat");
            continue; // ERROR CHECK STAT SYS CALL
        }

        if (S_ISDIR(fileStat.st_mode)) { // IF THE ENTRY IS A DIRECTORY, RECURSIVELY CALL THE FUNCTION
            read_directory(filePath, dirInfo, i, capacity);
        } else { // IF THE ENTRY IS ANYTHING ELSE,
            // CHECK IF THE ARRAY IS FULL
            if (*i == *capacity) {
                // DOUBLE THE CAPACITY
                *capacity *= 2;

                // RESIZE THE ARRAY
                EntryInfo* temp = realloc(*dirInfo, *capacity * sizeof(EntryInfo));
                *dirInfo = temp;
            }

            EntryInfo info;
            
            strncpy(info.name, entry->d_name, sizeof(info.name));

            char *relativePath = strstr(filePath, dirInput);
            if (relativePath != NULL) {
                relativePath += strlen(dirInput);
                if (*relativePath == '/') {
                    relativePath++;
                }
            } else {
                relativePath = filePath;
            }

            strncpy(info.path, relativePath, sizeof(info.path));
            info.size = fileStat.st_size;
            info.lastedited = fileStat.st_mtime;   

            (*dirInfo)[*i] = info; 
            (*i)++;
        }
    }

    closedir(dir);
}

 // GET ALL INFO REGARDING DIRECTORY 1
    // int i=0;
    // while ((entry = readdir(dir1)) != NULL) {
    //     char filePath[1024];
    //     snprintf(filePath, sizeof(filePath), "%s/%s", dirInput1, entry->d_name); // GET FULL PATH FOR I-NODE RETRIEVAL
    //     if (stat(filePath, &fileStat) == -1) { // USE STAT TO GET I-NODE INFO
    //         perror("stat");
    //         return 1;
    //     }
    //     if (i == capacity1) {
    //         // DOUBLE THE CAPACITY
    //         capacity1 *= 2;

    //         // RESIZE THE ARRAY
    //         EntryInfo* temp = realloc(dir1Info, capacity1 * sizeof(EntryInfo));

    //         dir1Info = temp;
    //     }

    //     EntryInfo info;
    //     strncpy(info.name, entry->d_name, sizeof(info.name));

    //     char *relativePath = strstr(filePath, dirInput1); // FIND THE DIRECTORY NAME IN THE PATH,
    //     if (relativePath != NULL) { // IF FOUND,
    //         relativePath += strlen(dirInput1); // ADD THE LENGTH OF THE DIRECTORY NAME TO THE POINTER (TO IGNORE IT)
    //         if (*relativePath == '/') {
    //             relativePath++;
    //         }
    //     } else {
    //         relativePath = filePath;
    //     }

    //     strncpy(info.path, relativePath, sizeof(info.path));
    //     info.size = fileStat.st_size;
    //     info.lastedited = fileStat.st_mtime;   

    //     dir1Info[i] = info; 
    //     i++;
    // }