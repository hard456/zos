
#ifndef ZOS_FAT_H
#define ZOS_FAT_H


class Fat {

private:

    //definice na vyznam hodnot FAT tabulky
    const int FAT_UNUSED = 65535;
    const int FAT_FILE_END = 65534;
    const int FAT_BAD_CLUSTER = 65533;

    //struktura na boot record - nova verze
    struct boot_record {
        char volume_descriptor[251];               //popis
        int fat_type;                             //typ FAT - pocet clusteru = 2^fat_type (priklad FAT 12 = 4096)
        int fat_copies;                           //kolikrat je za sebou v souboru ulozena FAT
        unsigned int cluster_size;                //velikost clusteru ve znacich (n x char) + '/0' - tedy 128 znamena 127 vyznamovych znaku + '/0'
        long root_directory_max_entries_count;    //pocet polozek v root_directory = pocet souboru MAXIMALNE, nikoliv aktualne - pro urceni kde zacinaji data - resp velikost root_directory v souboru
        unsigned int cluster_count;               //pocet pouzitelnych clusteru (2^fat_type - reserved_cluster_count)
        unsigned int reserved_cluster_count;      //pocet rezervovanych clusteru pro systemove polozky
        char signature[4];                        //pro vstupni data od vyucujicich konzistence FAT - "OK","NOK","FAI" - v poradku / FAT1 != FAT2 != FATx / FAIL - ve FAT1 == FAT2 == FAT3, ale obsahuje chyby, nutna kontrola
    };

//struktura na root directory - nova verze
    struct root_directory{
        char file_name[13];             //8+3 format + '/0'
        char file_mod[10];              //unix atributy souboru+ '/0'
        short file_type;                //0 = soubor, 1 = adresar
        long file_size;                 //pocet znaku v souboru
        unsigned int first_cluster;     //cluster ve FAT, kde soubor zacina - POZOR v cislovani root_directory ma prvni cluster index 0 (viz soubor a.txt)
    };

public:

void openFatFile(std::string filename);
void closeFatFile();

};



#endif //ZOS_FAT_H
