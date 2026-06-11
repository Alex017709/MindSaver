#ifndef INPUT_DATA_H
#define INPUT_DATA_H

#include <vector>
#include "../date/date.h"
#include "full.h"  
#include "stat.h"

class input_data {
private:
    int ID;
    int hetch;
    short nastry; 
    short energy;
    short stress;
    char note[300];

public:int 
    userID; 
    static const char* get_txt(int index);
    void input(short n, short e, short s, const char* text_note, int h, int currentUserID);
    std::vector<FullRecord> get_all_data(int currentUserID);
    StatsResult get_statistic(date start, date end, int currentUserID);
    void IDgenerate();
    std::vector<FullRecord> get_by_hashtag(int h, int currentUserID);
    bool edit_record(date d, short n, short e, short s, const char* msg, int h, int currentUserID);
    bool delete_record(date d, int currentUserID);
    void input_with_date(short n, short e, short s, const char* text_note, int h, int currentUserID, date d);
};

#endif