
#ifndef ZOS_FAT_H
#define ZOS_FAT_H

#include <bits/stringfwd.h>
#include <string>
#include <stdlib.h>
#include <cmath>
#include <deque>
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


    struct boot_record {
        char volume_descriptor[250];
        int8_t fat_type;
        int8_t fat_copies;
        int16_t cluster_size;
        int32_t cluster_count;
        char signature[9];
    };


    struct directory{
        char file_name[13];
        bool isFile;
        int32_t size;
        int32_t start_cluster;
    };

    struct dir_position{
        directory dir;
        int32_t parent;
        int32_t old_cluster;
    };

    FILE *f;
    struct boot_record *p_boot_record;
    std::vector<int32_t> fatTable;
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
    std::vector<int32_t> setFatTable();
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
    int getNumberOfClusters(int file_size);
    void defragment();
    std::vector<std::string> loadClusters();
    std::deque<Fat::dir_position> loadDirectories();
    int getFreeFolderIndex();
    void printFatTable();
    void freeBootRecord();
};



#endif //ZOS_FAT_H
