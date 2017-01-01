
#include "Fat.h"

#include <stdio.h>
#include <iostream>
#include <string.h>

void Fat::openFatFile(char* filename) {
        if((f = fopen(filename,"r+")) == NULL){
            f = fopen(filename,"w+");
        }
}

void Fat::closeFatFile() {
        fclose(f);
}

void Fat::getBootRecord() {
    fseek(f, SEEK_SET, 0); //skok na začátek
    p_boot_record = (struct boot_record *) malloc (sizeof(struct boot_record));
    fread(p_boot_record, sizeof(struct boot_record), 1, f);

    std::cout << p_boot_record->volume_descriptor << std::endl;
    std:: cout << p_boot_record->fat_type << std::endl;
    std:: cout << p_boot_record->cluster_count << std::endl;
    std:: cout << p_boot_record->fat_copies << std::endl;
    std:: cout << p_boot_record->cluster_size << std::endl;
    std:: cout << p_boot_record->reserved_cluster_count << std::endl;
    std:: cout << p_boot_record->root_directory_max_entries_count << std::endl;
    std:: cout << p_boot_record->signature << std::endl;
}

void Fat::setBootRecord() {
    p_boot_record->fat_type = FAT_TYPE;
    p_boot_record->fat_copies = FAT_COPIES;
    p_boot_record->cluster_size = CLUSTER_SIZE;
    p_boot_record->root_directory_max_entries_count = ROOT_DIRECOTY_MAX_ENTRIES_COUNT;
    p_boot_record->cluster_count = CLUSTER_COUNT;
    p_boot_record->reserved_cluster_count = RESERVED_CLUSTER_COUNT;
}

void Fat::getFatTable() {
    fatTable.resize(p_boot_record->cluster_count);
    unsigned int *fat_item;
    fat_item = (unsigned int *) malloc (sizeof (unsigned int));
    for (int i = 0; i < p_boot_record->cluster_count; ++i) {
        fread(fat_item, sizeof(unsigned int),1,f);
        std::cout << fat_item << std::endl;
        fatTable.at(i) = (unsigned int) fat_item;
    }
}

void Fat::setFatTable() {
    fatTable.resize(p_boot_record->cluster_count);
}

void Fat::getVectorCluster() {
    cluster.resize(p_boot_record->cluster_count);
    char *p_cluster = (char *) malloc(sizeof(char) * (p_boot_record->cluster_size));
    for (int i = 0; i < p_boot_record->cluster_count; i++) {
        fread(p_cluster, sizeof(char) * p_boot_record->cluster_size, 1, f);
        std::cout << p_cluster << std::endl;
        cluster.at(i) = p_cluster;
    }

}

void Fat::setVectorCluster() {
    cluster.resize(p_boot_record->cluster_count);
}

void Fat::loadFileStructure() {

    getBootRecord();

    if(p_boot_record->fat_type != FAT_TYPE){
        setBootRecord();
        setFatTable();
        setVectorCluster();
    }
    else{
        getFatTable();
        getVectorCluster();
    }
}

void Fat::writeBootRecord() {
    memset(p_boot_record->signature, '\0', sizeof(p_boot_record->signature));
    memset(p_boot_record->volume_descriptor, '\0', sizeof(p_boot_record->volume_descriptor));
    fwrite(&p_boot_record, sizeof(p_boot_record), 1, f);
}
