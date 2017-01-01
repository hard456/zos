#include <iostream>
#include <stdlib.h>
#include <string>

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
    fat->loadFileStructure();
    fat->writeBootRecord();

    if(action.compare("-a") == 0){
        compareArgumentNumber(argc,5);

    }
    else if(action.compare("-f") == 0){
        compareArgumentNumber(argc,4);

    }
    else if(action.compare("-c") == 0){
        compareArgumentNumber(argc,4);

    }
    else if(action.compare("-m") == 0){
        compareArgumentNumber(argc,5);

    }
    else if(action.compare("-r") == 0){
        compareArgumentNumber(argc,4);

    }
    else if(action.compare("-l") == 0){
        compareArgumentNumber(argc,4);

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

