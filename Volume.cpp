#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <cstring>
#include <string>
#include <conio.h>
#include<math.h>
using namespace std;

struct bootSectorData{
    char jmpBootcode[3]{0};//không sử dụng
    char groupID[9] = {'4','4','2','5','9','2','5','9','9'};
    short szOfSector = 512;
    short szOfCluster = 4;
    short numOfEntry = 120;
    unsigned int numSectOfVol;
    char nameVol[32]{0};
    char password[32]{0};
    short endOfSystemArea = 512 + 120*64; 
    char bootProgram[422]{0};// không sử dụng
    char EndOfBootSector[2] = {(char)0xAA,(char)0x55}; 
};
struct date{
    short day;
    short month;
    short year;
    char hour;
    char minute;
    char second;
};
struct Entry{
    char name[32]{0};
    bool status = 0;
    date importDate = {0,0,0,0,0,0};
    unsigned short clusterStart = 0;
    unsigned int szOfFile = 0;
    char password[16]{0};
};

void hashPass(char pass[], int size) {
    for (int i = 0; i < size; i++){
        if (pass[i] == 0) pass[i] = '0';
    }
    char groupID[9] = { '4','4','2','5','9','2','5','9','9' };
    for (int i = 0; i < size; i++) {
        pass[i] = (char)(((int)pass[i] + (int)groupID[i % 9] + (i*7749 % 256)) % 256);
    }
}
void getPassword(char s[], int size){
    char ch = 0;
    memset(s,0,size);
    fflush(stdin);
    while (ch != 13)
    {
        fflush(stdin);
        ch = getch();
        if (ch <= 0) getch();
        else if (ch > 31 && ch < 127){
            if ((int)strlen(s) < size - 1){
                cout << '*';
                s[strlen(s)] = ch;
            }
        }else if (ch == 8){
            if (s[0]){
                s[strlen(s) - 1] = 0;
                cout << ch << ' ' << ch;
            }
        }
    }
    cout << endl;
    fflush(stdin); 
}
bool checkPassword(char pass[], char password[],int size){
    for (int i = 0; i < size; i++){
        if (pass[i] != password[i]) {
            cout << "Wrong password!" << endl;
            return 1;
        }
    }
    cout << "Correct Password!" << endl;
    return 0;
}

class Volume{
public:
    //system
    bootSectorData bootSect;
    Entry* RDET;
public:
    Volume(){
        this->RDET = new Entry[this->bootSect.numOfEntry];
    }
    ~Volume(){
        delete[] RDET;
    }
    bool createVolume(unsigned int szVol, char* name){
        try{
            bootSect.numSectOfVol = (szVol*pow(1024,3))/bootSect.szOfSector;// Tinh so luong sector cua volume
            for (int i = 0; i < 32; i++){
                bootSect.nameVol[i] = name[i];  //Gan ten
            }
            //Ghi bootsector
            ofstream fo(bootSect.nameVol,ios::out|ios::binary);
            fo.write(bootSect.jmpBootcode,3);
            fo.write(bootSect.groupID,9);
            fo.write((char*)&bootSect.szOfSector,2);
            fo.write((char*)&bootSect.szOfCluster,2);
            fo.write((char*)&bootSect.numOfEntry,2);
            fo.write((char*)&bootSect.numSectOfVol,4);
            fo.write(bootSect.nameVol,32);
            fo.write(bootSect.password,32);
            fo.write((char*)&bootSect.endOfSystemArea,2);
            fo.write(bootSect.bootProgram,422);
            fo.write(bootSect.EndOfBootSector,2);
            //Ghi bang RDET
            //this->RDET = new Entry[bootSect.numOfEntry];
            for (int i = 0; i < bootSect.numOfEntry; i++){
                fo.write(RDET[i].name,32);
                //fo.write(RDET[i].extension,4);
                fo.write((char*)&RDET[i].status,1);
                fo.write((char*)&RDET[i].importDate,9);
                fo.write((char*)&RDET[i].clusterStart,2);
                fo.write((char*)&RDET[i].szOfFile,4);
                fo.write(RDET[i].password,16);
            }
            //Lap day cac vi tri trong cua volume bang null
            int szData = bootSect.numSectOfVol * bootSect.szOfSector 
                - bootSect.szOfSector - bootSect.numOfEntry * 64;
            char* data = new char[szData]{0};
            fo.write(data,szData);
            fo.close();
            //giai phong bo nho
            //delete[] pass;
            delete[] data;
            return 0;
        }catch (runtime_error){
            return 1;
        }
    }
    void readRDET(){
        //Doc bang RDET
        fstream vol(this->bootSect.nameVol,ios::in|ios::binary);
        vol.seekg(this->bootSect.szOfSector);   
        for (int i = 0; i < bootSect.numOfEntry; i++){
            vol.read(RDET[i].name, 32);
            vol.read((char*)&RDET[i].status, 1);
            vol.read((char*)&RDET[i].importDate, 9);
            vol.read((char*)&RDET[i].clusterStart, 2);
            vol.read((char*)&RDET[i].szOfFile, 4);
            vol.read(RDET[i].password, 16);
        }
        vol.close();
    }
    bool modifiedVol(char* name, string path){
        fstream _vol(path + name,ios::in|ios::binary);
        _vol.seekg(54);
        _vol.read(bootSect.password, 32);
        _vol.close();
        try{
            if (bootSect.password[0] != 0){
                cout << "Enter current password of volume: ";
                char* pass = new char[32]{0};
                getPassword(pass,32);
                hashPass(pass,32);
                if (checkPassword(pass, bootSect.password, 32) == 1){
                    fstream __vol(path + name,ios::in|ios::out|ios::binary);
                    __vol.seekg(54);
                    memset(bootSect.password,0,32);
                    __vol.write(bootSect.password, 32);
                    __vol.close();
                    return 1;
                }
            }
            for (int i = 0; i < 32; i++){
                bootSect.nameVol[i] = name[i];  //Gan ten
            }
            //Doc bang RDET
            readRDET();
            //lay size volume
            fstream vol(path + this->bootSect.nameVol,ios::in|ios::binary);
            vol.seekg(0,vol.end);
            this->bootSect.numSectOfVol = vol.tellg() / 512;
            vol.close();
            return 0;
        }catch (runtime_error){
            return 1;
        }
    }
    bool listFile(){
        try{
            readRDET();
            for (int i = 0; i < bootSect.numOfEntry; i++){
                if (RDET[i].status != 0){
                    cout << "Name: " << RDET[i].name << "\t\tSize: " 
                        << RDET[i].szOfFile << " B\t\t Import Time: "
                        << RDET[i].importDate.day << "/" << RDET[i].importDate.month 
                        << "/" << RDET[i].importDate.year << " " << (int)RDET[i].importDate.hour << ":"
                        << (int)RDET[i].importDate.minute << ":" <<(int)RDET[i].importDate.second;
                    if (RDET[i].password[0] != 0) cout << "\t\tPassword: ON" << endl;
                    else cout << "\t\tPassword: OFF" << endl;
                }
            }
            return 0;
        }catch(runtime_error){
            return 1;
        }
    }
    bool importFile(string filename, string path){
        readRDET();
        for (int i = 0; i < bootSect.numOfEntry; i++){
            string _name = string(RDET[i].name);
            // kiem tra file co ton tai trong volume khong
            if (_name == filename){
                cout << "ERROR - 101 | File have already exist" << endl;
                return 101;
            }
        }
        //Lay 1 entry rong de luu thong tin cua tap tin
        int import_Entry = 0;
        for (int i = 0; i < bootSect.numOfEntry; i++){
            if (RDET[i].status == 0){
                import_Entry = i;
                break;
            }
            if (i == bootSect.numOfEntry - 1){
                cout << "ERROR - 102 | Out of memory!" << endl;
                return 102;
            }
        }
        //Lay ten va password cua tap tin
        for (int i = 0; i < filename.length(); i++){
            RDET[import_Entry].name[i] = filename[i];
        }
        //Doi trang thai cua tap tin
        RDET[import_Entry].status = 1;
        //Lay ngay thang nam import tap tin
        time_t info = time(NULL);
        tm* now = localtime(&info);
        RDET[import_Entry].importDate.day = now->tm_mday;
        RDET[import_Entry].importDate.month = now->tm_mon + 1;
        RDET[import_Entry].importDate.year = now->tm_year + 1900;
        RDET[import_Entry].importDate.hour = now->tm_hour;
        RDET[import_Entry].importDate.minute = now->tm_min;
        RDET[import_Entry].importDate.second = now->tm_sec;
        //Lay Cluster bat dau
        if (import_Entry == 0) RDET[import_Entry].clusterStart = 5;
        else RDET[import_Entry].clusterStart = RDET[import_Entry - 1].clusterStart + ceil((double)(ceil((double)RDET[import_Entry - 1].szOfFile / 512)) / 4);
        //Lay kich thuoc cua tap tin
        ifstream importFile(path + filename, std::ifstream::binary);
        importFile.seekg (0, importFile.end);
        int length = importFile.tellg();
        importFile.seekg (0, importFile.beg);
        RDET[import_Entry].szOfFile = length;
        //Mo volume de ghi du lieu vao
        fstream _vol(this->bootSect.nameVol,ios::in|ios::out|ifstream::binary);
        //Cap nhat bang thu muc goc
        _vol.seekg(512 + 64 * import_Entry);  //dua con tro file den vi tri cua entry can cap nhat    
        _vol.write(RDET[import_Entry].name,32);
        _vol.write((char*)&RDET[import_Entry].status,1);           //Cap nhat thong tin entry import
        _vol.write((char*)&RDET[import_Entry].importDate,9);
        _vol.write((char*)&RDET[import_Entry].clusterStart,2);
        _vol.write((char*)&RDET[import_Entry].szOfFile,4);
        _vol.write(RDET[import_Entry].password,16);
        //Ghi du lieu cua tap tin muon import vao volume
        _vol.seekg(RDET[import_Entry].clusterStart * bootSect.szOfCluster * bootSect.szOfSector); //Dua con tro file den cluster can ghi du lieu
        //Lay du lieu cua file import   
        char * buffer = new char [RDET[import_Entry].szOfFile];
        importFile.read (buffer,RDET[import_Entry].szOfFile);
        importFile.close();
        //Ghi du lieu vao volume     
        _vol.write(buffer, RDET[import_Entry].szOfFile);
        _vol.close();
        //giai phong bo nho
        delete[] buffer;
        //delete[] RDET;
        return 0;
    }
    bool outportFile(string filename, string path){
        readRDET();
        for (int i = 0; i < bootSect.numOfEntry; i++){
            string _name = string(RDET[i].name);
            // kiem tra file co ton tai trong volume khong
            if (_name == filename){
                if (RDET[i].password[0] != 0){
                    cout << "Enter password: ";
                    char* pass = new char [16]{0};
                    getPassword(pass,16);
                    hashPass(pass,16);
                    if (checkPassword(pass, RDET[i].password, 16) == 1){
                        delete[] pass;
                        return 1;
                    }
                    delete[] pass;
                }
                //Lay du lieu file trong volume
                char* buffer = new char [RDET[i].szOfFile];
                fstream _vol(this->bootSect.nameVol,ios::in|ios::binary);
                _vol.seekg(RDET[i].clusterStart * bootSect.szOfCluster * bootSect.szOfSector);
                _vol.read(buffer,RDET[i].szOfFile);
                _vol.close();
                //Chep du lieu ra ben ngoai
                ofstream fo(path + filename,ios::out|ios::binary);
                fo.write(buffer,RDET[i].szOfFile);
                fo.close();
                delete[] buffer;
                break;
            }
            if (i == bootSect.numOfEntry - 1) {
                cout << "ERROR - 103 | File does not exist!" << endl;
                return 103;
            }
        }
        return 0;
    }
    bool deleteFile(string filename){
        readRDET();
        for (int i = 0; i < bootSect.numOfEntry; i++){
            string _name = string(RDET[i].name);
            // kiem tra file co ton tai trong volume khong
            if (_name == filename){
                if (RDET[i].password[0] != 0){
                    cout << "Enter password: ";
                    char* pass = new char [16]{0};
                    getPassword(pass,16);
                    hashPass(pass,16);
                    if (checkPassword(pass, RDET[i].password, 16) == 1){
                        delete[] pass;
                        return 1;
                    }
                }
                //xoa vung du lieu
                char* buffer = new char [RDET[i].szOfFile]{0};
                fstream _vol(this->bootSect.nameVol,ios::in|ios::out|ios::binary);
                _vol.seekg(RDET[i].clusterStart * bootSect.szOfCluster * bootSect.szOfSector);
                _vol.write(buffer,RDET[i].szOfFile);
                delete[] buffer;
                //Doc du lieu tu entry ke tiep entry muon xoa roi ghi de len truoc
                _vol.seekg(RDET[i + 1].clusterStart * bootSect.szOfCluster * bootSect.szOfSector);
                char* data = new char[bootSect.numSectOfVol*bootSect.szOfSector - RDET[i + 1].clusterStart * bootSect.szOfCluster * bootSect.szOfSector];
                _vol.read(data,bootSect.numSectOfVol*bootSect.szOfSector - RDET[i + 1].clusterStart * bootSect.szOfCluster * bootSect.szOfSector);
                _vol.seekg(RDET[i].clusterStart * bootSect.szOfCluster * bootSect.szOfSector);
                _vol.write(data,bootSect.numSectOfVol*bootSect.szOfSector - RDET[i + 1].clusterStart * bootSect.szOfCluster * bootSect.szOfSector);
                char* empty = new char[RDET[i].szOfFile]{0};
                _vol.write(empty, RDET[i].szOfFile);
                delete[] empty;
                delete[] data;
                //cap nhat Entry
                memset(RDET[i].name, (char)0, 32);
                RDET[i].status = 0;
                RDET[i].importDate = {0, 0, 0, 0, 0, 0};
                //RDET[i].clusterStart = 0;
                RDET[i].szOfFile = 0;
                memset(RDET[i].password, (char)0, 16);
                //Cap nhat bang thu muc goc
                for (int j = i; j < bootSect.numOfEntry - 1; j++){
                    int tmp = RDET[j].clusterStart;
                    RDET[j] = RDET[j + 1];
                    RDET[j].clusterStart = tmp;
                }
                _vol.seekg(512 + 64 * i);  //dua con tro file den vi tri cua entry can cap nhat    
                for (int j = i; j < bootSect.numOfEntry;j++){
                    _vol.write(RDET[j].name,32);
                    _vol.write((char*)&RDET[j].status,1);           //Cap nhat thong tin entry
                    _vol.write((char*)&RDET[j].importDate,9);
                    _vol.write((char*)&RDET[j].clusterStart,2);
                    _vol.write((char*)&RDET[j].szOfFile,4);
                    _vol.write(RDET[j].password,16);
                }
                _vol.close();
                break;
            }
            if (i == bootSect.numOfEntry - 1) {
                cout << "ERROR - 103 | File does not exist!" << endl;
                return 103;
            }
        }
        return 0;
    }
    bool setupFilePassword(string filename){
        for (int i = 0; i < bootSect.numOfEntry; i++){
            string _name = string(RDET[i].name);
            // kiem tra file co ton tai trong volume khong
            if (_name == filename){
                if (RDET[i].password[0] != 0){
                    cout << "Update password!" << endl;
                    if(this->changFilePassword(filename) == 0) return 0;
                    return 1;
                }else{  
                    cout << "Create password!" << endl;
                    cout << "Enter new password: ";
                    char* pass = new char[16]{0};
                    getPassword(pass,16);
                    cout << "Re-enter new password: ";
                    char* tmp = new char[16];
                    getPassword(tmp, 16);
                    if (checkPassword(pass, tmp, 16) == 0){
                        hashPass(pass, 16);
                        for (int j = 0;j < 16; j++){
                            RDET[i].password[j] = pass[j];
                        }
                    }
                    //Cap nhat bang thu muc goc
                    fstream _vol(this->bootSect.nameVol,ios::in|ios::out|ios::binary);
                    _vol.seekg(512 + 64 * i);  //dua con tro file den vi tri cua entry can cap nhat    
                    _vol.write(RDET[i].name,32);
                    _vol.write((char*)&RDET[i].status,1);           //Cap nhat thong tin entry
                    _vol.write((char*)&RDET[i].importDate,9);
                    _vol.write((char*)&RDET[i].clusterStart,2);
                    _vol.write((char*)&RDET[i].szOfFile,4);
                    _vol.write(RDET[i].password,16);
                    _vol.close();
                    delete[] pass;
                    delete[] tmp;
                    return 0;
                }
            }   
            if (i == bootSect.numOfEntry - 1) {
                cout << "ERROR - 103 | File does not exist!" << endl;
                return 103;
            }
        }
        return 0;
    }
    bool changFilePassword(string filename){
        for (int i = 0; i < bootSect.numOfEntry; i++){
            string _name = string(RDET[i].name);
            // kiem tra file co ton tai trong volume khong
            if (_name == filename){
            
                char* pass = new char[16]{0};
                cout << "Enter current password of file: ";
                getPassword(pass,16);
                hashPass(pass,16);
                if (checkPassword(pass, RDET[i].password, 16) == 1){
                    return 1;
                }
                
                memset(pass,0,16);
                cout << "Enter new password: ";
                getPassword(pass,16);
                cout << "Re-enter new password: ";
                char* tmp = new char[16];
                getPassword(tmp, 16);
                if (checkPassword(pass, tmp, 16) == 0){
                    hashPass(pass, 16);
                //cout << pass << endl
                for (int j = 0;j < 16; j++){
                    RDET[i].password[j] = pass[j];
                }
                //Cap nhat bang thu muc goc
                fstream _vol(this->bootSect.nameVol,ios::in|ios::out|ios::binary);
                _vol.seekg(512 + 64 * i);  //dua con tro file den vi tri cua entry can cap nhat    
                _vol.write(RDET[i].name,32);
                _vol.write((char*)&RDET[i].status,1);           //Cap nhat thong tin entry
                _vol.write((char*)&RDET[i].importDate,9);
                _vol.write((char*)&RDET[i].clusterStart,2);
                _vol.write((char*)&RDET[i].szOfFile,4);
                _vol.write(RDET[i].password,16);
                _vol.close();
                }
                delete[] tmp;
                break;
            }
            if (i == bootSect.numOfEntry - 1) {
                cout << "ERROR - 103 | File does not exist!" << endl;
                return 103;
            }
        }
        return 0;
    }
    bool setupVolumePassword(){
        if (this->bootSect.password[0] == 0){
            cout << "Create password for volume!" << endl;
            cout << "Enter new password: ";
            char* pass = new char[32]{0};
            getPassword(pass,32);
            cout << "Re-enter new password: ";
            char* tmp = new char[32];
            getPassword(tmp, 32);
            if (checkPassword(pass, tmp, 32) != 0) return 1;
            hashPass(pass, 32);
            for (int j = 0;j < 32; j++){
                bootSect.password[j] = pass[j];
            }
            fstream vol(bootSect.nameVol,ios::in|ios::out|ios::binary);
            vol.seekg(54);
            vol.write(bootSect.password,32);
            vol.close();
            delete[] pass;
        }else if (bootSect.password[0] != 0){
            cout << "Update password of volume!" << endl;
            cout << "Enter current password of volume: ";
            char* pass = new char[32]{0};
            getPassword(pass,32);
            hashPass(pass,32);
            if (checkPassword(pass, bootSect.password, 32) == 1){
                return 1;
            }
            memset(pass,0,32);
            cout << "Enter new password: ";
            getPassword(pass,32);
            cout << "Re-enter new password: ";
            char* tmp = new char[32];
            getPassword(tmp, 32);
            if (checkPassword(pass, tmp, 32) != 0) return 1;
            hashPass(pass, 32);
            for (int j = 0;j < 32; j++){
                bootSect.password[j] = pass[j];
            }
            fstream vol(bootSect.nameVol,ios::in|ios::out|ios::binary);
            vol.seekg(54);
            vol.write(bootSect.password,32);
            vol.close();
            delete[] pass;
            delete[] tmp;
        }
        return 0;
    }
};

int main(){
    int choice = 9;
    Volume E;
    cout << "|Program of design and oganize architecture of volume & file|" << endl;
    cout << "|___________________________V 0.0.1_________________________|" << endl;
    cout << "|___________________________________________________________|" << endl;
    
    while (choice != 0){
        if (choice == 1){
            cout << "[1]. Create new volume" << endl;
            cout << "[2]. Modified existing volume" << endl;
            cout << "Your choice: ";
            int tmp;
            cin >> tmp;
            cin.ignore(1);
            if (tmp == 1){
                char* name = new char [32]{0};
                //cin.ignore(1);
                cout << "Enter name: ";
                cin.getline(name,32);
                cout << "Enter size of volume (GB): ";
                unsigned int szVol;
                cin >> szVol;
                if (E.createVolume(szVol,name) == 0){
                    cout << "Create success volume "<< E.bootSect.nameVol << endl;
                    cout << "Opening volume "<< E.bootSect.nameVol << endl;
                }
                delete[] name;
            }else if (tmp == 2){
                cout << "Enter name: ";
                char* name = new char [32]{0};
                cin.getline(name,32);
                cout << "Enter directory: ";
                string path;
                cin >> path;
                if (E.modifiedVol(name, path) == 0)
                cout << "Opening volume "<< E.bootSect.nameVol << endl;
            }
        }
        if (choice == 2){
            E.setupVolumePassword();
        }
        if (choice == 3){
            cout << "Files on volume " << E.bootSect.nameVol << ":" << endl;
            E.listFile();
        }
        if (choice == 4){
            cout << "Enter the name of file: ";
            string name;
            cin >> name;
            
            if (E.setupFilePassword(name) == 0)cout << "Create password success" << endl;  
        }
        if (choice == 5){
            cout << "Enter the name of import file: ";
            string name;
            cin >> name;
            cout << "Enter directory contains import file: ";
            string path;
            cin >> path;
            if (E.importFile(name, path) == 0) cout << "Import success" << endl;
        }
        if (choice == 6){
            cout << "Enter the name of export file: ";
            string name;
            cin >> name;
            cout << "Enter directory: ";
            string path;
            cin >> path;
            if (E.outportFile(name,path) == 0) cout << "Export success!" << endl;
        }
        if (choice == 7){
            cout << "Enter the name of file: ";
            string name;
            cin >> name;
            if (E.deleteFile(name) == 0) cout << "Delete success!" << endl;   
        }
        if (choice == 0){
            break;
        }
        cout << "--------------------------------------------------------------------------" << endl;
        cout << " _________________________________________________" << endl;
        cout << "|                                                 |" << endl;
        cout << "|[1] Create / Modified VOLUME                     |" << endl;
        cout << "|[2] Create / Change Password of VOLUME           |" << endl;
        cout << "|[3] List files of VOLUME                         |" << endl;
        cout << "|[4] Create / Change password of a file on VOLUME |" << endl;
        cout << "|[5] Import a file from outside to VOLUME         |" << endl;
        cout << "|[6] Export a file frome VOLUME to outside        |" << endl;
        cout << "|[7] Delete a file                                |" << endl;
        cout << "|[0] Exit program                                 |" << endl;
        cout << "|_________________________________________________|" << endl;
        cout << "Your choice: ";
        cin >> choice;
        cin.ignore(1);
    }
    return 0;  
}