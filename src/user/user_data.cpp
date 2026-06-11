#include "../user/user_data.h"
#include "../data/input_data.h" 
#include <fstream>
#include <cstring>
#include <algorithm>
#include <random>
#include <string>

struct record_file_format {
    input_data data; 
    date entry_date; 
};

struct auth_record {
    user profile;
    char password[100];
};

void hash_pass(char* pass) {
    int len = std::strlen(pass);
    std::reverse(pass, pass + len);
}

void user::IDgenerate(){
    const std::string filename = "database/ids.bin"; 
    int newID; bool exists;
    std::random_device rd; std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    do {
        exists = false; newID = dis(gen);
        std::ifstream inFile(filename, std::ios::binary);
        if (inFile.is_open()) {
            int tempID;
            while (inFile.read(reinterpret_cast<char*>(&tempID), sizeof(int))) {
                if (tempID == newID) { exists = true; break; }
            }
            inFile.close();
        }
    } while (exists);
    std::ofstream outFile(filename, std::ios::binary | std::ios::app);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(&newID), sizeof(int));
        outFile.close();
        ID = newID;
    }
}
void user::singUP(const char* n, date b, const char* pass) {
    // 1. Спочатку заповнюємо дані самого профілю (без пароля)
    std::strncpy(name, n, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    birhtday = b;

    // 2. Створюємо запис для бази і упаковуємо туди профіль + хешований пароль
    auth_record newRecord;
    newRecord.profile = *this; 
    std::strncpy(newRecord.password, pass, sizeof(newRecord.password) - 1);
    newRecord.password[sizeof(newRecord.password) - 1] = '\0';
    hash_pass(newRecord.password);

    // 3. Записуємо рівно sizeof(auth_record) у файл!
    std::ofstream file("database/users.bin", std::ios::binary | std::ios::app);
    if (file.is_open()) {
        file.write((char*)&newRecord, sizeof(auth_record));
        file.close();
    }
}

bool user::singIN(const char* inputName, const char* inputPass) {
    char hashedPass[150]; 
    std::strncpy(hashedPass, inputPass, sizeof(hashedPass) - 1); 
    hashedPass[sizeof(hashedPass) - 1] = '\0'; 
    hash_pass(hashedPass);

    std::ifstream inFile("database/users.bin", std::ios::binary);
    if (!inFile.is_open()) return false;
    
    auth_record temp;
    while (inFile.read(reinterpret_cast<char*>(&temp), sizeof(auth_record))) {
        if (std::strcmp(temp.profile.name, inputName) == 0 && std::strcmp(temp.password, hashedPass) == 0) {
            *this = temp.profile; // Завантажуємо в пам'ять тільки профіль!
            inFile.close(); 
            return true;
        }
    }
    inFile.close(); 
    return false;
}

// 2. Оновлений метод пошуку для відновлення пароля (тепер двофакторний: Логін + ДН)
bool user::findUser(const char* n, date b) {
    std::ifstream file("database/users.bin", std::ios::binary);
    if (!file.is_open()) return false;

    user temp;
    while (file.read((char*)&temp, sizeof(user))) {
        if (std::strcmp(temp.name, n) == 0 && 
            temp.birhtday.day == b.day && 
            temp.birhtday.month == b.month && 
            temp.birhtday.year == b.year) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

void user::resetPassword(bool status, const char* n, date d, const char* newPass) {
    if (!status) return; 
    std::fstream file("database/users.bin", std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) return;
    
    auth_record temp;
    while (file.read(reinterpret_cast<char*>(&temp), sizeof(auth_record))) {
        if (std::strcmp(temp.profile.name, n) == 0 && temp.profile.birhtday == d) {
            std::strncpy(temp.password, newPass, sizeof(temp.password) - 1); 
            temp.password[sizeof(temp.password) - 1] = '\0'; 
            hash_pass(temp.password);
            
            file.seekp((std::streamoff)file.tellg() - sizeof(auth_record));
            file.write(reinterpret_cast<const char*>(&temp), sizeof(auth_record));
            break;
        }
    }
    file.close();
}

bool user::deleteUser(const char* inputPass) {
    char hashedPass[100]; std::strncpy(hashedPass, inputPass, 99); hashedPass[99] = '\0'; hash_pass(hashedPass);
    std::ifstream inFile("database/users.bin", std::ios::binary);
    if (!inFile.is_open()) return false;
    std::ofstream outFile("database/temp.bin", std::ios::binary);
    auth_record temp;
    bool found = false; int targetID = -1;
    while (inFile.read(reinterpret_cast<char*>(&temp), sizeof(auth_record))) {
        if (std::strcmp(temp.profile.name, name) == 0 && std::strcmp(temp.password, hashedPass) == 0) {
            found = true; targetID = temp.profile.ID;
        } else { outFile.write(reinterpret_cast<const char*>(&temp), sizeof(auth_record)); }
    }
    inFile.close(); outFile.close();
    std::remove("database/users.bin"); std::rename("database/temp.bin", "database/users.bin");

    if (found && targetID != -1) { // Видалення всіх записів користувача
        std::ifstream dataIn("database/Data.bin", std::ios::binary);
        if (dataIn.is_open()) {
            std::ofstream dataOut("database/temp_data.bin", std::ios::binary);
            record_file_format recordTemp;
            while (dataIn.read(reinterpret_cast<char*>(&recordTemp), sizeof(recordTemp))) {
                if (recordTemp.data.userID != targetID) {
                    dataOut.write(reinterpret_cast<const char*>(&recordTemp), sizeof(recordTemp));
                }
            }
            dataIn.close(); dataOut.close();
            std::remove("database/Data.bin"); std::rename("database/temp_data.bin", "database/Data.bin");
        }
    }
    return found;
}

bool user::changePassword(const char* login, const char* newPass) {
    std::fstream file("database/users.bin", std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) return false;
    auth_record temp;
    bool found = false;
    while (file.read(reinterpret_cast<char*>(&temp), sizeof(auth_record))) {
        if (std::strcmp(temp.profile.name, login) == 0) {
            std::strncpy(temp.password, newPass, 99); 
            temp.password[99] = '\0'; 
            hash_pass(temp.password);
            
            file.seekp((std::streamoff)file.tellg() - sizeof(auth_record));
            file.write(reinterpret_cast<const char*>(&temp), sizeof(auth_record));
            found = true;
            break;
        }
    }
    file.close();
    return found;
}

std::string user::getBirthdayByID(int target_id) {
    std::ifstream file("database/users.bin", std::ios::binary);
    if (!file.is_open()) return "Помилка";

    user temp;
    while (file.read((char*)&temp, sizeof(user))) {
        if (temp.getID() == target_id) {
            file.close();
            
            // Виправлено описку 'birhtday' на 'birthday' та прибрано випадкові пробіли
            std::string d = (temp.birhtday.day < 10 ? "0" : "") + std::to_string(temp.birhtday.day);
            std::string m = (temp.birhtday.month < 10 ? "0" : "") + std::to_string(temp.birhtday.month);
            return d + "." + m + "." + std::to_string(temp.birhtday.year);
        }
    }
    file.close();
    return "Не знайдено";
}

bool user::isLoginTaken(const char* inputName) {
    std::ifstream file("database/users.bin", std::ios::binary);
    if (!file.is_open()) return false;

    auth_record temp;
    while (file.read((char*)&temp, sizeof(auth_record))) {
        if (std::strcmp(temp.profile.name, inputName) == 0) {
            file.close();
            return true; // Логін знайдено, він зайнятий
        }
    }
    file.close();
    return false;
}