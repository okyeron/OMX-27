#pragma once

class OmxModeInterface
{
public:
    OmxModeInterface(){}
    virtual ~OmxModeInterface(){}
    virtual void method1() = 0;    
    virtual void method2() = 0;
};