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

        // CHECK IF THE ARRAY IS FULL
        if (*i == *capacity) {
            // DOUBLE THE CAPACITY
            *capacity *= 2;

            // RESIZE THE ARRAY
            EntryInfo* temp = realloc(*dirInfo, *capacity * sizeof(EntryInfo));
            *dirInfo = temp;
        }

        EntryInfo info;
        info.linkPointer[0] = '\0'; // INITIALIZE THE LINK POINTER TO NULL (IN CASE IT IS A FILE OR DIRECTORY)
        strncpy(info.name, entry->d_name, sizeof(info.name));

        strncpy(info.path, filePath, sizeof(info.path));

        info.size = fileStat.st_size;
        info.lastedited = fileStat.st_mtime;   

        if (S_ISLNK(fileStat.st_mode)) {
            ssize_t len = readlink(filePath, info.linkPointer, sizeof(info.linkPointer) - 1);
            if (len != -1) {
                info.linkPointer[len] = '\0';  // Ensure null-termination
            }
        }

        (*dirInfo)[*i] = info; 
        (*i)++;

        if (S_ISDIR(fileStat.st_mode)) { // IF THE ENTRY IS A DIRECTORY, RECURSIVELY CALL THE FUNCTION
            read_directory(filePath, dirInfo, i, capacity);
        } 
    }

    closedir(dir);
}
 