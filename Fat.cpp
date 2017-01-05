
#include "Fat.h"

#include <stdio.h>
#include <iostream>
#include <string.h>

void Fat::openFatFile(char* filename) {
        if((f = fopen(filename,"r+")) == NULL){
            newfile = true;
            f = fopen(filename,"w+");
        }
}

void Fat::closeFatFile() {
        fclose(f);
}

void Fat::loadBootRecord() {
    fseek(f, SEEK_SET, 0); //skok na začátek
    p_boot_record = (struct boot_record *) malloc (sizeof(struct boot_record));
    fread(p_boot_record, sizeof(struct boot_record), 1, f);
    printfBootRecord();
}

void Fat::setBootRecord() {
    p_boot_record->fat_type = FAT_TYPE;
    p_boot_record->fat_copies = FAT_COPIES;
    p_boot_record->cluster_size = CLUSTER_SIZE;
    p_boot_record->cluster_count = CLUSTER_COUNT;
    printfBootRecord();
}

void Fat::loadFatTable() {
    fatTable.resize(p_boot_record->cluster_count);
    int32_t *fat_item;
    fat_item = (int32_t *) malloc (sizeof (int32_t));
    for (int j = 0; j < p_boot_record->fat_copies; j++) {
    for (int i = 0; i < p_boot_record->cluster_count; i++) {
        fread(fat_item, sizeof(*fat_item),1,f);
        if(j == 0){
            fatTable.at(i) = (int32_t) *fat_item;
            if(fatTable.at(i) == FAT_UNUSED){ printf("%d - FILE_UNUSED \n",fatTable.at(i)); }
            else if(fatTable.at(i) == FAT_FILE_END){ printf("%d - FAT_FILE_END \n",fatTable.at(i)); }
            else if(fatTable.at(i) == FAT_BAD_CLUSTER){ printf("%d - FAT_BAD_CLUSTER \n",fatTable.at(i)); }
            else if(fatTable.at(i) == FAT_DIRECTORY) { printf("%d - FAT_DIRECTORY \n",fatTable.at(i));  }
            else{ printf("%d \n", fatTable.at(i)); }
        }
        else{
            if(fatTable.at(i) != (int32_t) *fat_item){
                std::cout << "FAT 0 at index " << i << "doest not equal with FAT " << j  << "." << std::endl;
            }
        }
    }
    }
}

void Fat::setFatTable() {
    fatTable.resize(p_boot_record->cluster_count);
    for (int i = 0; i < p_boot_record->cluster_count; i++) {
        fatTable.at(i) = FAT_UNUSED;
    }
}

void Fat::loadVectorCluster() {
    cluster.resize(p_boot_record->cluster_count);
    char *p_cluster = (char*) malloc(sizeof(char) * (p_boot_record->cluster_size));
    for (int i = 0; i < p_boot_record->cluster_count; i++) {
        fread(p_cluster, sizeof(char) * p_boot_record->cluster_size, 1, f);
        std::cout << i << ": " << p_cluster << std::endl;
        cluster.at(i) = p_cluster;
    }

}

void Fat::setVectorCluster() {
    cluster.resize(p_boot_record->cluster_count);
}

void Fat::loadFile() {

    loadBootRecord();

    if(newfile == true){
        setBootRecord();
        setFatTable();
        setVectorCluster();
    }
    else{
        loadFatTable();
        loadVectorCluster();
    }
}

void Fat::writeBootRecord() {
    fseek(f, SEEK_SET, 0);
    memset(p_boot_record->signature, '\0', sizeof(p_boot_record->signature));
    memset(p_boot_record->volume_descriptor, '\0', sizeof(p_boot_record->volume_descriptor));
    fwrite(&p_boot_record, sizeof(p_boot_record), 1, f);
}

void Fat::printfBootRecord() {
    std::cout << "volume_descriptor: " << p_boot_record->volume_descriptor << std::endl;
    printf("fat_type: %d\n",p_boot_record->fat_type);
    std:: cout << "cluster_count: " << p_boot_record->cluster_count << std::endl;
    printf("fat_copies: %d\n",p_boot_record->fat_copies);
    std:: cout << "cluster_size: " << p_boot_record->cluster_size << std::endl;
    std:: cout << "signature: " << p_boot_record->signature << std::endl;
}
