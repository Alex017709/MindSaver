#include "date.h"
#include <ctime>

void date::PCdate() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    day = static_cast<short>(now->tm_mday);
    month = static_cast<short>(now->tm_mon + 1); 
    year = static_cast<short>(now->tm_year + 1900);
}

bool date::isValid() const {
    if (day <= 0 || month <= 0 || year <= 0 || year > 9999) return false;
    if (month > 12) return false;
    short daysInMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) daysInMonth[2] = 29;
    if (day > daysInMonth[month]) return false;
    return true;
}

bool date::operator<(const date& other) const {
    if (year != other.year) return year < other.year;
    if (month != other.month) return month < other.month;
    return day < other.day;
}

bool date::operator==(const date& other) const {
    return (day == other.day && month == other.month && year == other.year);
}

bool date::operator<=(const date& other) const { return (*this < other) || (*this == other); }
bool date::operator>=(const date& other) const { return !(*this < other); }