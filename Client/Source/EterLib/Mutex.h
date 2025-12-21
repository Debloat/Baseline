#pragma once

class Mutex
{
    public:
        Mutex();
        ~Mutex();

        void Lock();
        void Unlock();
        bool Trylock();

    private:
        CRITICAL_SECTION lock;
};
