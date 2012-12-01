#include <Arduino.h>

// Max value of unsigned long
#define ULONG_MAX 4294967295UL

struct AngeleanTime {
    unsigned long time;
    unsigned int year;

    unsigned int week;
    unsigned int weekDay;

    bool leapYear;

    unsigned long millisDiff;
};


class Angelean {
    public:
        Angelean(unsigned long time, unsigned int year):
            _startTime(time), _startYear(year), _secPerDay(86400) {
            _startMilli = millis();
        }
        void setYear(unsigned int year){
            _startYear = year;
        }
        void setTime(unsigned long time){
            _startTime = time;
            _startMilli = millis();
        }
        void setTimeStr(char *timeStr);
        AngeleanTime* getTime(AngeleanTime* retTime);
        char* getTimeStr(char* retStr);

    private:
        unsigned long _startTime;
        unsigned int _startYear;
        unsigned long _startMilli;

        const unsigned long _secPerDay;

};
