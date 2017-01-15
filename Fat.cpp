
#include "Fat.h"

#include <stdio.h>
#include <iostream>
#include <string.h>

/**
 * Otevření souboru, když nebude existovat, tak dojde k vytvoření
 * @param filename název fat souboru
 */
void Fat::openFatFile(char *filename) {
    if ((f = fopen(filename, "r+")) == NULL) {
        newfile = true;
        f = fopen(filename, "w+");
    } else {
        newfile = false;
    }
}

/**
 * Zavře fat soubor
 */
void Fat::closeFatFile() {
    fclose(f);
}

/**
 * Načtení boot record z tabulky
 */
void Fat::loadBootRecord() {
    fseek(f, 0, SEEK_SET); //skok na začátekl
    fread(p_boot_record, sizeof(struct boot_record), 1, f);
}

/**
 * Nastavení boot record pro přídat, že fat soubor nebude existovat
 */
void Fat::setBootRecord() {
    memset(p_boot_record->signature, '\0', sizeof(p_boot_record->signature));
    memset(p_boot_record->volume_descriptor, '\0', sizeof(p_boot_record->volume_descriptor));
    char vol_descriptor[250] = "fat type 8, copies 2, cluster_size 256, cluster_count 251";
    char signature[9] = "honza";
    strcpy(p_boot_record->volume_descriptor, vol_descriptor);
    strcpy(p_boot_record->signature, signature);
    p_boot_record->fat_type = FAT_TYPE;
    p_boot_record->fat_copies = FAT_COPIES;
    p_boot_record->cluster_size = CLUSTER_SIZE;
    p_boot_record->cluster_count = CLUSTER_COUNT;
}

/**
 * Načtení fat tabulky
 */
void Fat::loadFatTable() {
    fatTable.resize(p_boot_record->cluster_count);
    int32_t *fat_item;
    fat_item = (int32_t *) malloc(sizeof(int32_t));
    for (int j = 0; j < p_boot_record->fat_copies; j++) {
        for (int i = 0; i < p_boot_record->cluster_count; i++) {
            fread(fat_item, sizeof(*fat_item), 1, f);
            if (j == 0) {
                fatTable.at(i) = (int32_t) *fat_item;
            } else {
                if (fatTable.at(i) != (int32_t) *fat_item) {
                    std::cout << "FAT 0 at index " << i << " does not equal with FAT " << j << "." << std::endl;
                }
            }
        }
    }
    rootDirectoryPosition = ftell(f);
}

/**
 * Nastaví fat table pro přípdad, že by se vytvářel nová fat soubor
 */
std::vector<int32_t> Fat::setFatTable() {
    std::vector<int32_t> fat_table;
    fat_table.resize(p_boot_record->cluster_count);
    for (int i = 0; i < p_boot_record->cluster_count; i++) {
        if(i == 0){
            fat_table.at(i) = FAT_DIRECTORY;
        }
        else{
            fat_table.at(i) = FAT_UNUSED;
        }
    }
    return fat_table;
}

/**
 * Když bude fat soubor existovat, tak zavolá funkce pro načtení fat
 * tabulky a boot recordu. Když nebude existovat, tak vytvoří fat tabulku
 * a boot record.
 */
void Fat::loadFile() {
    p_boot_record = (struct boot_record *) malloc(sizeof(struct boot_record));

    if (newfile) {
        setBootRecord();
        fatTable = setFatTable();
        writeBootRecord();
        writeFatTable();
        writeFreeClusters();
        fseek(f,sizeof(struct boot_record)+fatTable.size()*sizeof(int32_t)*p_boot_record->fat_copies,SEEK_SET);
        rootDirectoryPosition = ftell(f);
    } else {
        loadBootRecord();
        loadFatTable();
    }
    dir.resize((int) p_boot_record->cluster_size / sizeof(directory));

}

/**
 * Vytiskne boot record
 */
void Fat::printfBootRecord() {
    std::cout << "volume_descriptor: " << p_boot_record->volume_descriptor << std::endl;
    printf("fat_type: %d\n", p_boot_record->fat_type);
    std::cout << "cluster_count: " << p_boot_record->cluster_count << std::endl;
    printf("fat_copies: %d\n", p_boot_record->fat_copies);
    std::cout << "cluster_size: " << p_boot_record->cluster_size << std::endl;
    std::cout << "signature: " << p_boot_record->signature << std::endl;
}

/**
 * Načtě adresář do vectoru dir
 */
void Fat::loadDirectory() {
    struct directory *dir_item = (struct directory *) malloc(sizeof(struct directory));
    for (int i = 0; i < dir.size(); i++) {
        fread(dir_item, sizeof(*dir_item), 1, f);
        dir.at(i) = *dir_item;
    }
    fseek(f, -(sizeof(struct directory) * dir.size()), SEEK_CUR);
}

/**
 * Vypíše obsah souboru
 * @param fatPosition počínají index clusteru souboru
 */
void Fat::printFileContent(int fatPosition) {
    int actualClusterPosition = 0;
    bool endCycle = false;
    while (true) {
        if(endCycle == true){
            break;
        }
        fseek(f, p_boot_record->cluster_size * (fatPosition - actualClusterPosition),
              SEEK_CUR);
        char *p_cluster = (char *) malloc(sizeof(char) * (p_boot_record->cluster_size));
        fread(p_cluster, sizeof(char) * p_boot_record->cluster_size, 1, f);
        printf("%s",p_cluster);
        fseek(f, -(sizeof(char) * (p_boot_record->cluster_size)), SEEK_CUR);

        actualClusterPosition = fatPosition;
        fatPosition = fatTable.at(fatPosition);
        if(fatPosition == FAT_FILE_END){
            endCycle = true;
        }
}
}

/**
 * Vrátí vector cesty z původního ukazatele cesty
 * @param file ukazatel cesty
 * @return vector cesty
 */
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

/**
 * Ověří existenci cesty a nastaví číslo číslo clusteru souboru nebo adresáře
 * @param file cesta souboru
 * @return -1 cesta neexistuje, 1 jedná se o soubor, 0 adresář, -2 root
 */
int Fat::checkPath(char *file) {
    std::vector<std::string> path;
    path = getPathVector(file);
    int actualClusterPosition = 0;
    clusterStartIndex = path.size()-2;
    if(path.size()-1 == 0){
        return -2;
    }
    for (int i = 0; i < path.size(); i++) {
        if(path.at(i) == "/"){
            clusterStartIndex = getClusterIndex(path.at(i + 1));
            continue;
        }
        for (int j = 0; j < dir.size(); j++) {
            if (path.at(i) == dir.at(j).file_name) {
                fseek(f, p_boot_record->cluster_size * (dir.at(j).start_cluster - actualClusterPosition), SEEK_CUR);
                if (i == (path.size() - 1)) {
                    clusterStartIndex = dir.at(j).start_cluster;
                    fseek(f,rootDirectoryPosition,SEEK_SET);
                    if(dir.at(j).isFile == 0){
                        filename = path.at(path.size()-1);

                        return 0;
                    }
                    filename = path.at(path.size()-1);

                    return 1;
                }
                actualClusterPosition = dir.at(j).start_cluster;
                loadDirectory();
                break;

            } else if (j == (dir.size() - 1)) {
                return -1;
            }
        }
    }
    return -1;
}

/**
 * Vypíše indexy clusterů, kde leží soubor
 * @param index počáteční index souboru
 */
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

/**
 * Přidá adresář do virtuální fatky
 * @param newFolder je název adresáře pro přidání
 * @return true, když se povedlo přidat
 */
bool Fat::addFolder(char *newFolder) {
    int adrEmptyIndex;
    int cluster;
    int indexInputCluster = clusterStartIndex;
    if(clusterStartIndex == -1){
        indexInputCluster = 0;
    }
    fseek(f,rootDirectoryPosition+p_boot_record->cluster_size*indexInputCluster,SEEK_SET);
    loadDirectory();
    fseek(f,rootDirectoryPosition+p_boot_record->cluster_size*indexInputCluster,SEEK_SET);
    for (int i = 0; i < dir.size(); i++) {
        if(strcmp(newFolder,dir.at(i).file_name) == 0){
            std::cout << "FOLDER ALREADY EXISTS" << std::endl;
            return false;
        }
    }
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
        std::cout << "DIRECTORY IS FULL" << std::endl;
        return false;
    }
    fwrite(&adr, sizeof(adr), 1, f);
    return true;
}

/**
 * Vrátí index clusteru pro file nebo adresář, porovnává to
 * s vektorem dir, který obsahuje položky určitého adresáře
 * @param name jméno adresáře
 * @return index clusteru
 */
int Fat::getClusterIndex(std::string name) {
    for (int i = 0; i < dir.size(); i++) {
        if(dir.at(i).file_name == name){
            return dir.at(i).start_cluster;
        }
    }
}

/**
 * Vrátí číslo volného cluster podle Fat tabulky
 * @return -1 když nebude žádný cluster volný, jinak vrátí index clusteru
 */
int Fat::getFreeCluster() {
    for (int i = 0; i < fatTable.size(); i++) {
        if(fatTable.at(i) == FAT_UNUSED){
            fatTable.at(i) = FAT_DIRECTORY;
            return i;
        }
    }
    return -1;
}

/**
 * Zjistí, jestli jsou v adresáři nějaké jiné složky nebo soubory
 * @return true když je adresář prázdný
 */
bool Fat::isFolderEmpty(){
    fseek(f,rootDirectoryPosition+p_boot_record->cluster_size*clusterStartIndex,SEEK_SET);
    char *positionInCluster = (char*) malloc(sizeof(p_boot_record->cluster_size));
    fread(positionInCluster, sizeof(p_boot_record->cluster_size), 1, f);
    for (int i = 0; i < p_boot_record->cluster_size; i++) {
        if(*positionInCluster != '\0'){
            return false;
        }
    }

    return true;
}

/**
 * Smaže prázdný adresář
 * @param filename jméno adresáře k smazání
 */
void Fat::deleteFolder(std::string filename) {
    if(clusterStartIndex == -1){
        clusterStartIndex = 0;
    }
    int foldePosition=0;
    fseek(f,rootDirectoryPosition+p_boot_record->cluster_size*clusterStartIndex,SEEK_SET);
    loadDirectory();
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

/**
 * Nastaví pozici v adresáři před Root cluster
 */
void Fat::setRootPosition() {
    fseek(f,rootDirectoryPosition,SEEK_SET);
}

/**
 * Přidá soubor do fat souboru
 * @param newFile cesta nového souboru
 * @return false pro chybu při přidvání (Málo místa ve fat tabulce, atd)
 */
bool Fat::addFile(char *newFile) {
    FILE *f_new;
    int emptyFolderIndex = -1;
    if(clusterStartIndex == -1){
    clusterStartIndex = 0;
    }
    std::vector<std::string> path = getPathVector(newFile);
    setRootPosition();
    fseek(f,p_boot_record->cluster_size*clusterStartIndex,SEEK_CUR);
    loadDirectory();
    for (int i = 0; i < dir.size(); ++i) {
        if(dir.at(i).file_name == path.at(path.size()-1))
        {
            std::cout << "FILE ALREADY EXISTS" << std::endl;
            return false;
        }
    }
    for (int i = 0; i < dir.size(); ++i) {
        if(dir.at(i).file_name[0] == '\0')
        {
            emptyFolderIndex = i;
            break;
        }
    }
    if(emptyFolderIndex == -1){
        std::cout << "FOLDER IS NOT EMPTY" << std::endl;
        return false;
    }
    fseek(f,sizeof(directory)*emptyFolderIndex,SEEK_CUR);

    if ((f_new = fopen(newFile, "r+")) == NULL) {
        std::cout << "FILE NOT FOUND" << std::endl;
        return false;
    }

    fseek(f_new,0,SEEK_END);
    int size = ftell(f_new);
    fseek(f_new,0,SEEK_SET);

    int count_cluster = 0;
    int remainder = 0;
    if((remainder = size % p_boot_record->cluster_size) != 0){
        count_cluster = ((int)(size/p_boot_record->cluster_size))+1;
    }
    else{
        count_cluster = ((int)(size/p_boot_record->cluster_size));
    }

    std::vector<int> free_positions = getFreeTableArray(count_cluster);

    struct directory file;
    memset(file.file_name,'\0',sizeof(file.file_name));
    strcpy(file.file_name, path.at(path.size()-1).c_str());
    file.size = size;
    file.isFile = 1;
    file.start_cluster = free_positions.at(0);

    fwrite(&file, sizeof(file), 1, f);

    for (int j = 0; j < free_positions.size(); j++) {
        if(j == free_positions.size()-1){
            fatTable.at(free_positions.at(j)) = FAT_FILE_END;
        }
        else{
            fatTable.at(free_positions.at(j)) = free_positions.at(j+1);
        }
    }
    char *read = (char *) malloc(sizeof(char) * (p_boot_record->cluster_size));
    int tmp = 0;
    for (int k = 0; k < count_cluster; k++) {
        if (tmp + sizeof(char) * p_boot_record->cluster_size > size) {
            fread(read, sizeof(char) * remainder, 1, f_new);
            fseek(f, rootDirectoryPosition + p_boot_record->cluster_size * free_positions.at(k), SEEK_SET);
            fwrite(read, sizeof(char) * remainder, 1, f);
        } else {
            tmp = tmp + sizeof(char) * p_boot_record->cluster_size;
            fread(read, sizeof(char) * p_boot_record->cluster_size, 1, f_new);
            fseek(f, rootDirectoryPosition + p_boot_record->cluster_size * free_positions.at(k), SEEK_SET);
            fwrite(read, sizeof(char) * p_boot_record->cluster_size, 1, f);
        }
    }
    return true;
}


/**
 * Funkce, která vloží do vektoru čísla prázdných clusterů podle parametru
 * @param count počet prázdných clusterů, které potřebuji
 * @return vektor prázdných čísel clusterů
 */
std::vector<int> Fat::getFreeTableArray(int count) {
    std::vector<int> free_table_position;
    for (int i = 0; i < fatTable.size(); i++) {
        if(fatTable.at(i) == FAT_UNUSED){
            if(free_table_position.size() == count){
                break;
            }
            free_table_position.push_back(i);
        }
    }
    return free_table_position;
}

/**
 * Smaže soubor z fat souboru
 * @param fileName název souboru k smazání
 * @param fileCluster číslo clusteru souboru
 */
void Fat::deleteFile(std::string fileName, int fileCluster) {
    int folderPosition = 0;
    if(clusterStartIndex == -1){
        clusterStartIndex = 0;
    }
    fseek(f,rootDirectoryPosition+p_boot_record->cluster_size*clusterStartIndex,SEEK_SET);
    loadDirectory();
    for (int i = 0; i < dir.size(); i++) {
        if(dir.at(i).file_name == fileName){
            folderPosition = i;
            break;
        }
    }
    fseek(f,sizeof(directory)*folderPosition,SEEK_CUR);
    struct directory *file = (struct directory *) malloc(sizeof(struct directory));
    fwrite(file, sizeof(struct directory), 1, f);
    int tmp = 0;
    char *empty_cluster = (char *) malloc(sizeof(char)*p_boot_record->cluster_size);
    while(true){
        fseek(f,rootDirectoryPosition+p_boot_record->cluster_size*fileCluster,SEEK_SET);
        fwrite(empty_cluster, sizeof(char)*p_boot_record->cluster_size, 1, f);
        int tmp = fileCluster;
        fileCluster = fatTable.at(fileCluster);
        fatTable.at(tmp) = FAT_UNUSED;
        if(fileCluster == FAT_FILE_END){
            fseek(f,rootDirectoryPosition+p_boot_record->cluster_size*fileCluster,SEEK_SET);
            fwrite(empty_cluster, sizeof(char)*p_boot_record->cluster_size, 1, f);
            break;
        }
    }
    std::cout << "OK" << std::endl;
}

/**
 * Zapíše boot record do fat souboru
 */
void Fat::writeBootRecord() {
    fseek(f, 0, SEEK_SET);
    fwrite(p_boot_record, sizeof(boot_record), 1, f);
}

/**
 * Zapíše fat table do fat souboru
 */
void Fat::writeFatTable() {
    int32_t fat_item;
    for (int j = 0; j < p_boot_record->fat_copies; j++) {
        for (int i = 0; i < p_boot_record->cluster_count; i++) {
            fat_item = fatTable.at(i);
            fwrite(&fat_item, sizeof(fat_item), 1, f);
        }
    }

}

/**
 * Zapíše čisté clustery při vytvoření FAT souboru
 */
void Fat::writeFreeClusters() {
    char *empty_cluster = (char *) malloc(sizeof(char)*p_boot_record->cluster_size);
    memset(empty_cluster,'\0',sizeof(char)*p_boot_record->cluster_size);
    for (int i = 0; i < p_boot_record->cluster_count; i++) {
        fseek(f,rootDirectoryPosition+p_boot_record->cluster_size*i,SEEK_SET);
        fwrite(empty_cluster, sizeof(char)*p_boot_record->cluster_size, 1, f);
    }
    setRootPosition();
}

bool Fat::isItemInFolder() {
    for (int i = 0; i < dir.size(); i++) {
        if(dir.at(i).file_name[0] != '\0'){
            return true;
        }
    }
    return false;
}

/**
 * Funkce pro výpis struktury uložených dat ve fat souboru
 */
void Fat::tree(){
    std::cout << "+ROOT" << std::endl;
    escape.append("\t");
    printTreeItems();
}

/**
 *  Rekurzivní funkce pro výpis adresáře
 */
void Fat::printTreeItems(){
    long tmp_position = 0;
    int directory_items = p_boot_record->cluster_size / sizeof(directory);
    directory *dir = (directory *) malloc(sizeof(directory));
    for (int j = 0; j < directory_items; j++) {
        fread(dir, sizeof(directory), 1, f);
        if (dir->start_cluster!=0 && dir->isFile) {
            printf("%s-%s %d %d\n", escape.c_str(), dir->file_name, dir->start_cluster, getNumberOfClusters(dir->size));
        }else if(dir->start_cluster!=0){
            printf("%s+%s\n",escape.c_str(),dir->file_name);
            escape.append("\t");
            tmp_position = ftell(f);
            setRootPosition();
            fseek(f, p_boot_record->cluster_size*dir->start_cluster, SEEK_CUR);
            printTreeItems();
            fseek(f, tmp_position, SEEK_SET);
        }
    }
    escape = escape.substr(0, escape.size()-1);
    printf("%s--\n", escape.c_str());
    free(dir);
}

/**
 * Vrátí počet clusterů souboru podle velikosti
 * @param file_size velikost souboru
 * @return počet clusterů
 */
int Fat::getNumberOfClusters(int file_size) {
    if(file_size % p_boot_record->cluster_size == 0){
        int fileClusterCount = file_size / p_boot_record->cluster_size;
        return fileClusterCount;
    }
    else{
        int fileClusterCount = file_size / p_boot_record->cluster_size + 1;
        return fileClusterCount;
    }
}

/**
 * Defragmentace FAT souboru
 */
void Fat::defragment() {
    int position = 1;
    int freeFolderIndex = 0;
    bool isReadyToAdd = true;
    struct dir_position dir_pos;
    std::vector<std::string> clusters;
    std::vector<int32_t> fat_table;
    std::vector<Fat::dir_position>dir_used;
    std::deque<Fat::dir_position> directory;
    fat_table = setFatTable();
    clusters = loadClusters();
    directory = loadDirectories();
    setRootPosition();
    writeFreeClusters();
    while(!directory.empty()){
        dir_pos = directory.front();
        directory.pop_front();
        if(dir_pos.parent != 0){
            for (int i = 0; i < directory.size(); i++) {
                if(directory.at(i).dir.start_cluster == dir_pos.parent){
                    directory.push_back(dir_pos);
                    isReadyToAdd = false;
                    break;
                }
            }
            if(!isReadyToAdd){
                isReadyToAdd = true;
                continue;
            }
            else{
                int parent = 0;
                int positionInFolder = 0;
                for (int i = 0; i < dir_used.size(); i++) {
                    if(dir_used.at(i).old_cluster == dir_pos.parent){
                        parent = dir_used.at(i).dir.start_cluster;
                        break;
                    }
                }

                fseek(f,rootDirectoryPosition + p_boot_record->cluster_size*parent, SEEK_SET);
                loadDirectory();
                for (int j = 0; j < dir.size(); j++) {
                    if(strcmp(dir.at(j).file_name,dir_pos.dir.file_name) == 0){
                        positionInFolder = j;
                        break;
                    }
                }
                fseek(f,rootDirectoryPosition + p_boot_record->cluster_size*parent + sizeof(struct directory)*positionInFolder,SEEK_SET);
                dir_pos.old_cluster = dir_pos.dir.start_cluster;
                dir_pos.dir.start_cluster = position;
                fwrite(&dir_pos.dir, sizeof(struct directory),1,f);
            }
        }
        else {
            setRootPosition();
            freeFolderIndex = getFreeFolderIndex();
            fseek(f, rootDirectoryPosition + sizeof(struct directory) * freeFolderIndex, SEEK_SET);
            dir_pos.old_cluster = dir_pos.dir.start_cluster;
            dir_pos.dir.start_cluster = position;
            fwrite(&dir_pos.dir, sizeof(struct directory), 1, f);
        }
            if(!dir_pos.dir.isFile){
                fseek(f,rootDirectoryPosition + p_boot_record->cluster_size*dir_pos.dir.start_cluster, SEEK_SET);
                for (int i = 0; i < directory.size(); i++) {
                    if(directory.at(i).parent == dir_pos.old_cluster){
                        fwrite(&directory.at(i).dir,sizeof(struct directory),1,f);
                        fseek(f,sizeof(struct directory),SEEK_CUR);
                    }
                }
                dir_used.push_back(dir_pos);
                fat_table.at(position) = FAT_DIRECTORY;
                position++;
            }
            else{
                int clusters_number = getNumberOfClusters(dir_pos.dir.size);
                std::vector<int32_t> old_clusters;
                int index = dir_pos.old_cluster;
                while(true){
                    if(fatTable.at(index) == FAT_FILE_END){
                        old_clusters.push_back(index);
                        break;
                    }
                    old_clusters.push_back(index);
                    index = fatTable.at(index);
                }

                char *cluster_item;

                for (int i = 0; i < clusters_number; i++) {
                    cluster_item = (char *) clusters.at(old_clusters.at(i)).c_str();
                    if(i == (clusters_number-1)){
                        int remainder = dir_pos.dir.size % p_boot_record->cluster_size;
                        if(remainder != 0){
                            char *cluster_item = (char *) malloc(sizeof(char) * (remainder));
                        }
                        fseek(f, rootDirectoryPosition + p_boot_record->cluster_size * position, SEEK_SET);
                        fwrite(cluster_item, sizeof(char)*remainder, 1, f);
                        fat_table.at(position) = FAT_FILE_END;
                        position++;
                    }
                    else{
                        fseek(f, rootDirectoryPosition + p_boot_record->cluster_size * position, SEEK_SET);
                        fwrite(cluster_item, sizeof(char) * p_boot_record->cluster_size, 1, f);
                        fat_table.at(position) = position+1;
                        position++;
                    }
                }
        }
    }
    fatTable = fat_table;
}

/**
 * Načte obsah clusterů do vektoru
 * @return vrátí vektro clusterů
 */
std::vector<std::string> Fat::loadClusters() {
    std::vector<std::string> clusters;
    clusters.resize(p_boot_record->cluster_count);
    char *p_cluster = (char *) malloc(sizeof(char) * (p_boot_record->cluster_size));
    for (int i = 0; i < p_boot_record->cluster_count; i++) {
        if(fatTable[i] != FAT_UNUSED && fatTable[i] != FAT_DIRECTORY && fatTable[i] != FAT_BAD_CLUSTER){
            fseek(f, rootDirectoryPosition + p_boot_record->cluster_size * i, SEEK_SET);
            fread(p_cluster, sizeof(char) * p_boot_record->cluster_size, 1, f);
            clusters.at(i) = p_cluster;
        }
    }
    return clusters;
}

/**
 * Načtení FAT souborů a adresářů do deque
 * @return vrátí deque souborů a adresářů
 */
std::deque<Fat::dir_position> Fat::loadDirectories() {
    struct dir_position dir_position;
    std::deque<struct dir_position> directory;
    for (int i = 0; i < fatTable[i]; i++) {
        if(fatTable.at(i) == FAT_DIRECTORY){
            fseek(f, rootDirectoryPosition + p_boot_record->cluster_size * i, SEEK_SET);
            loadDirectory();
            for (int j = 0; j < dir.size(); j++) {
                if(dir.at(j).file_name[0] != '\0'){
                    dir_position.dir = dir.at(j);
                    dir_position.parent = i;
                    directory.push_back(dir_position);
                }
            }
        }
    }
    return directory;
}

/**
 * Vrátí prázdný index adresáře
 * @return prázdný index adresáře
 */
int Fat::getFreeFolderIndex(){
    loadDirectory();
    for (int i = 0; i < dir.size(); i++) {
        if(dir.at(i).file_name[0] == '\0'){
            return i;
        }
    }
    return 0;
}

void Fat::printFatTable() {
    for (int i = 0; i < p_boot_record->cluster_count; ++i) {
        if (fatTable.at(i) == FAT_UNUSED) { printf("%d: %d - FILE_UNUSED \n",i,fatTable.at(i)); }
        else if (fatTable.at(i) == FAT_FILE_END) { printf("%d: %d - FAT_FILE_END \n",i,fatTable.at(i)); }
        else if (fatTable.at(i) == FAT_BAD_CLUSTER) { printf("%d: %d - FAT_BAD_CLUSTER \n",i,fatTable.at(i)); }
        else if (fatTable.at(i) == FAT_DIRECTORY) { printf("%d: %d - FAT_DIRECTORY \n",i,fatTable.at(i)); }
        else { printf("%d: %d \n",i,fatTable.at(i)); }
    }
}

