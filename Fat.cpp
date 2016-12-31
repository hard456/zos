
#include "Fat.h"

#include <stdio.h>


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

    //nastaví p_boot_record
    if(!isdigit(p_boot_record->fat_type)){
        setBootRecord();
        setFatTable();
    }
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
    if(fatTable.size() != p_boot_record->cluster_count){
        fatTable.resize(p_boot_record->cluster_count);
        unsigned int *fat_item;
        fat_item = (unsigned int *) malloc (sizeof (unsigned int));
        for (int i = 0; i < p_boot_record->cluster_count; ++i) {
            fread(fat_item, sizeof(unsigned int),1,f);
            fatTable.at(i) = (unsigned int) fat_item;
        }
    }
}

void Fat::setFatTable() {
    fatTable.resize(p_boot_record->cluster_count);
}

void Fat::getVectorCluster() {

}

void Fat::setVectorCluster() {

}
