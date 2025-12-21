#pragma once

#ifndef __WIN32__
#include <semaphore.h>
#else

#endif

class CSemaphore
{
    private:
#ifndef __WIN32__
        sem_t* 	m_hSem;
#else
        HANDLE m_hSem;
#endif

    public:
        CSemaphore();
        ~CSemaphore();

        int	Initialize();
        void	Clear();
        void	Destroy();
        int	Wait();
        int	Release(int count = 1);
};
