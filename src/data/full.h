#ifndef FULL_H
#define FULL_H

#include "../date/date.h"

struct FullRecord {
    int ID;
    int userID; 
    int hetch;
    short nastry; 
    short energy;
    short stress;
    char note[300];
    date entry_date;
};

#endif