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
            fat->fileFatIndexes(fat->startIndex);
        }
        else if(index == -1 || index == 0){
            std::cout << "PATH NOT FOUND" << std::endl;
        }
    }
    else if(action.compare("-m") == 0){
        compareArgumentNumber(argc,5);
        int index = fat->checkPath(argv[4]);
        if(index == 0){
            fat->addFolder(argv[3],argv[4]);
        }
        else if(index == -1 || index == 1){
            std::cout << "PATH NOT FOUND" << std::endl;
        }
    }
    else if(action.compare("-r") == 0){
        compareArgumentNumber(argc,4);
        int index = fat->checkPath(argv[3]);
        if(index == 0){

        }
        else if(index == -1){
            std::cout << "PATH NOT FOUND" << std::endl;
        }
    }
    else if(action.compare("-l") == 0){
        compareArgumentNumber(argc,4);
        int index = fat->checkPath(argv[3]);
        if(index == 1){
            std::cout << fat->filename << ": ";
            fat->printFileContent(fat->startIndex);
        }
        else if(index == -1 || index == 0){
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

