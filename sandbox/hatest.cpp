#include <iostream>

#include <cstdlib>
#include <iofwdutil/HybridAllocator.hh>

class tsalloc
{
    public:
        tsalloc(int ts) : mem_(NULL)
        {
            mem_ = new char[ts];
        }

        ~tsalloc()
        {
            delete [] mem_;
        }

        char * mem_;
        iofwdutil::HybridAllocator<2048> ha_;
};

class nsalloc
{
    public:
        nsalloc(int ts) : mem_(NULL)
        {
            mem_ = new char[ts];
        }

        ~nsalloc()
        {
            delete [] mem_;
        }

        char * mem_;
};

double get_time()
{
    struct timespec timeval;
    clock_gettime( CLOCK_REALTIME, &timeval );
    return (double) (timeval.tv_sec + (timeval.tv_nsec * 1.0e-9));
}

void ts_test(const int num)
{
    double total = 0.0;
    double t1 = get_time();
    for(int i = 0 ; i < 1000000 ; i++)
    {
        tsalloc * h = new tsalloc(num);
        delete h;
    }
    total += (get_time() - t1);
    fprintf(stderr, "ha class size %i %.6f\n", num, total);
}

void ns_test(const int num)
{
    double total = 0.0;
    double t1 = get_time();
    for(int i = 0 ; i < 1000000 ; i++)
    {
        nsalloc * h = new nsalloc(num);
        delete h;
    }
    total += (get_time() - t1);
    fprintf(stderr, "std class size %i %.6f\n", num, total);
}

void ha_test1(int num)
{
    double total = 0.0;
    for(int i = 0 ; i < 10000 ; i++)
    {
        iofwdutil::HybridAllocator<2048> h;
        double t1 = get_time();
        for(int j = 0 ; j < num ; j++)
        {
            size_t * t = static_cast<size_t *>(h.malloc(sizeof(size_t)));
            h.free(t);
        }
        total += (get_time() - t1);
    }
    fprintf(stderr, "ha %i %.6f\n", num, total);
}

void ha_t_test1(int num)
{
    double total = 0.0;
    for(int i = 0 ; i < 10000 ; i++)
    {
        iofwdutil::HybridAllocator<2048> h;
        double t1 = get_time();
        for(int j = 0 ; j < num ; j++)
        {
            size_t * t = h.hamalloc<size_t>(1);
            h.hafree(t);
        }
        total += (get_time() - t1);
    }
    fprintf(stderr, "ha t %i %.6f\n", num, total);
}

void std_test1(int num)
{
    double total = 0.0;
    for(int i = 0 ; i < 10000 ; i++)
    {
        double t1 = get_time();
        for(int j = 0 ; j < num ; j++)
        {
            size_t * t = (size_t *)malloc(sizeof(size_t));
            free(t);
        }
        total += (get_time() - t1);
    }
    fprintf(stderr, "std %i %.6f\n", num, total);
}

int main()
{
    for(int i = 1 ; i <= 4096 ; i*=2)
    {
        std_test1(i);
        ha_test1(i);
        ha_t_test1(i);
    }

    for(int i = 1 ; i <= 4096 ; i*=2)
    {
        ns_test(i);
        ts_test(i);
    }


}
