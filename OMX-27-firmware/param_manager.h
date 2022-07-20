#pragma once
#include <Arduino.h>

class ParamManager
{
public:
    static const int kMaxPages = 10;
    
    // If true page will loop back to first after going past last page
    // If false, page will not increment
    bool rollPages = false;

    // If true the current page will be locked. Incrementing or decrementing params will loop on current page
    bool lockSelectedPage = false;

    // Max 10 pages, returns index of new page. returns -1 if can't add
    int8_t addPage(uint8_t numberOfParams = 4);
    void incrementParam();
    void decrementParam();

    void incrementPage();
    void decrementPage();

    int8_t getSelPage();
    void setSelPage(int8_t newPage);
    int8_t getSelParam();
    void setSelParam(int8_t newParam);
    uint8_t getNumPages();
    uint8_t getNumOfParamsForPage(uint8_t pageIndex);

private:
    int8_t selectedPage = 0;
    int8_t selectedParam = 0;
    uint8_t numberOfPages = 0;
    uint8_t pageConfigs[kMaxPages];
};