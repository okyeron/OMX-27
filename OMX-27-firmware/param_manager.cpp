#include "param_manager.h"

// Max 10 pages
int8_t ParamManager::addPage(uint8_t numberOfParams)
{
    if (numberOfPages >= kMaxPages)
        return -1;

    uint8_t newPageIndex = numberOfPages;
    pageConfigs[newPageIndex] = numberOfParams;
    numberOfPages = numberOfPages + 1;
    return newPageIndex;
}
void ParamManager::incrementParam()
{
    if (numberOfPages == 0)
        return;

    selectedParam++;
    if (selectedParam >= pageConfigs[selectedPage])
    {
        selectedParam = 0;
        if (!lockSelectedPage)
        {
            incrementPage();
        }
    }
}
void ParamManager::decrementParam()
{
    if (numberOfPages == 0)
        return;

    selectedParam--;
    if (selectedParam < 0)
    {
        selectedParam = max(pageConfigs[selectedPage] - 1, 0);
        if (!lockSelectedPage)
        {
            decrementPage();
        }
    }
}

void ParamManager::incrementPage()
{
    if (numberOfPages == 0)
        return;

    selectedPage = selectedPage + 1;

    if (selectedPage >= numberOfPages)
    {
        if (rollPages)
        {
            selectedPage = 0;
        }
        else
        {
            selectedPage = max(selectedPage - 1, 0);
        }
    }
}
void ParamManager::decrementPage()
{
    if (numberOfPages == 0)
        return;

    selectedPage = selectedPage - 1;

    if (selectedPage < 0)
    {
        if (rollPages)
        {
            selectedPage = numberOfPages - 1;
        }
        else
        {
            selectedPage = min(selectedPage + 1, numberOfPages - 1);
        }
    }
}

int8_t ParamManager::getSelPage()
{
    return selectedPage;
}

void ParamManager::setSelPage(int8_t newPage)
{
    if (newPage < 0 || newPage >= numberOfPages)
        return;
    selectedPage = newPage;
}

int8_t ParamManager::getSelParam()
{
    return selectedParam;
}

void ParamManager::setSelParam(int8_t newParam)
{
    if (numberOfPages == 0)
        return;
    if (newParam < 0 || newParam >= pageConfigs[selectedPage])
        return;

    selectedParam = newParam;
}

uint8_t ParamManager::getNumPages()
{
    return numberOfPages;
}

uint8_t ParamManager::getNumOfParamsForPage(uint8_t pageIndex)
{
    if (numberOfPages == 0 || pageIndex < 0 || pageIndex >= numberOfPages)
        return 0;

    return pageConfigs[pageIndex];
}