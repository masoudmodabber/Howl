#ifndef PASSEDPAWNSETUP_H
#define PASSEDPAWNSETUP_H

class PassedPawnSetup
{
public:
    static long long WhitePassedMask[64];
    static long long BlackPassedMask[64];

    static void Initialize();
    static void Cleanup();

private:
    static bool initialized;

    static void setPassedPawnMask();
};

#endif