#ifndef DATE_H
#define DATE_H

struct date {
    short day;
    short month;
    short year;

    void PCdate();
    bool isValid() const;

    bool operator<(const date& other) const;
    bool operator==(const date& other) const;
    bool operator<=(const date& other) const; 
    bool operator>=(const date& other) const;
};

#endif