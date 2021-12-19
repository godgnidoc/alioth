#ifndef __rutex__
#define __rutex__

/**
 * @module rutex
 * @version 1.0.0; Feb. 03, 2021 by GodGnidoc
 * @encoding UTF8
 * @brief
 *  原子锁模块基于CAS原子操作实现递归同步互斥量，用于在高并发场景提高临界资源的同步操作效率 */

#include <atomic>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

class rutex {
    private:
        std::atomic_uint64_t owner;
        size_t layer;
        friend class rlock;

    public:
        rutex():owner(0), layer(0){}
        rutex( const rutex& ) = delete;
        rutex( rutex&& ) = delete;
        rutex& operator = (const rutex&) = delete;
        rutex& operator = (rutex&&) = delete;
};

class rlock {
    private:
        using id = uint64_t;
    public:
        struct defer_lock_t {};
    private:
        rutex& x;
        id me;
    public:
        inline rlock( rutex& l ): x(l) { lock(); }
        inline rlock( rutex& l, unsigned int times ): x(l) { try_to_lock(times); }
        inline rlock( rutex& l, defer_lock_t): x(l) {}
        inline rlock( const rlock& r ): x(r.x) { lock(); }
        inline rlock( rlock&& r ): x(r.x) { lock(); }
        ~rlock() { unlock(); }

        inline void lock() {
            if(me != nobody())
                return;

            me = myself();

            if(x.owner != me) {
                id exp = nobody();
                while( !x.owner.compare_exchange_weak(exp, me) ) {
                    exp = nobody();
                }
            }

            x.layer += 1;
        }

        inline bool try_to_lock( unsigned int times ) {
            if( me != nobody() )
                return true;
            
            me = myself();

            if( x.owner == me ) {
                x.layer += 1;
                return true;
            }

            while( times-- > 0 ) {
                id exp = nobody();
                if( x.owner.compare_exchange_weak(exp, me) ) {
                    x.layer += 1;
                    return true;
                }
            }

            me = nobody();
            return false;
        }

        inline void unlock() {
            if( me != nobody() && --x.layer == 0 ) {
                x.owner.compare_exchange_strong(me, nobody());
            }
            me = nobody();
        }
    private:
        inline id myself() { 
            #ifdef _WIN32
            return GetCurrentThreadId();
            #else
            return pthread_self();
            #endif
        }
        inline id nobody() { 
            return 0; 
        }
};

#endif