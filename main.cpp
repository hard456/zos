#include <iostream>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include "Fat.h"

Fat* fat;

void compareArgumentNumber(int argc, int requiredNumber){
    if(argc != requiredNumber){
        fat->closeFatFile();
        std::cout << "Wrong number of arguments." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void callAction(int argc, char **argv){
    std::string action = argv[2];

    fat = new Fat();
    fat->openFatFile(argv[1]);
    fat->loadFile();
    if(!fat->newfile){
        fat->loadRootDirectory();
    }
//    fat->writeBootRecord();

    if(action.compare("-a") == 0){
        compareArgumentNumber(argc,5);

    }
    else if(action.compare("-f") == 0){
        compareArgumentNumber(argc,4);

    }
    else if(action.compare("-c") == 0){
        compareArgumentNumber(argc,4);
        int index = fat->checkPath(argv[3]);
        if(index == 1){
            std::cout << fat->filename << " ";
            fat->printfFileClusterIndexes(fat->startIndex);
        }
        else if(index == -1 || index == 0 || index == -2){
            std::cout << "PATH NOT FOUND" << std::endl;
        }
    }
    else if(action.compare("-m") == 0){
        compareArgumentNumber(argc,5);
        int index = fat->checkPath(argv[4]);
        if(index == 0 || index == -2){
            if(fat->addFolder(argv[3])){
                std::cout << "OK" << std::endl;
            }
            else{
                std::cout << "DIRECTORY IS FULL" << std::endl;
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
                for (int i = 0; i < path.size()-1; ++i) {
                    if(i == 0){
                        newPath += (path.at(i));
                    }
                    else{
                        newPath += (path.at(i)+"/");
                    }
                }
                std::cout << newPath;
                fat->setRootPosition();
                fat->loadRootDirectory();
                fat->checkPath((char *) newPath.c_str());
                fat->deleteFolder(filename);
            }
            else{
                std::cout << "not clear";
            }

        }
        else if(index == -1){
            std::cout << "PATH NOT FOUND" << std::endl;
        }
        else if(index == -2){
            std::cout << "YOU CAN NOT DELETE ROOT" << std::endl;
        }
    }
    else if(action.compare("-l") == 0){
        compareArgumentNumber(argc,4);
        int index = fat->checkPath(argv[3]);
        if(index == 1){
            std::cout << fat->filename << ": ";
            fat->printFileContent(fat->startIndex);
        }
        else if(index == -1 || index == 0 || index == -2){
            std::cout << "PATH NOT FOUND" << std::endl;
        }
    }
    else if(action.compare("-p") == 0){
        compareArgumentNumber(argc,3);

    }
    else{
        std::cout << "Wrong action argument." << std::endl;
        fat->closeFatFile();
        exit(EXIT_FAILURE);
    }
    fat->closeFatFile();
}

int main(int argc, char *argv[]) {
    if(argc < 3){
        std::cout << "Wrong number of arguments." << std::endl;
        exit(EXIT_FAILURE);
    }
    callAction(argc, argv);
}

