#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <bmi.h>
#include "zoidfs/client/bmi_comm.h"

/* measure the performance of the BMI interface for C clients */

/* test buffer sizes and the number of itrs per buffer size */
size_t buflen [] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8124,16384,32768,65536,131072,262144,524288,1048576,2097152,4194304,8388608,16777216};
int numitr [] = {2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,1000,1000,1000,1000,1000,100,100,10,10,10,10};
size_t ubuflen [] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8124};
int unumitr [] = {10000,10000,10000,10000,10000,10000,10000,10000,5000,5000,5000,2000,2000,2000};

static BMI_addr_t peer_addr;
static void * bmi_bar_recv_buf = NULL;
static void * bmi_bar_send_buf = NULL;

static pthread_t * threads = NULL;
static int num_threads = 0;
static pthread_barrier_t tbarr;
static double * etime = NULL;
static bmi_context_id * context = NULL;

struct threaddata
{
    int tid;
    int client;
    int unexpected;
};
typedef struct threaddata threaddata_t;
static threaddata_t * threaddata = NULL;

struct server_request
{
    bmi_size_t size;
};

/* timer helpers */
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

/* setup BMI */
static int bmi_test_init(char * addr, int server, char * method)
{
    int ret = 0;
    int i = 0;
    struct BMI_unexpected_info request_info;
    int outcount = 0;

    /* Initialize BMI */
    if(server)
    {
        ret = BMI_initialize(method, addr, BMI_INIT_SERVER);
        if (ret < 0) {
            fprintf(stderr, "BMI_initialize() server failed.\n");
            exit(1);
        }
    }
    else
    {
        ret = BMI_initialize(NULL, NULL, 0);
        if (ret < 0) {
            fprintf(stderr, "BMI_initialize() client failed.\n");
            exit(1);
        }
    }

    context = (bmi_context_id *) malloc(sizeof(bmi_context_id) * num_threads);
    for(i = 0 ; i < num_threads ; i++)
    {
        ret = BMI_open_context(&context[i]);
            if (ret < 0) {
            fprintf(stderr, "BMI_open_context() failed.\n");
            exit(1);
        }
    }

    /* Create a new BMI context */
    if(server)
    {
        /* wait for an initial request  */
        do
        {
            ret = BMI_testunexpected(1, &outcount, &request_info, 10);
        } while (ret == 0 && outcount == 0);

        if (request_info.error_code != 0)
        {
            fprintf(stderr, "Request recv failure (bad state).\n");
            return (-1);
        }

        if (request_info.size != sizeof(struct server_request))
        {
            fprintf(stderr, "Bad Request!\n");
            exit(-1);
        }

        peer_addr = request_info.addr;
    }
    else
    {
        /* Perform an address lookup on the ION */
        ret = BMI_addr_lookup(&peer_addr, addr);
        if (ret < 0) {
            fprintf(stderr, "BMI_addr_lookup() failed, addr = %s.\n", addr);
            exit(1);
        }

        for(i = 0 ; i < num_threads ; i++)
        {
            /* allocate a buffer for the initial request and ack */
            struct server_request * my_req = (struct server_request *) BMI_memalloc(peer_addr,
                            sizeof(struct server_request), BMI_SEND);
            my_req->size = 128;
            void * in_user_ptr = &peer_addr;
            void * out_user_ptr = NULL;
            bmi_op_id_t cops = 0;
            bmi_error_code_t error_code;
            bmi_size_t actual_size = 0;

            /* send the initial request on its way */
            ret = BMI_post_sendunexpected(&cops, peer_addr, my_req,
                  sizeof(struct server_request), BMI_PRE_ALLOC,
                  0, in_user_ptr, context[i], NULL);
            if (ret < 0)
            {
                errno = -ret;
                perror("BMI_post_send");
                return (-1);
            }
            if (ret == 0)
            {
            /* check for completion of request */
                do
                {
                    ret = BMI_test(cops, &outcount, &error_code, &actual_size,
                        &out_user_ptr, 10, context[i]);
                } while (ret == 0 && outcount == 0);
            }
        }
    }

    /* setup the bmi barrier buffers */
    bmi_bar_send_buf = BMI_memalloc(peer_addr, 1, BMI_SEND);
    bmi_bar_recv_buf = BMI_memalloc(peer_addr, 1, BMI_RECV);

    return 0;
}

/* linear barrier */
static int bmi_barrier(int client, bmi_msg_tag_t tag, int size)
{
    bmi_size_t actual_size = 0;

    /* if server, recv then send */
    if(!client)
    {
        int i = 0;
        for(i = 0 ; i < size ; i++)
        {
            bmi_comm_recv(peer_addr, bmi_bar_recv_buf, 1, (2*i) + 1, context[i], &actual_size);
            bmi_comm_send(peer_addr, bmi_bar_send_buf, 1, (2*i), context[i]);
        }
    }
    /* if client, send then recv */
    else
    {
        bmi_comm_send(peer_addr, bmi_bar_send_buf, 1, tag + 1, context[tag / 2]);
        bmi_comm_recv(peer_addr, bmi_bar_recv_buf, 1, tag, context[tag / 2], &actual_size);
    }

    return 0;
}

/* shutdown BMI */
static int bmi_test_finalize()
{
    int ret = 0;
    int i = 0;
    
    BMI_memfree(peer_addr, bmi_bar_send_buf, 128, BMI_SEND);
    BMI_memfree(peer_addr, bmi_bar_recv_buf, 128, BMI_RECV);

    for(i = 0 ; i < num_threads ; i++)
    {
        BMI_close_context(context[i]);
    }
    free(context);

    /* Finalize BMI */
    ret = BMI_finalize();
    if (ret < 0) {
        fprintf(stderr, "BMI_finalize() failed.\n");
        exit(1);
    }
    return 0;
}

int report_results(unsigned int id, size_t buflen, int itr)
{
    int i = 0;
    double max = etime[0];
    double min = etime[0];
    double avg = etime[0];
    double bw_max = 0.0;
    double bw_min = 0.0;
    double bw_avg = 0.0;
    double bw_aggr = 0.0;

    for(i = 1 ; i < num_threads ; i++)
    {
        avg += etime[i];
        if(min > etime[i])
        {
            min = etime[i]; 
        }
        if(max < etime[i])
        {
            max = etime[i];
        }
    }

    avg /= num_threads;
    bw_max = 2.0 * itr * buflen / min / (1024 * 1024);
    bw_min = 2.0 * itr * buflen / max / (1024 * 1024);
    bw_avg = 2.0 * itr * buflen / avg / (1024 * 1024);
    bw_aggr = 2.0 * num_threads * itr * buflen / max / (1024 * 1024);

    fprintf(stderr, "test %u: size = %lu, itr = %i, min t = %f, max t = %f, avg t = %f, min bw = %f, max bw = %f, avg bw = %f, aggr bw = %f\n", 
        id, buflen, itr, min, max, avg, bw_min, bw_max, bw_avg, bw_aggr); 

    return 0;
}

void * run_test(void * t)
{
    threaddata_t * d = (threaddata_t *)t;
    void * sendbuf = NULL;
    void * recvbuf = NULL;
    unsigned int i = 0;

    /* run through the bmi buflen tests */
    for(i = 0 ; (i < sizeof(buflen) / sizeof(size_t) && !d->unexpected) || (i < sizeof(ubuflen) / sizeof(size_t) && d->unexpected) ; i++)
    {
        bmi_size_t actual_size = 0;
        bmi_msg_tag_t tag = 0; /* make tag based on the thread id */
        struct timespec t1, t2;

        /* create the send and recv buffers */
        if(d->unexpected)
        {
            sendbuf = BMI_memalloc(peer_addr, ubuflen[i], BMI_SEND);
        }
        else
        {
            sendbuf = BMI_memalloc(peer_addr, buflen[i], BMI_SEND);
            recvbuf = BMI_memalloc(peer_addr, buflen[i], BMI_RECV);
        }

        if(!d->unexpected)
        {
            /* if this a client, send and then recv */
            if(d->client)
            {
                /* warmup */
                bmi_comm_send(peer_addr, sendbuf, buflen[i], 1, context[d->tid]);
                bmi_comm_recv(peer_addr, recvbuf, buflen[i], 0, context[d->tid], &actual_size);

                /* sync here */
                pthread_barrier_wait(&tbarr);

                int j = 0;
                get_time(&t1);
                for(j = 0 ; j < numitr[i] ; j++)
                {
                    bmi_comm_send(peer_addr, sendbuf, buflen[i], 1, context[d->tid]);
                    bmi_comm_recv(peer_addr, recvbuf, buflen[i], 0, context[d->tid], &actual_size);
                    if(buflen[i] != actual_size)
                    {
                        fprintf(stderr, "buflen != actual_size, buflen = %lu, actual_size = %lu\n", buflen[i], actual_size);
                    }
                }
                get_time(&t2);
                etime[d->tid] = elapsed_time(&t1, &t2);
                pthread_barrier_wait(&tbarr);

                /* only thread 0 reports the results */
                if(d->tid == 0)
                {
                    report_results(i, buflen[i], numitr[i]); 
                }
            }
            /* if this a server, recv then send */
            else
            {
                int k = 0;
                /* warmup */
                for(k = 0 ; k < num_threads ; k++)
                {
                    bmi_comm_recv(peer_addr, recvbuf, buflen[i], 1, context[d->tid], &actual_size);
                    bmi_comm_send(peer_addr, sendbuf, buflen[i], 0, context[d->tid]);
                }

                int j = 0;
                for(j = 0 ; j < numitr[i] ; j++)
                {
                    for(k = 0 ; k < num_threads ; k++)
                    {
                        bmi_comm_recv(peer_addr, recvbuf, buflen[i], 1, context[d->tid], &actual_size);
                        bmi_comm_send(peer_addr, sendbuf, buflen[i], 0, context[d->tid]);
                        if(buflen[i] != actual_size)
                        {
                            fprintf(stderr, "buflen != actual_size, buflen = %lu, actual_size = %lu\n", buflen[i], actual_size);
                        }
                    }
                }
            }
        }
        else
        {
            /* if this a client, send and then recv */
            if(d->client)
            {
                int j = 0;
                bmi_size_t bl = ubuflen[i];

                pthread_barrier_wait(&tbarr);

                get_time(&t1);
                for(j = 0 ; j < unumitr[i] ; j++)
                {
                    bmi_comm_sendu(peer_addr, sendbuf, bl, tag, context[d->tid]);
                }
                get_time(&t2);
                etime[d->tid] = elapsed_time(&t1, &t2);
                pthread_barrier_wait(&tbarr);

                if(d->tid == 0)
                {
                    report_results(i, ubuflen[i], unumitr[i]);
                }
            }
            /* if this a server, recv then send */
            else
            {
                int j = 0;
                bmi_size_t bl = ubuflen[i];
                for(j = 0 ; j < unumitr[i] ; j++)
                {
                    bmi_comm_recvu(&peer_addr, &recvbuf, &bl, &tag);
                    BMI_memfree(peer_addr, recvbuf, ubuflen[i], BMI_RECV);
                }
            }
        }

        if(d->unexpected)
        {
            BMI_memfree(peer_addr, sendbuf, ubuflen[i], BMI_SEND);
        }
        else
        {
            BMI_memfree(peer_addr, sendbuf, buflen[i], BMI_SEND);
            BMI_memfree(peer_addr, recvbuf, buflen[i], BMI_RECV);
        }
    }

    pthread_exit(NULL);
    return NULL;
}

/* main */
int main(int argc, char * argv[])
{
    char * addr = NULL;
    char * cors = NULL;
    char * host = NULL;
    int client = 0;
    char method[16];
    char network[16];
    int len = 0;
    pthread_attr_t attr;
    void * status = NULL;
    int unexpected = 0;
    int i = 0;

    if(argc < 5)
    {
        fprintf(stderr, "incorrect args!\n");
        fprintf(stderr, "bmi-pp-perf tcp://127.0.0.1:12347 {client,server} {expected,unexpected} numthreads\n");
        return - 1;
    }
    addr = argv[1];
    cors = argv[2];
    if(strcmp(cors, "client") == 0)
    {
        client = 1;
    }

    if(strcmp(argv[3], "unexpected") == 0)
    {
        unexpected = 1;
    }
    num_threads = atoi(argv[4]);

    /* compute the bmi method to use based on the address */
    host = strchr(addr, ':');
    if (!host)
        return 1;
    len = host - addr;
    strncpy(network, addr, len);
    network[len] = '\0';
    sprintf(method, "bmi_%s", network);

    /* start up bmi */
    if(strcmp(cors, "server") == 0)
    {
        bmi_test_init(addr, 1, method);
    }
    else
    {
        bmi_test_init(addr, 0, method);
    }

    /* setup the the threads */
    if(client)
    {
        threads = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
        threaddata = (threaddata_t*) malloc(sizeof(threaddata_t) * num_threads);
        etime = (double *) malloc(sizeof(double) * num_threads);
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        pthread_barrier_init(&tbarr, NULL, num_threads);

        for(i = 0 ; i < num_threads ; i++)
        {
            threaddata[i].tid = i;
            threaddata[i].client = client;
            threaddata[i].unexpected = unexpected;
            pthread_create(&threads[i], &attr, run_test, (void *)&threaddata[i]);
        }
    }
    else
    {
        threads = (pthread_t *) malloc(sizeof(pthread_t));
        threaddata = (threaddata_t*) malloc(sizeof(threaddata_t));
        etime = (double *) malloc(sizeof(double));
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        pthread_barrier_init(&tbarr, NULL, 1);

        threaddata[0].tid = 0;
        threaddata[0].client = 0;
        threaddata[0].unexpected = unexpected;
        pthread_create(&threads[0], &attr, run_test, (void *)&threaddata[0]);
    }

    /* cleanup threads */
    pthread_attr_destroy(&attr);
    if(client)
    {
        for(i = 0 ; i < num_threads ; i++)
        {
            pthread_join(threads[i], &status);
        }
    }
    else
    {
            pthread_join(threads[0], &status);
    }

    pthread_barrier_destroy(&tbarr);
    free(etime);
    free(threaddata);
    free(threads);

    /* shutdown bmi */
    bmi_test_finalize();

    return 0;
}
