
#include "Fat.h"

#include <stdio.h>
#include <iostream>
#include <string.h>

void Fat::openFatFile(char *filename) {
    if ((f = fopen(filename, "r+")) == NULL) {
        newfile = true;
        f = fopen(filename, "w+");
    } else {
        newfile = false;
    }
}

void Fat::closeFatFile() {
    fclose(f);
}

void Fat::loadBootRecord() {
    fseek(f, SEEK_SET, 0); //skok na začátek
    fread(p_boot_record, sizeof(struct boot_record), 1, f);
    printfBootRecord();
}

void Fat::setBootRecord() {
    p_boot_record->fat_type = FAT_TYPE;
    p_boot_record->fat_copies = FAT_COPIES;
    p_boot_record->cluster_size = CLUSTER_SIZE;
    p_boot_record->cluster_count = CLUSTER_COUNT;
//    printfBootRecord();
}

void Fat::loadFatTable() {
    fatTable.resize(p_boot_record->cluster_count);
    int32_t *fat_item;
    fat_item = (int32_t *) malloc(sizeof(int32_t));
    for (int j = 0; j < p_boot_record->fat_copies; j++) {
        for (int i = 0; i < p_boot_record->cluster_count; i++) {
            fread(fat_item, sizeof(*fat_item), 1, f);
            if (j == 0) {
                fatTable.at(i) = (int32_t) *fat_item;
                if (fatTable.at(i) == FAT_UNUSED) { printf("%d - FILE_UNUSED \n", fatTable.at(i)); }
                else if (fatTable.at(i) == FAT_FILE_END) { printf("%d - FAT_FILE_END \n", fatTable.at(i)); }
                else if (fatTable.at(i) == FAT_BAD_CLUSTER) { printf("%d - FAT_BAD_CLUSTER \n", fatTable.at(i)); }
                else if (fatTable.at(i) == FAT_DIRECTORY) { printf("%d - FAT_DIRECTORY \n", fatTable.at(i)); }
                else { printf("%d \n", fatTable.at(i)); }
            } else {
                if (fatTable.at(i) != (int32_t) *fat_item) {
                    std::cout << "FAT 0 at index " << i << " does not equal with FAT " << j << "." << std::endl;
                }
            }
        }
    }
    rootDirectoryPosition = ftell(f);
}

void Fat::setFatTable() {
    fatTable.resize(p_boot_record->cluster_count);
    for (int i = 0; i < p_boot_record->cluster_count; i++) {
        fatTable.at(i) = FAT_UNUSED;
    }
}

void Fat::loadFile() {

    p_boot_record = (struct boot_record *) malloc(sizeof(struct boot_record));

    if (newfile) {
        setBootRecord();
        setFatTable();
    } else {
        loadBootRecord();
        loadFatTable();
    }


    dir.resize((int) p_boot_record->cluster_size / sizeof(directory));

}

void Fat::writeBootRecord() {
    fseek(f, SEEK_SET, 0);
    memset(p_boot_record->signature, '\0', sizeof(p_boot_record->signature));
    memset(p_boot_record->volume_descriptor, '\0', sizeof(p_boot_record->volume_descriptor));
    fwrite(&p_boot_record, sizeof(p_boot_record), 1, f);
}

void Fat::printfBootRecord() {
    std::cout << "volume_descriptor: " << p_boot_record->volume_descriptor << std::endl;
    printf("fat_type: %d\n", p_boot_record->fat_type);
    std::cout << "cluster_count: " << p_boot_record->cluster_count << std::endl;
    printf("fat_copies: %d\n", p_boot_record->fat_copies);
    std::cout << "cluster_size: " << p_boot_record->cluster_size << std::endl;
    std::cout << "signature: " << p_boot_record->signature << std::endl;
}

void Fat::loadRootDirectory() {

    struct directory *dir_item = (struct directory *) malloc(sizeof(struct directory));
    for (int i = 0; i < dir.size(); i++) {
        fread(dir_item, sizeof(*dir_item), 1, f);
        dir.at(i) = *dir_item;
    }
    fseek(f, -(sizeof(struct directory) * dir.size()), SEEK_CUR);
}

void Fat::printFileContent(int fatPosition) {
    int actualClusterPosition = 0;
    bool endCycle = false;
    while (true) {
        fseek(f, p_boot_record->cluster_size * (fatPosition - actualClusterPosition),
              SEEK_CUR);
        char *p_cluster = (char *) malloc(sizeof(char) * (p_boot_record->cluster_size));
        fread(p_cluster, sizeof(char) * p_boot_record->cluster_size, 1, f);
        printf("%s",p_cluster);
        fseek(f, -(sizeof(char) * (p_boot_record->cluster_size)), SEEK_CUR);

        actualClusterPosition = fatPosition;
        fatPosition = fatTable.at(fatPosition);
        if(endCycle == true){
            break;
        }
        if(fatTable.at(fatPosition) == FAT_FILE_END){
            endCycle = true;
        }
}
}

std::vector<std::string> Fat::getPathVector(char *file) {
    std::string item;
    std::vector<std::string> path;
    for (int i = 0; i < strlen(file); i++) {
        if(i == 0){
            if(file[i] == '/'){
                path.push_back("/");
                continue;
            }
            else if(file[i] != '/'){
                path.push_back("/");
            }
        }
        if (file[i] == '/' && item == "") {
            continue;
        } else if (i == strlen(file) - 1) {
            if (file[i] != '/') {
                item = item + file[i];
                path.push_back(item);
                item.clear();
            } else if (item != "") {
                path.push_back(item);
                item.clear();
            }
        } else if (file[i] != '/') {
            item = item + file[i];
        } else if (file[i] == '/' && item != "") {
            path.push_back(item);
            item.clear();
        }
    }
    return path;
}

int Fat::checkPath(char *file) {

    std::vector<std::string> path;
    path = getPathVector(file);
    int actualClusterPosition = 0;
    parentIndex = path.size()-2;
    if(path.size()-1 == 0){
        return -2;
    }
    for (int i = 0; i < path.size(); i++) {
        if(path.at(i) == "/"){
            parentIndex = getParentIndex(path.at(i+1));
            continue;
        }
        for (int j = 0; j < dir.size(); j++) {
            if (path.at(i) == dir.at(j).file_name) {
                if( i == (path.size()-1)){
                    parentIndex = dir.at(j).start_cluster;
                }
                else{

                }
                fseek(f, p_boot_record->cluster_size * (dir.at(j).start_cluster - actualClusterPosition), SEEK_CUR);
                if (i == (path.size() - 1)) {
                    fseek(f,rootDirectoryPosition,SEEK_SET);
                    if(dir.at(j).isFile == 0){
                        filename = path.at(path.size()-1);
                        startIndex = dir.at(j).start_cluster;
                        return 0;
                    }
                    filename = path.at(path.size()-1);
                    startIndex = dir.at(j).start_cluster;
                    return 1;
                }
                actualClusterPosition = dir.at(j).start_cluster;
                loadRootDirectory();
                break;

            } else if (j == (dir.size() - 1)) {
                return -1;
            }
        }
    }
    return -1;
}

void Fat::tree() {
    fseek(f,rootDirectoryPosition,SEEK_SET);
}

bool Fat::isFolderEmpty(int clusterIndex) {

}

void Fat::printfFileClusterIndexes(int index) {
    while(true){
        if(fatTable.at(index) == FAT_FILE_END){
            printf("%d",index);
            break;
        }
        printf("%d, ",index);
        index = fatTable.at(index);

    }
}

bool Fat::addFolder(char *newFolder) {
    int adrEmptyIndex;
    int cluster;
    int indexInputCluster = parentIndex;
    if(parentIndex == -1){
        indexInputCluster = 0;
    }
    fseek(f,rootDirectoryPosition+p_boot_record->cluster_size*indexInputCluster,SEEK_SET);
    loadRootDirectory();
    fseek(f,rootDirectoryPosition+p_boot_record->cluster_size*indexInputCluster,SEEK_SET);
    for (int i = 0; i < dir.size(); i++) {
        if(dir.at(i).file_name[0] == '\0'){
            adrEmptyIndex = i;
            break;
        }
    }
    fseek(f,sizeof(struct directory)*adrEmptyIndex,SEEK_CUR);
    struct directory adr;
    memset(adr.file_name,'\0',sizeof(adr.file_name));
    strcpy(adr.file_name, newFolder);
    adr.size = 0;
    adr.isFile = 0;
    if((cluster = getFreeCluster()) != -1){
        adr.start_cluster = cluster;
    }
    else{
        return false;
    }
    fwrite(&adr, sizeof(adr), 1, f);
    return true;
}

int Fat::getParentIndex(std::string name) {
    for (int i = 0; i < dir.size(); i++) {
        if(dir.at(i).file_name == name){
            return dir.at(i).start_cluster;
        }
    }
}

int Fat::getFreeCluster() {
    for (int i = 0; i < fatTable.size(); i++) {
        if(fatTable.at(i) == FAT_UNUSED){
            fatTable.at(i) = FAT_DIRECTORY;
            return i;
        }
    }
    return -1;
}

bool Fat::isFolderEmpty(){
    fseek(f,rootDirectoryPosition+p_boot_record->cluster_size*parentIndex,SEEK_SET);
    loadRootDirectory();
    char *positionInCluster = (char*) malloc(sizeof(p_boot_record->cluster_count));
    fread(positionInCluster, sizeof(p_boot_record->cluster_count), 1, f);
    for (int i = 0; i < p_boot_record->cluster_count; i++) {
        if(*positionInCluster != '\0'){
            return false;
        }
    }

    return true;
}

void Fat::deleteFolder(std::string filename) {
    if(parentIndex == -1){
        parentIndex = 0;
    }
    std::cout << parentIndex;
    int foldePosition=0;
    fseek(f,rootDirectoryPosition+p_boot_record->cluster_size*parentIndex,SEEK_SET);
    loadRootDirectory();
    for (int i = 0; i < dir.size(); i++) {
        if(dir.at(i).file_name == filename){
            foldePosition = i;
            fatTable.at(dir.at(i).start_cluster) = FAT_UNUSED;
            break;
        }
    }
    fseek(f,sizeof(struct directory)*(foldePosition),SEEK_CUR);
    struct directory *adr = (struct directory *) malloc(sizeof(struct directory));
    fwrite(adr, sizeof(struct directory), 1, f);
    std::cout << "OK" << std::endl;
}

void Fat::setRootPosition() {
    fseek(f,rootDirectoryPosition,SEEK_SET);

}
