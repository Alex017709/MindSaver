#include "input_data.h"
#include <fstream>
#include <cstring>
#include <random>
#include <cstdio>

const char* txt_array[10] = {"party", "birthday", "shopping", "fishing", 
    "watch film", "listening music", "work", "teaching", "relax", "play game"};

struct record_file_format {
    input_data data; 
    date entry_date; 
};

const char* input_data::get_txt(int index) {
    if (index >= 0 && index < 10) return txt_array[index];
    return "";
}

void input_data::input(short n, short e, short s, const char* text_note, int h, int currentUserID){
    nastry = n;
    energy = e;
    stress = s;
    hetch = h;
    userID = currentUserID; // Прив'язка до поточного користувача
    std::strncpy(note, text_note, 299);
    note[299] = '\0';

    IDgenerate();

    record_file_format file_record;
    file_record.data = *this;         
    file_record.entry_date.PCdate();  

    std::ofstream outFile("database/Data.bin", std::ios::binary | std::ios::app);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(&file_record), sizeof(record_file_format));
        outFile.close();
    }
}

std::vector<FullRecord> input_data::get_all_data(int currentUserID){
    std::vector<FullRecord> records;
    std::ifstream inFile("database/Data.bin", std::ios::binary);
    if (!inFile.is_open()) return records;
    
    record_file_format temp;
    while (inFile.read(reinterpret_cast<char*>(&temp), sizeof(record_file_format))) {
        if (temp.data.userID == currentUserID) { // Тільки свої записи
            FullRecord rec;
            rec.ID = temp.data.ID; 
            rec.userID = temp.data.userID;
            rec.hetch = temp.data.hetch;
            rec.nastry = temp.data.nastry;
            rec.energy = temp.data.energy;
            rec.stress = temp.data.stress;
            std::strncpy(rec.note, temp.data.note, 300);
            rec.entry_date = temp.entry_date;
            records.push_back(rec);
        }
    }
    inFile.close();
    return records;
}

StatsResult input_data::get_statistic(date start, date end, int currentUserID){
    StatsResult res = {0, 0, 0, 0};
    std::ifstream inFile("database/Data.bin", std::ios::binary);
    if (!inFile.is_open()) return res;
    
    record_file_format temp;
    float n = 0, e = 0, s = 0;
    while (inFile.read(reinterpret_cast<char*>(&temp), sizeof(record_file_format))) {
        if (temp.data.userID == currentUserID && temp.entry_date >= start && temp.entry_date <= end) {
            n += temp.data.nastry; e += temp.data.energy; s += temp.data.stress; 
            res.count++;
        }
    }
    inFile.close();
    if (res.count > 0) {
        res.avg_nastry = n / res.count; res.avg_energy = e / res.count; res.avg_stress = s / res.count;
    }
    return res;
}

void input_data::IDgenerate(){
    const std::string filename = "database/idb.bin"; 
    int newID; bool exists;
    std::random_device rd; std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999);
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

std::vector<FullRecord> input_data::get_by_hashtag(int h, int currentUserID) {
    std::vector<FullRecord> records;
    std::ifstream inFile("database/Data.bin", std::ios::binary);
    if (!inFile.is_open()) return records;
    record_file_format temp;
    while (inFile.read(reinterpret_cast<char*>(&temp), sizeof(record_file_format))) {
        if (temp.data.userID == currentUserID && temp.data.hetch == h) {
            FullRecord rec = {temp.data.ID, temp.data.userID, temp.data.hetch, temp.data.nastry, temp.data.energy, temp.data.stress};
            std::strncpy(rec.note, temp.data.note, 300);
            rec.entry_date = temp.entry_date;
            records.push_back(rec);
        }
    }
    inFile.close();
    return records;
}

bool input_data::edit_record(date d, short n, short e, short s, const char* msg, int h, int currentUserID) {
    std::fstream file("database/Data.bin", std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) return false;
    record_file_format temp;
    while (file.read(reinterpret_cast<char*>(&temp), sizeof(record_file_format))) {
        if (temp.data.userID == currentUserID && temp.entry_date == d) {
            temp.data.nastry = n; temp.data.energy = e; temp.data.stress = s; temp.data.hetch = h;
            std::strncpy(temp.data.note, msg, 299);
            file.seekp((std::streamoff)file.tellg() - sizeof(record_file_format));
            file.write(reinterpret_cast<const char*>(&temp), sizeof(record_file_format));
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

bool input_data::delete_record(date d, int currentUserID) {
    std::ifstream inFile("database/Data.bin", std::ios::binary);
    if (!inFile.is_open()) return false;
    std::ofstream outFile("database/temp_data.bin", std::ios::binary);
    record_file_format temp;
    bool found = false;
    while (inFile.read(reinterpret_cast<char*>(&temp), sizeof(record_file_format))) {
        if (temp.data.userID == currentUserID && temp.entry_date == d) { found = true; } 
        else { outFile.write(reinterpret_cast<const char*>(&temp), sizeof(record_file_format)); }
    }
    inFile.close(); outFile.close();
    std::remove("database/Data.bin");
    std::rename("database/temp_data.bin", "database/Data.bin");
    return found;
}

void input_data::input_with_date(short n, short e, short s, const char* text_note, int h, int currentUserID, date d){
    nastry = n;
    energy = e;
    stress = s;
    hetch = h;
    userID = currentUserID; 
    std::strncpy(note, text_note, 299);
    note[299] = '\0';

    IDgenerate();

    record_file_format file_record;
    file_record.data = *this;         
    file_record.entry_date = d;  // ТУТ МИ ЗБЕРІГАЄМО САМЕ ДАТУ З КАЛЕНДАРЯ

    std::ofstream outFile("database/Data.bin", std::ios::binary | std::ios::app);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(&file_record), sizeof(record_file_format));
        outFile.close();
    }
}