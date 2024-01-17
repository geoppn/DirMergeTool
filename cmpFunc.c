#include "cmpFunc.h"
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

int compare_files(const char* file1, const char* file2) {
    FILE* f1 = fopen(file1, "rb");
    FILE* f2 = fopen(file2, "rb");
    if (f1 == NULL || f2 == NULL) {
        if (f1 != NULL) fclose(f1);
        if (f2 != NULL) fclose(f2);
        return 0;
    }

    int result = 1;
    char ch1, ch2;
    do {
        ch1 = getc(f1);
        ch2 = getc(f2);

        if (ch1 != ch2) {
            result = 0;
            break;
        }
    } while (ch1 != EOF && ch2 != EOF);

    fclose(f1);
    fclose(f2);
    return result;
}

void read_directory(const char *dirInput, EntryInfo **dirInfo, int *i, int *capacity) {
    DIR *dir = opendir(dirInput);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct stat fileStat;
    
    static int initialDirLen = 0;
    if (initialDirLen == 0) {
        initialDirLen = strlen(dirInput);
    }

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

        char *relativePath = filePath + initialDirLen;
        strncpy(info.relativepath, relativePath, sizeof(info.path));
        printf("%s\n", info.relativepath);

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
            if (strcmp(dir1Info[i].name, dir2Info[j].name) == 0 && strcmp(dir1Info[i].relativepath, dir2Info[j].relativepath) == 0) {
                if (dir1Info[i].type == dir2Info[j].type) {
                    if (dir1Info[i].type == FILE1 && dir1Info[i].size == dir2Info[j].size && compare_files(dir1Info[i].relativepath, dir2Info[j].relativepath)) {
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
            printf("%s\n", dir1Info[i].relativepath);
        }
    }

    printf("In dirB:\n");
    for (int i = 0; i < size2; i++) {
        int found = 0;
        for (int j = 0; j < size1; j++) {
            if (strcmp(dir2Info[i].name, dir1Info[j].name) == 0 && strcmp(dir2Info[i].relativepath, dir1Info[j].relativepath) == 0) {
                if (dir2Info[i].type == dir1Info[j].type) {
                    if (dir1Info[i].type == FILE1 && dir1Info[i].size == dir2Info[j].size && compare_files(dir1Info[i].relativepath, dir2Info[j].relativepath)) {
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
            printf("%s\n", dir2Info[i].relativepath);
        }
    }
}

int directory_exists(const char *path) {
    struct stat statbuf;

    if (stat(path, &statbuf) == -1) 
    {
        // Error occurred while trying to get info about the directory
        return NOTEXISTS;
    }

    return EXISTS;
}

int file_exists(const char* path) {
    if (access(path, F_OK) != -1) {
        return EXISTS;
    } else {
        return NOTEXISTS;
    }
}

void copy_file(const char* sourcePath, const char* destinationPath) {
    char ch;
    FILE *source, *target;

    source = fopen(sourcePath, "r");
    if (source == NULL) {
        return;
    }

    target = fopen(destinationPath, "w");
    if (target == NULL) {
        fclose(source);
        return;
    }

    while ((ch = fgetc(source)) != EOF) {
        fputc(ch, target);
    }

    fclose(source);
    fclose(target);
}

void copy_directory(const char* sourcePath, const char* destinationPath) {
    DIR* dir = opendir(sourcePath);
    if (dir == NULL) {
        return;
    }

    mkdir(destinationPath, 0777);

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char source[100];
        sprintf(source, "%s/%s", sourcePath, entry->d_name);

        char destination[100];
        sprintf(destination, "%s/%s", destinationPath, entry->d_name);

        if (entry->d_type == DT_DIR) {
            copy_directory(source, destination);
        } else {
            copy_file(source, destination);
        }
    }

    closedir(dir);
}

void merge_directories(EntryInfo* dir1Info, int size1, EntryInfo* dir2Info, int size2, const char* mergedDirName) {
    if (directory_exists(mergedDirName) == NOTEXISTS) {
        mkdir(mergedDirName, 0777);
    }

    for (int i = 0; i < size1; i++) {
        char newPath[100];
        sprintf(newPath, "%s/%s", mergedDirName, dir1Info[i].relativepath);
        char newDirPath[100];
        strncpy(newDirPath, newPath, strrchr(newPath, '/') - newPath);
        mkdir(newDirPath, 0777);
        if (dir1Info[i].type == FILE1) {
            if (file_exists(newPath) == EXISTS) {
                struct stat statbuf;
                stat(newPath, &statbuf);
                if (dir1Info[i].lastedited > statbuf.st_mtime) {
                    copy_file(dir1Info[i].path, newPath);
                }
            } else {
                copy_file(dir1Info[i].path, newPath);
            }
        } else if (dir1Info[i].type == DIRECTORY) {
            copy_directory(dir1Info[i].path, newPath);
        } else if (dir1Info[i].type == SOFT_LINK || dir1Info[i].type == HARD_LINK) {
            link(dir1Info[i].path, newPath);
        }
    }

    for (int i = 0; i < size2; i++) {
        char newPath[100];
        sprintf(newPath, "%s/%s", mergedDirName, dir2Info[i].relativepath);
        char newDirPath[100];
        strncpy(newDirPath, newPath, strrchr(newPath, '/') - newPath);
        mkdir(newDirPath, 0777);
        if (dir2Info[i].type == FILE1) {
            if (file_exists(newPath) == EXISTS) {
                struct stat statbuf;
                stat(newPath, &statbuf);
                if (dir2Info[i].lastedited > statbuf.st_mtime) {
                    copy_file(dir2Info[i].path, newPath);
                }
            } else {
                copy_file(dir2Info[i].path, newPath);
            }
        } else if (dir2Info[i].type == DIRECTORY) {
            copy_directory(dir2Info[i].path, newPath);
        } else if (dir2Info[i].type == SOFT_LINK || dir2Info[i].type == HARD_LINK) {
            link(dir2Info[i].path, newPath);
        }
    }
}