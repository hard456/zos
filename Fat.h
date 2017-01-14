
#ifndef ZOS_FAT_H
#define ZOS_FAT_H

#include <bits/stringfwd.h>
#include <string>
#include <stdlib.h>
#include <cmath>

#include <vector>

class Fat {

private:

    //boot_record constants
    static const int8_t FAT_TYPE = 8;
    static const int8_t FAT_COPIES = 2;
    static const int16_t CLUSTER_SIZE = 256;
    static const int32_t CLUSTER_COUNT = 251;

    //definice na vyznam hodnot FAT tabulky
    static const int32_t FAT_UNUSED = INT32_MAX - 1;
    static const int32_t FAT_FILE_END = INT32_MAX - 2;
    static const int32_t FAT_BAD_CLUSTER = INT32_MAX - 3;
    static const int32_t FAT_DIRECTORY = INT32_MAX - 4;

    //272B
    struct boot_record {
        char volume_descriptor[250];               //popis
        int8_t fat_type;                             //typ FAT - pocet clusteru = 2^fat_type (priklad FAT 12 = 4096)
        int8_t fat_copies;                           //kolikrat je za sebou v souboru ulozena FAT
        int16_t cluster_size;                //velikost clusteru ve znacich (n x char) + '/0' - tedy 128 znamena 127 vyznamovych znaku + '/0'
        int32_t cluster_count;               //pocet pouzitelnych clusteru (2^fat_type - reserved_cluster_count)
        char signature[9];                        //pro vstupni data od vyucujicich konzistence FAT - "OK","NOK","FAI" - v poradku / FAT1 != FAT2 != FATx / FAIL - ve FAT1 == FAT2 == FAT3, ale obsahuje chyby, nutna kontrola
    };


    struct directory{
        char file_name[13];             //8+3 format + '/0'
        bool isFile;                //0 = soubor, 1 = adresar
        int32_t size;                 //pocet znaku v souboru
        int32_t start_cluster;     //cluster ve FAT, kde soubor zacina - POZOR v cislovani root_directory ma prvni cluster index 0 (viz soubor a.txt)
    };

    FILE *f;
    struct boot_record *p_boot_record;
    std::vector<int32_t> fatTable;
    std::vector<std::string> clusterContent;
    int rootDirectoryPosition;
    std::string escape;

public:
    std::vector<directory> dir;
    bool newfile;
    int clusterStartIndex;
    std::string filename;

    void openFatFile(char* filename);
    void closeFatFile();
    void loadBootRecord();
    void setBootRecord();
    void loadFatTable();
    void setFatTable();
    void loadFile();
    void writeBootRecord();
    void printfBootRecord();
    void loadDirectory();
    void printFileContent(int fatPosition);
    std::vector<std::string> getPathVector(char *file);
    int checkPath(char *file);
    void tree();
    void printfFileClusterIndexes(int index);
    bool addFolder(char *newFolder);
    int getClusterIndex(std::string name);
    int getFreeCluster();
    bool isFolderEmpty();
    void deleteFolder(std::string filename);
    void setRootPosition();
    bool addFile(char *newFile);
    std::vector<int> getFreeTableArray(int count);
    void deleteFile(std::string fileName, int fileCluster);
    void writeFatTable();
    void writeFreeClusters();
    bool isItemInFolder();
    void printTreeItems();
    void toUpperCase();
    int getNumberOfClusters(int file_size);

    char *appendCharToCharArray(char *array, char a);

    char *removeCharToCharArray(char *array);
};



#endif //ZOS_FAT_H
