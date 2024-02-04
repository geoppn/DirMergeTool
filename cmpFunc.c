#include "cmpFunc.h"
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

int compare_files(const char* file1, const char* file2) { // FUNCTION TO COMPARE TWO FILES CONTENT (ASSIGNMENT REQUIREMENT)
    FILE* f1 = fopen(file1, "rb");
    FILE* f2 = fopen(file2, "rb"); // OPEN THE FILES
    if (f1 == NULL || f2 == NULL) {
        if (f1 != NULL) fclose(f1);
        if (f2 != NULL) fclose(f2);
        return 0;
    }

    int result = 1;
    char ch1, ch2;
    do { // COMPARE CHARACTER BY CHARACTER
        ch1 = getc(f1);
        ch2 = getc(f2);

        if (ch1 != ch2) {
            result = 0;
            break;
        }
    } while (ch1 != EOF && ch2 != EOF);

    fclose(f1);
    fclose(f2); // CLOSE THE FILES
    return result;
}

void read_directory(const char *dirInput, EntryInfo **dirInfo, int *i, int *capacity) { // FUNCTION TO READ A DIRECTORY AND STORE THE INFO IN AN ARRAY OF  A STRUCT WITH ALL NECESSARY INFO
    DIR *dir = opendir(dirInput);  // OPEN THE DIRECTORY
    if (dir == NULL) {
        perror("opendir");
        return;
    } // ERROR CHECK OPEN DIR SYS CALL

    struct dirent *entry; // USE THE DIRENT LIBRARY TO READ THE DIRECTORY
    struct stat fileStat; // I-NODE INFO
    
    static int initialDirLen = 0;
    if (initialDirLen == 0) {
        initialDirLen = strlen(dirInput);
    } // STORE INITIAL DIR. NAME LENGTH ON THE FIRST CALL FOR RELATIVE PATH RETRIEVAL IN THE FUTURE

    while ((entry = readdir(dir)) != NULL) { // READ THE DIRECTORY [WHILE ENTRIES STILL EXIST]
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

        EntryInfo info; // CREATE A STRUCT TO STORE THE FINALIZED INFO FOR EACH ENTRY
        info.linkPointer[0] = '\0'; // INITIALIZE THE LINK POINTER TO NULL (IN CASE IT IS A FILE OR DIRECTORY)
        strncpy(info.name, entry->d_name, sizeof(info.name));

        strncpy(info.path, filePath, sizeof(info.path));

        char *relativePath = filePath + initialDirLen; // CALCULATE THE RELATIVE PATH BY REMOVING THE ROOT DIR. NAME
        strncpy(info.relativepath, relativePath, sizeof(info.path));
        

        info.size = fileStat.st_size;
        info.lastedited = fileStat.st_mtime;   
        info.iID = fileStat.st_ino;

        if (S_ISLNK(fileStat.st_mode)) { // IF THE ENTRY IS A (SOFT) LINK, GET THE LINK POINTER
            ssize_t len = readlink(filePath, info.linkPointer, sizeof(info.linkPointer) - 1);
            if (len != -1) {
                info.linkPointer[len] = '\0';  // NULL TERMINATE THE LINK POINTER [TO AVOID ERRORS]
            }
        }

        // CHECK THE TYPE OF THE ENTRY, STORE FOR FUTURE EASE OF ACCESS
        if (entry->d_type == DT_REG) {
            if (fileStat.st_nlink > 1) {
                info.type = HARD_LINK; // HARD LINK
            } else {
                info.type = FILE1; // FILE, "FILE" IS A RESERVED WORD
            }
        } else if (entry->d_type == DT_DIR) {
            info.type = DIRECTORY; // DIRECTORY
        } else if (entry->d_type == DT_LNK) {
            info.type = SOFT_LINK; // SOFT/SYMBOLIC LINK
        }
        // STORE THE INFO IN THE ARRAY ACCORDINGLY
        (*dirInfo)[*i] = info; 
        (*i)++;

        if (S_ISDIR(fileStat.st_mode)) { // IF THE ENTRY IS A DIRECTORY, RECURSIVELY CALL THE FUNCTION
            read_directory(filePath, dirInfo, i, capacity);
        } 
    }

    closedir(dir);
}
 
void compare_directories(EntryInfo* dir1Info, int size1, EntryInfo* dir2Info, int size2) { // FUNCTION TO COMPARE TWO DIRECTORIES AND PRINT THEIR ACCORDING DIFFERENCES
    printf("In dirA:\n");
    for (int i = 0; i < size1; i++) { // ITERATE FOR EACH ENTRY IN THE FIRST DIRECTORY
        int found = 0;
        for (int j = 0; j < size2; j++) { // ITERATE FOR EACH ENTRY IN THE SECOND DIRECTORY
            if (strcmp(dir1Info[i].name, dir2Info[j].name) == 0 && strcmp(dir1Info[i].relativepath, dir2Info[j].relativepath) == 0) { // CHECK IF THE ENTRIES HAVE THE SAME NAME AND RELATIVE PATH [REQUIREMENT FOR ALL TYPES]
                if (dir1Info[i].type == dir2Info[j].type) { // CHECK IF THE ENTRIES HAVE THE SAME TYPE
                    if (dir1Info[i].type == FILE1 && dir1Info[i].size == dir2Info[j].size && compare_files(dir1Info[i].relativepath, dir2Info[j].relativepath)) { // CHECK EXTRA REQUIREMENTS FOR FILES
                        found = 1;
                    } else if (dir1Info[i].type == DIRECTORY) {
                        found = 1;
                    } else if (dir1Info[i].type == SOFT_LINK && strcmp(dir1Info[i].linkPointer, dir2Info[j].linkPointer) == 0) { // CHECK EXTRA REQUIREMENTS FOR SOFT LINKS
                        found = 1;
                    } else if (dir1Info[i].type == HARD_LINK && dir1Info[i].iID == dir2Info[j].iID) { // CHECK EXTRA REQUIREMENTS FOR HARD LINKS
                        found = 1;
                    }
                }
                if (found) { 
                    break;
                }
            }
        }
        if (!found) { // IF THE ENTRY IS NOT FOUND IN THE SECOND DIRECTORY, PRINT IT
            printf("%s\n", dir1Info[i].relativepath); 
        }
    }
    // REPEAT THE SAME PROCESS FOR THE SECOND DIRECTORY
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

int directory_exists(const char *path) { // FUNCTION TO CHECK IF A DIRECTORY BY THE NAME/PATH PROVIDED ALREADY EXISTS
    struct stat statbuf; 

    if (stat(path, &statbuf) == -1) 
    {
        return NOTEXISTS;
    }

    return EXISTS;
}

int file_exists(const char* path) { // FUNCTION TO CHECK IF A FILE BY THE NAME/PATH PROVIDED ALREADY EXISTS
    if (access(path, F_OK) != -1) {
        return EXISTS;
    } else {
        return NOTEXISTS;
    }
}

void copy_file(const char* sourcePath, const char* destinationPath) { // FUNCTION TO COPY A FILE FROM A SOURCE TO A DESTINATION
    char ch;
    FILE *source, *target; // FILE POINTERS

    source = fopen(sourcePath, "r"); // OPEN THE SOURCE FILE IN READ MODE
    if (source == NULL) {
        return;
    }

    target = fopen(destinationPath, "w"); // OPEN THE DESTINATION FILE IN WRITE MODE
    if (target == NULL) {
        fclose(source);
        return;
    }

    while ((ch = fgetc(source)) != EOF) { // COPY THE CONTENTS OF THE SOURCE FILE TO THE DESTINATION FILE
        fputc(ch, target);
    }

    fclose(source);
    fclose(target); // CLOSE THE FILES
}

void copy_directory(const char* sourcePath, const char* destinationPath) { // FUNCTION TO COPY A DIRECTORY FROM A SOURCE TO A DESTINATION
    DIR* dir = opendir(sourcePath); // OPEN THE SOURCE DIRECTORY
    if (dir == NULL) {
        return;
    } // ERROR CHECK

    mkdir(destinationPath, 0777); // CREATE THE DESTINATION DIRECTORY

    struct dirent* entry; // USE THE DIRENT LIB
    while ((entry = readdir(dir)) != NULL) { // READ THE DIRECTORY [WHILE ENTRIES STILL EXIST]
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) { // SKIP . AND .. ENTRIES DUE TO LINUX ENVIROMENT
            continue;
        }

        char source[300];
        sprintf(source, "%s/%s", sourcePath, entry->d_name); // GET THE FULL PATH OF THE SOURCE ENTRY

        char destination[300];
        sprintf(destination, "%s/%s", destinationPath, entry->d_name); // GET THE FULL PATH OF THE DESTINATION ENTRY

        if (entry->d_type == DT_DIR) { // IF THE ENTRY IS A DIRECTORY, RECURSIVELY CALL THE FUNCTION TO COPY THE NESTED FOLDER
            copy_directory(source, destination);
        } else {
            copy_file(source, destination); // IF THE ENTRY IS A FILE, COPY IT
        }
    }

    closedir(dir); // CLOSE THE DIRECTORY
}

void merge_directories(EntryInfo* dir1Info, int size1, EntryInfo* dir2Info, int size2, const char* mergedDirName) {
    
    // CHECK IF THE MERGED DIRECTORY (NAME) ALREADY EXISTS
    if (directory_exists(mergedDirName) == NOTEXISTS) 
    {
        // CREATE IT IF NOT
        mkdir(mergedDirName, 0777);
    }

    // ITERATE THROUGH THE FIRST DIRECTORY
    for (int i = 0; i < size1; i++) 
    {
        // CREATE THE NEW PATH FOR THE NEW DIRECTORY
        char newPath[300];
        sprintf(newPath, "%s/%s", mergedDirName, dir1Info[i].relativepath); 
        char newDirPath[300];
        strncpy(newDirPath, newPath, strrchr(newPath, '/') - newPath); 

        // CHECK IF THE ENTRY IS A FILE
        if (dir1Info[i].type == FILE1) 
        {
            // IF THE FILE ALREADY EXISTS IN THE NEW DIRECTORY (CASE OF SAME FILE WITHIN BOTH DIRECTORIES)
            if (file_exists(newPath) == EXISTS) {
                struct stat statbuf;
                stat(newPath, &statbuf); // GET THE I-NODE INFO OF THE FILE
                
                if (dir1Info[i].lastedited > statbuf.st_mtime)  // COMPARE LAST EDITED DATES OF EACH
                {
                    copy_file(dir1Info[i].path, newPath);   // KEEP THE MOST RECENTLY EDITED ONE (AS PER ASSIGNMENT INSTRUCTIONS)
                }
            } else 
            {
                copy_file(dir1Info[i].path, newPath);       // COPY THE FILE IF IT DOES NOT EXIST
            }
        } else if (dir1Info[i].type == DIRECTORY)  // CHECK IF THE ENTRY IS A DIRECTORY
        {
            if (directory_exists(newPath) == EXISTS) 
            {
                continue;   // IGNORE DUPLICATES (CASE OF SAME DIRECTORY WITHIN BOTH DIRECTORIES)
            } else 
            {
                copy_directory(dir1Info[i].path, newPath);  // COPY THE DIRECTORY IF IT DOES NOT ALREADY EXIST
            }
        } else if (dir1Info[i].type == SOFT_LINK) // CHECK IF THE ENTRY IS A SOFT LINK
        {
            symlink(dir1Info[i].path, newPath); // CREATE THE CORRESPONDING LINK IN THE NEW DIRECTORY
        } else if ( dir1Info[i].type == HARD_LINK) // CHECK IF THE ENTRY IS A HARD LINK
        {
            link(dir1Info[i].path, newPath);  // CREATE THE CORRESPONDING LINK IN THE NEW DIRECTORY
        }
    }

    // REPEAT THE SAME PROCESS FOR THE SECOND DIRECTORY
    for (int i = 0; i < size2; i++) 
    {
        char newPath[300];
        sprintf(newPath, "%s/%s", mergedDirName, dir2Info[i].relativepath);
        char newDirPath[300];
        strncpy(newDirPath, newPath, strrchr(newPath, '/') - newPath);

        if (dir2Info[i].type == FILE1) 
        {
            if (file_exists(newPath) == EXISTS) 
            {
                struct stat statbuf;
                stat(newPath, &statbuf);
                
                if (dir2Info[i].lastedited > statbuf.st_mtime) 
                {
                    copy_file(dir2Info[i].path, newPath);
                }
            } else 
            {
                copy_file(dir2Info[i].path, newPath);
            }
        } else if (dir2Info[i].type == DIRECTORY) 
        {
            if (directory_exists(newPath) == EXISTS) 
            {
                continue;
            } else {
                copy_directory(dir2Info[i].path, newPath);
            }
        } else if (dir2Info[i].type == SOFT_LINK)
        {
            symlink(dir2Info[i].path, newPath);
        } else if ( dir2Info[i].type == HARD_LINK)
        {
            link(dir2Info[i].path, newPath);                
        }
    }
}