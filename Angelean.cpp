#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <Arduino.h>

#include "Angelean.h"

// Extract the int from the string starting on 'start'
// and containing 'n' digits.
int extractInt(char* start, size_t numChar){
    // C99 allows var length arrays. yay.
    char buffer[numChar+1];
    int result;
    strncpy(&(buffer[0]), start, numChar);
    buffer[numChar]='\0';
    result = atoi(&(buffer[0]));
    return result;
}


bool isLeapYear(unsigned int year){
    if (year % 400 == 0){
        return true;
    } else if (year % 100 == 0){
        return false;
    } else if(year % 4 == 0){
        return true;
    } else{
        return false;
    }
}

unsigned long mod(unsigned long a, unsigned long b, int* count){
    *count = 0;
    while(a >= b){
        a -= b;
        (*count)++;
    }
    return a;
}

void Angelean::setTimeStr(char *timeStr){
    unsigned int days;
    unsigned int mdays;
    unsigned int year;

    days = extractInt(&(timeStr[0]), 3);
    mdays = extractInt(&(timeStr[4]), 3);
    year = extractInt(&(timeStr[9]), 4);

    setTime(days*1000UL+mdays);
    setYear(year);
}

AngeleanTime* Angelean::getTime(AngeleanTime* retTime){
    unsigned long time;
    int yearOffset;
    unsigned int year;
    unsigned int week;
    unsigned int weekDay;
    unsigned long curMilli;

    unsigned long mdaysPerYear;

    mdaysPerYear = isLeapYear(_startYear) ? 366000 : 365000;

    curMilli = millis();
    if (curMilli < _startMilli){
        // Handle milli wrap around
        // TODO: This needs to handle time being kept in millidays.
        time = curMilli/_secPerDay + (ULONG_MAX - _startMilli)/_secPerDay + _startTime;
        _startMilli = curMilli;
        _startTime = time;
        time = mod(time, mdaysPerYear, &yearOffset);
        year = _startYear + yearOffset;
        _startYear = year;
    } else {
        time = (curMilli-_startMilli)/_secPerDay + _startTime;
        time = mod(time, mdaysPerYear, &yearOffset);
        if(yearOffset > 0){
            _startMilli = curMilli;
            _startTime = time;
            _startYear = _startYear + yearOffset;
        }
        year = _startYear;
    }

    week = time/10000;
    weekDay = time/1000 - week*10;

    retTime->time = time;
    retTime->year = year;
    retTime->week = week;
    retTime->weekDay = weekDay;
    retTime->leapYear = isLeapYear(year);
    retTime->millisDiff = curMilli-_startMilli;
    return retTime;
}

/**
 * retStr must be sizeof(char)*16
 **/
char* Angelean::getTimeStr(char* retStr){
    AngeleanTime curTime;
    unsigned int days;
    unsigned int mdays;
    int charOffset;

    getTime(&curTime);

    days = curTime.time/1000;
    mdays = curTime.time - (days * 1000);

    // First convert the days to a string.
    // Pad so that it's 3 digits long.
    if(days<10){
        charOffset = 2;
        retStr[0]='0';
        retStr[1]='0';
    }else if(days<100){
        charOffset = 1;
        retStr[0]='0';
    }else{
        charOffset = 0;
    }
    utoa(days, retStr+charOffset, 10);
    // Insert decimal point.
    retStr[3] = '.';
    // Convert millidays to string, also padding to 3 digits.
    if(mdays<10){
        charOffset = 2+4;
        retStr[4]='0';
        retStr[5]='0';
    }else if(mdays<100){
        charOffset = 1+4;
        retStr[4]='0';
    }else{
        charOffset = 0+4;
    }
    utoa(mdays, retStr+charOffset, 10);

    // Add year.
    retStr[7]=',';
    retStr[8]= ' ';
    utoa(curTime.year, retStr+9, 10);
    // Just in case.
    retStr[15] = '\0';

    return retStr;
}
