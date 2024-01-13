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
        info.iID = fileStat.st_ino;

        if (S_ISLNK(fileStat.st_mode)) {
            ssize_t len = readlink(filePath, info.linkPointer, sizeof(info.linkPointer) - 1);
            if (len != -1) {
                info.linkPointer[len] = '\0';  // Ensure null-termination
            }
        }

        // Determine the type of the entry
        if (entry->d_type == DT_REG) {
            if (fileStat.st_nlink > 1) {
                info.type = HARD_LINK; // hard link
            } else {
                info.type = FILE1; // file
            }
        } else if (entry->d_type == DT_DIR) {
            info.type = DIRECTORY; // directory
        } else if (entry->d_type == DT_LNK) {
            info.type = SOFT_LINK; // symbolic link
        }

        (*dirInfo)[*i] = info; 
        (*i)++;

        if (S_ISDIR(fileStat.st_mode)) { // IF THE ENTRY IS A DIRECTORY, RECURSIVELY CALL THE FUNCTION
            read_directory(filePath, dirInfo, i, capacity);
        } 
    }

    closedir(dir);
}
 
void compare_directories(EntryInfo* dir1Info, int size1, EntryInfo* dir2Info, int size2) {
    printf("In dirA:\n");
    for (int i = 0; i < size1; i++) {
        int found = 0;
        for (int j = 0; j < size2; j++) {
            if (strcmp(dir1Info[i].name, dir2Info[j].name) == 0 && strcmp(dir1Info[i].path, dir2Info[j].path) == 0) {
                if (dir1Info[i].type == dir2Info[j].type) {
                    if (dir1Info[i].type == FILE1 && dir1Info[i].size == dir2Info[j].size && memcmp(dir1Info[i].content, dir2Info[j].content, dir1Info[i].size) == 0) {
                        found = 1;
                    } else if (dir1Info[i].type == DIRECTORY) {
                        found = 1;
                    } else if (dir1Info[i].type == SOFT_LINK && strcmp(dir1Info[i].linkPointer, dir2Info[j].linkPointer) == 0) {
                        found = 1;
                    } else if (dir1Info[i].type == HARD_LINK && dir1Info[i].iID == dir2Info[j].iID) {
                        found = 1;
                    }
                }
                if (found) {
                    break;
                }
            }
        }
        if (!found) {
            printf("%s\n", dir1Info[i].path);
        }
    }

    printf("In dirB:\n");
    for (int i = 0; i < size2; i++) {
        int found = 0;
        for (int j = 0; j < size1; j++) {
            if (strcmp(dir2Info[i].name, dir1Info[j].name) == 0 && strcmp(dir2Info[i].path, dir1Info[j].path) == 0) {
                if (dir2Info[i].type == dir1Info[j].type) {
                    if (dir2Info[i].type == FILE1 && dir2Info[i].size == dir1Info[j].size && memcmp(dir2Info[i].content, dir1Info[j].content, dir2Info[i].size) == 0) {
                        found = 1;
                    } else if (dir2Info[i].type == DIRECTORY) {
                        found = 1;
                    } else if (dir2Info[i].type == SOFT_LINK && strcmp(dir2Info[i].linkPointer, dir1Info[j].linkPointer) == 0) {
                        found = 1;
                    } else if (dir2Info[i].type == HARD_LINK && dir2Info[i].iID == dir1Info[j].iID) {
                        found = 1;
                    }
                }
                if (found) {
                    break;
                }
            }
        }
        if (!found) {
            printf("%s\n", dir2Info[i].path);
        }
    }
}