#pragma once

#ifdef __WIN32__
typedef CRITICAL_SECTION	lock_t;
#else
typedef pthread_mutex_t		lock_t;
#endif

class CLock
{
    public:
        CLock();
        ~CLock();

        void	Initialize();
        void	Destroy();
        int	Trylock();
        void	Lock();
        void	Unlock();

    private:
        lock_t	m_lock;
        bool	m_bLocked;
};
