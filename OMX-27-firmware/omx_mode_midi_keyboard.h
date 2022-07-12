#pragma once

#include "omx_mode_interface.h"

class OmxModeMidiKeyboard : public OmxModeInterface
{
public:
    OmxModeMidiKeyboard(){}
    ~OmxModeMidiKeyboard(){}
    void method1();
    void method2();

private:
    int myMember;

};

// Provide implementation for the first method
void Concrete::method1()
{
    // Your implementation
}

// Provide implementation for the second method
void Concrete::method2()
{
    // Your implementation
}

int main(void)
{
    Interface *f = new Concrete();

    f->method1();
    f->method2();

    delete f;

    return 0;
}