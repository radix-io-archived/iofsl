#include <list>
#include <vector>
#include <cstdio>
#include <time.h>

int get_time(struct timespec * timeval)
{
    clock_gettime( CLOCK_REALTIME, timeval );
    return 0;
}

double elapsed_time(struct timespec * t1, struct timespec * t2)
{
    return ((double) (t2->tv_sec - t1->tv_sec) +
        1.0e-9 * (double) (t2->tv_nsec - t1->tv_nsec) );
}

class typeA
{
    public:
        int A;
};

int main()
{
    /* lists */
    std::list<typeA> * lA;
    std::list<typeA> * lA_cp;

    /* vectors */
    std::vector<typeA> * vA;
    std::vector<typeA> * vA_cp;

    struct timespec t1, t2;
    int i = 0;
    int limit = 1024 * 128;

    for(int j = 1024 ; j <= limit ; j *= 2)
    {
    lA = new std::list<typeA>;
    get_time(&t1);
    for(i = 0 ; i < j ; i++)
    {
       lA->insert(lA->begin(), typeA()); 
    }
    get_time(&t2);
    fprintf(stderr, "lA insert begin: num = %i time = %f\n", j, elapsed_time(&t1, &t2));

    lA_cp = new std::list<typeA>;
    get_time(&t1);
    lA_cp->insert(lA_cp->begin(), lA->begin(), lA->end());
    get_time(&t2);
    fprintf(stderr, "lA range insert begin: num = %i time = %f\n", j, elapsed_time(&t1, &t2));
    lA_cp->clear();
    delete lA_cp;

    lA->clear();
    delete lA;
    }

    for(int j = 1024 ; j <= limit ; j *= 2)
    {
    lA = new std::list<typeA>;
    get_time(&t1);
    for(i = 0 ; i < j ; i++)
    {
       lA->insert(lA->end(), typeA()); 
    }
    get_time(&t2);
    fprintf(stderr, "lA insert end: num = %i time = %f\n", j, elapsed_time(&t1, &t2));

    lA_cp = new std::list<typeA>;
    get_time(&t1);
    lA_cp->insert(lA_cp->end(), lA->begin(), lA->end());
    get_time(&t2);
    fprintf(stderr, "lA range insert end: num = %i time = %f\n", j, elapsed_time(&t1, &t2));
    lA_cp->clear();
    delete lA_cp;

    lA->clear();
    delete lA;
    }

    for(int j = 1024 ; j <= limit ; j *= 2)
    {
    lA = new std::list<typeA>;
    get_time(&t1);
    for(i = 0 ; i < j ; i++)
    {
       lA->push_back(typeA()); 
    }
    get_time(&t2);
    lA->clear();
    delete lA;
    fprintf(stderr, "lA push_back: num = %i time = %f\n", j, elapsed_time(&t1, &t2));
    }

    for(int j = 1024 ; j <= limit ; j *= 2)
    {
    vA = new std::vector<typeA>;
    get_time(&t1);
    for(i = 0 ; i < j ; i++)
    {
       vA->insert(vA->begin(), typeA()); 
    }
    get_time(&t2);
    fprintf(stderr, "vA insert begin: num = %i time = %f\n", j, elapsed_time(&t1, &t2));

    vA_cp = new std::vector<typeA>;
    get_time(&t1);
    vA_cp->insert(vA_cp->begin(), vA->begin(), vA->end());
    get_time(&t2);
    fprintf(stderr, "vA range insert begin: num = %i time = %f\n", j, elapsed_time(&t1, &t2));
    vA_cp->clear();
    delete vA_cp;

    vA->clear();
    delete vA;
    }

    for(int j = 1024 ; j <= limit ; j *= 2)
    {
    vA = new std::vector<typeA>;
    get_time(&t1);
    for(i = 0 ; i < j ; i++)
    {
       vA->insert(vA->end(), typeA()); 
    }
    get_time(&t2);
    fprintf(stderr, "vA insert end: num = %i time = %f\n", j, elapsed_time(&t1, &t2));

    vA_cp = new std::vector<typeA>;
    get_time(&t1);
    vA_cp->insert(vA_cp->end(), vA->begin(), vA->end());
    get_time(&t2);
    fprintf(stderr, "vA range insert end: num = %i time = %f\n", j, elapsed_time(&t1, &t2));
    vA_cp->clear();
    delete vA_cp;

    vA->clear();
    delete vA;
    }

    for(int j = 1024 ; j <= limit ; j *= 2)
    {
    vA = new std::vector<typeA>;
    get_time(&t1);
    for(i = 0 ; i < j ; i++)
    {
       vA->push_back(typeA()); 
    }
    get_time(&t2);
    vA->clear();
    delete vA;
    fprintf(stderr, "vA push_back: num = %i time = %f\n", j, elapsed_time(&t1, &t2));
    }

    return 0;
}
