#include "param_manager.h"

// Max 10 pages
int8_t ParamManager::addPage(uint8_t numberOfParams)
{
    if (numberOfPages >= kMaxPages)
        return -1;

    uint8_t newPageIndex = numberOfPages;
    pageConfigs[newPageIndex].numberOfParams = numberOfParams;
    pageConfigs[newPageIndex].enabled = true;
    numberOfPages = numberOfPages + 1;
    return newPageIndex;
}

void ParamManager::setPageEnabled(uint8_t pageIndex, bool enablePage)
{
    if(pageIndex < 0 || pageIndex > numberOfPages) return;

    pageConfigs[pageIndex].enabled = enablePage;

    if(!enablePage && selectedPage == pageIndex)
    {
        for(int8_t i = pageIndex - 1; i >= 0; i--)
        {
            if(pageConfigs[i].enabled)
            {
                selectedPage = i;
                return;
            }
        }

        for(int8_t i = pageIndex + 1; i < numberOfPages; i++)
        {
            if(pageConfigs[i].enabled)
            {
                selectedPage = i;
                return;
            }
        }
    }
}

void ParamManager::changeParam(int8_t direction)
{
    if (direction == 0)
        return;
    if (direction > 0)
        incrementParam();
    else
        decrementParam();
}

void ParamManager::incrementParam()
{
    if (numberOfPages == 0)
        return;

    selectedParam++;
    if (selectedParam >= pageConfigs[selectedPage].numberOfParams)
    {
        if (rollPages || selectedPage != numberOfPages -1) // Roll unless last page or roll pages
        {
            selectedParam = 0;
        }
        else
        {
            selectedParam = max(min(selectedParam - 1, pageConfigs[selectedPage].numberOfParams - 1), 0);
        }

        if (!lockSelectedPage)
        {
            incrementPage();
            selectedParam = constrain(selectedParam, 0, pageConfigs[selectedPage].numberOfParams - 1);
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
        if (rollPages || selectedPage != 0) // Roll unless first page or roll pages
        {
            selectedParam = max(pageConfigs[selectedPage].numberOfParams - 1, 0);
        }
        else
        {
            selectedParam = 0;
        }

        if (!lockSelectedPage)
        {
            decrementPage();

            selectedParam = constrain(selectedParam, 0, pageConfigs[selectedPage].numberOfParams - 1);
        }
    }
}

void ParamManager::incrementPage()
{
    if (numberOfPages == 0)
        return;

    // selectedPage = selectedPage + 1;

    bool foundEnabledPage = false;

    for (int8_t i = selectedPage + 1; i < numberOfPages; i++)
    {
        if (pageConfigs[i].enabled)
        {
            selectedPage = i;
            foundEnabledPage = true;
            break;
        }
    }

    if(!foundEnabledPage)
    {
        selectedPage = selectedPage + 1;
    }

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

    bool foundEnabledPage = false;

    for (int8_t i = selectedPage - 1; i >= 0; i--)
    {
        if (pageConfigs[i].enabled)
        {
            selectedPage = i;
            foundEnabledPage = true;
            break;
        }
    }

    if (!foundEnabledPage)
    {
        selectedPage = selectedPage - 1;
    }

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

void ParamManager::setSelPageAndParam(int8_t newPage, int8_t newParam)
{
    setSelPage(newPage);
    setSelParam(newParam);
}

int8_t ParamManager::getSelParam()
{
    return selectedParam;
}

void ParamManager::setSelParam(int8_t newParam)
{
    if (numberOfPages == 0)
        return;
    if (newParam < 0 || newParam >= pageConfigs[selectedPage].numberOfParams)
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

    return pageConfigs[pageIndex].numberOfParams;
}