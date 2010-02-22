#include <iostream>
#include <boost/pool/pool_alloc.hpp>

using namespace std;

#define NUMITR 10000
#define FRAG_S 4
#define FRAG_E 1

class b1
{
    public:
        b1() : a_(0), b_(0), c_(0)
        {
        }
        
        virtual ~b1()
        {
        }

        void * operator new(size_t s)
        {
            std::cout << "b1 alloc, size = " << s << std::endl;
            return malloc(s);
        }
    
        void operator delete(void * p)
        {
            std::cout << "b1 free" << std::endl;
            b1 * b1p = static_cast<b1 *>(p);
            if(b1p)
            {
                free(b1p);
            }
        }

        int a_;
        int b_;
        int c_;
};

class b2 : public b1
{
    public:
        b2() : d_(0), e_(0), f_(0)
        {
        }

        virtual ~b2()
        {
        }

        /*void * operator new(size_t s)
        {
            std::cout << "b2 alloc, size = " << s << std::endl;
            return malloc(s);
        }
    
        void operator delete(void * p)
        {
            std::cout << "b2 free" << std::endl;
            b1 * b1p = static_cast<b1 *>(p);
            if(b1p)
            {
                free(b1p);
            }
        }*/

        int d_;
        int e_;
        int f_;
};
double get_time()
{
    struct timespec timeval;

    clock_gettime( CLOCK_REALTIME, &timeval );
    return ((double) (timeval.tv_sec) + 1.0e-9 * (double) (timeval.tv_nsec) );
}

template<size_t S>
class A
{
    public:
        char a_[S];
};

template<size_t S>
void runTest(int n, A< S > * a_array[], boost::fast_pool_allocator< A<S> > & A_alloc)
{
    double a_sum = 0;
    double d_sum = 0;

    for(int j = 0 ; j < n ; j++)
    {
        double t1 = 0, t2 = 0, t3 = 0;

        t1 = get_time();
        for(int i = 0 ; i < NUMITR ; i++)
        {
           a_array[i] = A_alloc.allocate();
        }
        t2 = get_time();
        for(int i = 0 ; i < NUMITR ; i++)
        {
            A_alloc.deallocate(a_array[i]);
        }
        t3 = get_time();

        a_sum += (t2 - t1);
        d_sum += (t3 - t2);
    }

    std::cout << S << " " << NUMITR << " " << a_sum / n << " " << d_sum / n << std::endl;
}

template<size_t S>
void runTest_frag(int n, A< S > * a_array[], boost::fast_pool_allocator< A<S> > & A_alloc)
{
    double a_sum = 0;
    double d_sum = 0;

    for(int j = 0 ; j < n ; j++)
    {
        double t1 = 0, t2 = 0, t3 = 0, t4 = 0 , t5 = 0, frag = 0;

        t1 = get_time();
        for(int i = 0 ; i < NUMITR ; i++)
        {
            a_array[i] = A_alloc.allocate();
   
            if(i % FRAG_S == FRAG_E)
            {
                t4 = get_time();
                A_alloc.deallocate(a_array[i]);
                t5 = get_time();
                frag += (t5 - t4);
            }
        }
        t2 = get_time();
        for(int i = 0 ; i < NUMITR ; i++)
        {
            if(i % FRAG_S != FRAG_E)
            {
                A_alloc.deallocate(a_array[i]);
            }
        }
        t3 = get_time();

        a_sum += (t2 - t1 - frag);
        d_sum += (t3 - t2 + frag);
    }

    std::cout << S << " " << NUMITR << " " << a_sum / n << " " << d_sum / n << std::endl;
}

template<size_t S>
void runTest_norm_frag(int n, A< S > * a_array[])
{
    double a_sum = 0;
    double d_sum = 0;

    for(int j = 0 ; j < n ; j++)
    {
        double t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, frag = 0;

        t1 = get_time();
        for(int i = 0 ; i < NUMITR ; i++)
        {
            a_array[i] = new A<S>();

            if(i % FRAG_S == FRAG_E)
            {
                t4 = get_time();
                delete a_array[i];
                t5 = get_time();
                frag += (t5 - t4);
            }
        }
        t2 = get_time();
        for(int i = 0 ; i < NUMITR ; i++)
        {
            if(i % FRAG_S != FRAG_E)
            {
                delete a_array[i];
            }
        }
        t3 = get_time();

        a_sum += (t2 - t1 - frag);
        d_sum += (t3 - t2 + frag);
    }

    std::cout << S << " " << NUMITR << " " << a_sum / n << " " << d_sum / n << std::endl;
}

template<size_t S>
void runTest_norm(int n, A< S > * a_array[])
{
    double a_sum = 0;
    double d_sum = 0;

    for(int j = 0 ; j < n ; j++)
    {
        double t1 = 0, t2 = 0, t3 = 0;

        t1 = get_time();
        for(int i = 0 ; i < NUMITR ; i++)
        {
           a_array[i] = new A<S>();
        }
        t2 = get_time();
        for(int i = 0 ; i < NUMITR ; i++)
        {
            delete a_array[i];
        }
        t3 = get_time();

        a_sum += (t2 - t1);
        d_sum += (t3 - t2);
    }

    std::cout << S << " " << NUMITR << " " << a_sum / n << " " << d_sum / n << std::endl;
}

int main()
{

    boost::fast_pool_allocator< A<1> > A_1_alloc;
    boost::fast_pool_allocator< A<2> > A_2_alloc;
    boost::fast_pool_allocator< A<4> > A_4_alloc;
    boost::fast_pool_allocator< A<8> > A_8_alloc;
    boost::fast_pool_allocator< A<16> > A_16_alloc;
    boost::fast_pool_allocator< A<32> > A_32_alloc;
    boost::fast_pool_allocator< A<64> > A_64_alloc;
    boost::fast_pool_allocator< A<128> > A_128_alloc;
    boost::fast_pool_allocator< A<256> > A_256_alloc;
    boost::fast_pool_allocator< A<312> > A_312_alloc;
    boost::fast_pool_allocator< A<352> > A_352_alloc;
    boost::fast_pool_allocator< A<512> > A_512_alloc;
    boost::fast_pool_allocator< A<1024> > A_1024_alloc;
    boost::fast_pool_allocator< A<2048> > A_2048_alloc;
    boost::fast_pool_allocator< A<4096> > A_4096_alloc;
    boost::fast_pool_allocator< A<8192> > A_8192_alloc;

    A<1> * a_1_array[NUMITR];
    A<2> * a_2_array[NUMITR];
    A<4> * a_4_array[NUMITR];
    A<8> * a_8_array[NUMITR];
    A<16> * a_16_array[NUMITR];
    A<32> * a_32_array[NUMITR];
    A<64> * a_64_array[NUMITR];
    A<128> * a_128_array[NUMITR];
    A<256> * a_256_array[NUMITR];
    A<312> * a_312_array[NUMITR];
    A<352> * a_352_array[NUMITR];
    A<512> * a_512_array[NUMITR];
    A<1024> * a_1024_array[NUMITR];
    A<2048> * a_2048_array[NUMITR];
    A<4096> * a_4096_array[NUMITR];
    A<8192> * a_8192_array[NUMITR];

    std::cout << "pool test" << std::endl;
    runTest(200, a_1_array, A_1_alloc); 
    runTest(200, a_2_array, A_2_alloc); 
    runTest(200, a_4_array, A_4_alloc); 
    runTest(200, a_8_array, A_8_alloc); 
    runTest(200, a_16_array, A_16_alloc); 
    runTest(200, a_32_array, A_32_alloc); 
    runTest(200, a_64_array, A_64_alloc); 
    runTest(200, a_128_array, A_128_alloc); 
    runTest(200, a_256_array, A_256_alloc); 
    runTest(200, a_312_array, A_312_alloc); 
    runTest(200, a_352_array, A_352_alloc); 
    runTest(200, a_512_array, A_512_alloc); 
    runTest(200, a_1024_array, A_1024_alloc); 
    runTest(200, a_2048_array, A_2048_alloc); 
    runTest(200, a_4096_array, A_4096_alloc); 
    runTest(200, a_8192_array, A_8192_alloc); 

    std::cout << "frag pool test" << std::endl;
    runTest_frag(200, a_1_array, A_1_alloc); 
    runTest_frag(200, a_2_array, A_2_alloc); 
    runTest_frag(200, a_4_array, A_4_alloc); 
    runTest_frag(200, a_8_array, A_8_alloc); 
    runTest_frag(200, a_16_array, A_16_alloc); 
    runTest_frag(200, a_32_array, A_32_alloc); 
    runTest_frag(200, a_64_array, A_64_alloc); 
    runTest_frag(200, a_128_array, A_128_alloc); 
    runTest_frag(200, a_256_array, A_256_alloc); 
    runTest_frag(200, a_312_array, A_312_alloc); 
    runTest_frag(200, a_352_array, A_352_alloc); 
    runTest_frag(200, a_512_array, A_512_alloc); 
    runTest_frag(200, a_1024_array, A_1024_alloc); 
    runTest_frag(200, a_2048_array, A_2048_alloc); 
    runTest_frag(200, a_4096_array, A_4096_alloc); 
    runTest_frag(200, a_8192_array, A_8192_alloc); 

    std::cout << "norm test" << std::endl;
    runTest_norm(200, a_1_array); 
    runTest_norm(200, a_2_array); 
    runTest_norm(200, a_4_array); 
    runTest_norm(200, a_8_array); 
    runTest_norm(200, a_16_array); 
    runTest_norm(200, a_32_array); 
    runTest_norm(200, a_64_array); 
    runTest_norm(200, a_128_array); 
    runTest_norm(200, a_256_array); 
    runTest_norm(200, a_312_array); 
    runTest_norm(200, a_352_array); 
    runTest_norm(200, a_512_array); 
    runTest_norm(200, a_1024_array); 
    runTest_norm(200, a_2048_array); 
    runTest_norm(200, a_4096_array); 
    runTest_norm(200, a_8192_array); 

    std::cout << "norm test frag" << std::endl;
    runTest_norm_frag(200, a_1_array); 
    runTest_norm_frag(200, a_2_array); 
    runTest_norm_frag(200, a_4_array); 
    runTest_norm_frag(200, a_8_array); 
    runTest_norm_frag(200, a_16_array); 
    runTest_norm_frag(200, a_32_array); 
    runTest_norm_frag(200, a_64_array); 
    runTest_norm_frag(200, a_128_array); 
    runTest_norm_frag(200, a_256_array); 
    runTest_norm_frag(200, a_312_array); 
    runTest_norm_frag(200, a_352_array); 
    runTest_norm_frag(200, a_512_array); 
    runTest_norm_frag(200, a_1024_array); 
    runTest_norm_frag(200, a_2048_array); 
    runTest_norm_frag(200, a_4096_array); 
    runTest_norm_frag(200, a_8192_array); 

    b1 * b11 = new b1();
    b2 * b21 = new b2();
    delete b11;
    delete b21;

    return 0;
}
