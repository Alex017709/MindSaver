#ifndef USER_DATA_H
#define USER_DATA_H

#include "../date/date.h"
#include <string>

class user {
private:
    int ID;
    char name[250];
    date birhtday;
public: 
    int getID() const { return ID; } 
    void IDgenerate();
    bool singIN(const char* inputName, const char* inputPass);
    void singUP(const char* name, date b, const char* pass);
    bool findUser(const char* name, date b);
    void resetPassword(bool status, const char* n, date d, const char* newPass);
    bool deleteUser(const char* inputPass);
    bool changePassword(const char* login, const char* newPass);
    std::string getBirthdayByID(int target_id);
    bool isLoginTaken(const char* inputName);
};

#endif