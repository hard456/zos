#include <iostream>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <cstring>
#include "Fat.h"

Fat* fat;

/**
 * Porovná zadaný počet argumentů s potřebným počtem pro
 * příslušnou akci
 * @param argc počet argumentů
 * @param requiredNumber požadovaný počet argumentů
 */
void compareArgumentNumber(int argc, int requiredNumber){
    if(argc != requiredNumber){
        fat->closeFatFile();
        std::cout << "Wrong number of arguments." << std::endl;
        exit(EXIT_FAILURE);
    }
}

/**
 * Zavolá příšlé operace pro zpracovaní zadané akce
 * @param argc počet argumentů
 * @param argv argumenty
 */
void callAction(int argc, char **argv){
    std::string action = argv[2];

    fat = new Fat();
    fat->openFatFile(argv[1]);
    fat->loadFile();
    fat->loadDirectory();

    if(action.compare("-a") == 0){
        compareArgumentNumber(argc,5);
        int index = fat->checkPath(argv[4]);
        std::vector<std::string> path = fat->getPathVector(argv[3]);
        if(index == -1 || index == 1){
            std::cout << "PATH NOT FOUND" << std::endl;
        }
        else{
            if(path.at(path.size()-1).length() > 12){
                std::cout << "FILE IS LONGER THAN 12 CHARACTERS" << std::endl;
            }
            else{
                if(fat->addFile(argv[3])){
                    std::cout << "OK" << std::endl;
                }
            }
        }
    }
    else if(action.compare("-f") == 0){
        compareArgumentNumber(argc,4);
        std::string filename;
        int index = fat->checkPath(argv[3]);
        std::string newPath;
        int startFileCluster = fat->clusterStartIndex;
        if(index == 1){
            std::vector<std::string> path = fat->getPathVector(argv[3]);
            for (int i = 0; i < path.size()-1; i++) {
                if(i == 0){
                    newPath += (path.at(i));
                }
                else{
                    newPath += (path.at(i)+"/");
                }
            }
            fat->setRootPosition();
            fat->loadDirectory();
            fat->checkPath((char *)newPath.c_str());
            filename = path.at(path.size()-1);
            int parentCluster = fat->clusterStartIndex;
            fat->deleteFile(filename,startFileCluster);
        }
        else{
            std::cout << "PATH NOT FOUND" << std::endl;
        }
    }
    else if(action.compare("-c") == 0){
        compareArgumentNumber(argc,4);
        int index = fat->checkPath(argv[3]);
        if(index == 1){
            std::cout << fat->filename << " ";
            fat->printfFileClusterIndexes(fat->clusterStartIndex);
        }
        else if(index == -1 || index == 0 || index == -2){
            std::cout << "PATH NOT FOUND" << std::endl;
        }
    }
    else if(action.compare("-m") == 0){
        compareArgumentNumber(argc,5);
        int index = fat->checkPath(argv[4]);
        std::vector<std::string> path = fat->getPathVector(argv[3]);
        if(index == 0 || index == -2){
            if(path.at(path.size()-1).length() > 12){
                std::cout << "FOLDER IS LONGER THAN 12 CHARACTERS" << std::endl;
            }
            else{
                if(fat->addFolder(argv[3])){
                    std::cout << "OK" << std::endl;
                }
            }
        }
        else if(index == -1 || index == 1){
            std::cout << "PATH NOT FOUND" << std::endl;
        }
    }
    else if(action.compare("-r") == 0){
        std::string filename;
        compareArgumentNumber(argc,4);
        std::string newPath;
        int index = fat->checkPath(argv[3]);
        if(index == 0){
            filename = fat->filename;
            if(fat->isFolderEmpty()){
                std::vector<std::string> path = fat->getPathVector(argv[3]);
                for (int i = 0; i < path.size()-1; i++) {
                    if(i == 0){
                        newPath += (path.at(i));
                    }
                    else{
                        newPath += (path.at(i)+"/");
                    }
                }
                fat->setRootPosition();
                fat->loadDirectory();
                fat->checkPath((char *) newPath.c_str());
                fat->deleteFolder(filename);
            }
            else{
                std::cout << "FOLDER IS NOT EMPTY";
            }

        }
        else if(index == -1){
            std::cout << "PATH NOT FOUND" << std::endl;
        }
        else if(index == -2){
            std::cout << "YOU CAN NOT DELETE ROOT" << std::endl;
        }
        else if(index == 1){
            std::cout << "YOU CAN NOT DELETE FILE" << std::endl;
        }
    }
    else if(action.compare("-l") == 0){
        compareArgumentNumber(argc,4);
        int index = fat->checkPath(argv[3]);
        if(index == 1){
            std::cout << fat->filename << ": ";
            fat->printFileContent(fat->clusterStartIndex);
        }
        else if(index == -1 || index == 0 || index == -2){
            std::cout << "PATH NOT FOUND" << std::endl;
        }
    }
    else if(action.compare("-p") == 0){
        compareArgumentNumber(argc,3);
        if(!fat->isItemInFolder()){
            std::cout << "EMPTY" << std::endl;
        }
        else{
            fat->tree();
        }

    }
    else if(action.compare("-def") == 0){
        compareArgumentNumber(argc,3);
        if(!fat->isItemInFolder()){
            std::cout << "EMPTY" << std::endl;
        }
        else{
            std::cout << "BEFORE DEFRAGMENTING" << std::endl;
            fat->printFatTable();
            fat->defragment();
            std::cout << std::endl;
            std::cout << "AFTER DEFRAGMENTING" << std::endl;
            fat->printFatTable();
        }

    }
    else{
        std::cout << "Wrong action argument." << std::endl;
    }
    fat->writeBootRecord();
    fat->writeFatTable();
    fat->closeFatFile();
    fat->freeBootRecord();
}

/**
 * Načte argumenty
 * @param argc počet argumentů
 * @param argv argumenty
 * @return
 */
int main(int argc, char *argv[]) {
    if(argc < 3){
        std::cout << "Wrong number of arguments." << std::endl;
        exit(EXIT_FAILURE);
    }
    callAction(argc, argv);
}

