#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#include <bmi.h>
#include "zoidfs/client/bmi_comm.h"

/* measure the performance of the BMI interface for C clients */

/* test buffer sizes and the number of itrs per buffer size */
size_t buflen [] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8124,16384,32768,65536,131072,262144,524288,1048576,2097152,4194304,8388608,16777216};
int numitr [] = {50000,50000,50000,50000,50000,50000,50000,50000,50000,50000,50000,50000,50000,50000,50000,50000,5000,1000,1000,100,100,10,10,10,10};
size_t ubuflen [] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8124,16384};
int unumitr [] = {10000,10000,10000,10000,10000,10000,10000,10000,5000,5000,5000,2000,2000,2000,2000};

static BMI_addr_t peer_addr;
static bmi_context_id context;
static void * bmi_bar_recv_buf = NULL;
static void * bmi_bar_send_buf = NULL;

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

    ret = BMI_open_context(&context);
        if (ret < 0) {
        fprintf(stderr, "BMI_open_context() failed.\n");
        exit(1);
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
                  0, in_user_ptr, context, NULL);
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
                    &out_user_ptr, 10, context);
            } while (ret == 0 && outcount == 0);
        }
    }

    /* setup the bmi barrier buffers */
    bmi_bar_send_buf = BMI_memalloc(peer_addr, 128, BMI_SEND);
    bmi_bar_recv_buf = BMI_memalloc(peer_addr, 128, BMI_RECV);

    return 0;
}

static int bmi_barrier(int mode)
{
    bmi_size_t actual_size = 0;
    bmi_msg_tag_t tag = 456;

    /* if server, recv then send */
    if(mode)
    {
        bmi_comm_recv(peer_addr, bmi_bar_recv_buf, 128, tag, context, &actual_size);
        bmi_comm_send(peer_addr, bmi_bar_send_buf, 128, tag, context);
    }
    /* if server, send then recv */
    else
    {
        bmi_comm_send(peer_addr, bmi_bar_send_buf, 128, tag, context);
        bmi_comm_recv(peer_addr, bmi_bar_recv_buf, 128, tag, context, &actual_size);
    }

    return 0;
}

/* shutdown BMI */
static int bmi_test_finalize()
{
    int ret = 0;

    BMI_memfree(peer_addr, bmi_bar_send_buf, 128, BMI_SEND);
    BMI_memfree(peer_addr, bmi_bar_recv_buf, 128, BMI_RECV);
    
    BMI_close_context(context);

    /* Finalize BMI */
    ret = BMI_finalize();
    if (ret < 0) {
        fprintf(stderr, "BMI_finalize() failed.\n");
        exit(1);
    }
    return 0;
}

/* main */
int main(int argc, char * argv[])
{
    unsigned int i = 0;
    char * addr = NULL;
    char * cors = NULL;
    char * host = NULL;
    int mode = 0;
    char method[16];
    char network[16];
    int len = 0;
    int unexpected = 0;

    if(argc < 4)
    {
        fprintf(stderr, "incorrect args!\n");
        fprintf(stderr, "bmi-pp-perf tcp://127.0.0.1:12347 {client,server} {expected,unexpected}\n");
        return - 1;
    }
    addr = argv[1];
    cors = argv[2];

    if(strcmp(argv[3], "unexpected") == 0)
    {
        unexpected = 1;
    }

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
        mode = 1;
    }
    else
    {
        bmi_test_init(addr, 0, method);
        mode = 0;
    }

    /* run through the bmi buflen tests */
    for(i = 0 ; (i < sizeof(buflen) / sizeof(size_t) && !unexpected) || (i < sizeof(ubuflen) / sizeof(size_t) && unexpected) ; i++)
    {
        void * sendbuf = NULL;
        void * recvbuf = NULL;
        bmi_size_t actual_size = 0;
        bmi_msg_tag_t tag = 123;
        struct timespec t1, t2;

        /* create the send and recv buffers */
        if(unexpected)
        {
            sendbuf = BMI_memalloc(peer_addr, ubuflen[i], BMI_SEND);
            //recvbuf = BMI_memalloc(peer_addr, ubuflen[i], BMI_RECV);
        }
        else
        {
            sendbuf = BMI_memalloc(peer_addr, buflen[i], BMI_SEND);
            recvbuf = BMI_memalloc(peer_addr, buflen[i], BMI_RECV);
        }
    
        /* barrier and sync here */ 
        bmi_barrier(mode);

        if(!unexpected)
        {
            /* if this a client, send and then recv */
            if(strcmp(cors, "client") == 0)
            {
                int j = 0;
                get_time(&t1);
                for(j = 0 ; j < numitr[i] ; j++)
                {
                    bmi_comm_send(peer_addr, sendbuf, buflen[i], tag, context);
                    bmi_comm_recv(peer_addr, recvbuf, buflen[i], tag, context, &actual_size);
                }
                get_time(&t2);
                fprintf(stderr, "test %u: size = %lu, itr = %i, time = %f, bw = %f\n", i, buflen[i], numitr[i], elapsed_time(&t1, &t2), (1.0 * j * buflen[i] / elapsed_time(&t1, &t2)) / (1024.0 * 1024.0));
            }
            /* if this a server, recv then send */
            else if(strcmp(cors, "server") == 0)
            {
                int j = 0;
                for(j = 0 ; j < numitr[i] ; j++)
                {
                    bmi_comm_recv(peer_addr, recvbuf, buflen[i], tag, context, &actual_size);
                    bmi_comm_send(peer_addr, sendbuf, buflen[i], tag, context);
                }
            }
        }
        else
        {
            /* if this a client, send and then recv */
            if(strcmp(cors, "client") == 0)
            {
                int j = 0;
                bmi_size_t bl = ubuflen[i];
                get_time(&t1);
                for(j = 0 ; j < unumitr[i] ; j++)
                {
                    bmi_comm_sendu(peer_addr, sendbuf, bl, tag, context);
                    bmi_comm_recvu(&peer_addr, &recvbuf, &bl, &tag);
                    BMI_memfree(peer_addr, recvbuf, ubuflen[i], BMI_RECV);
                }
                get_time(&t2);
                fprintf(stderr, "test %u: size = %lu, itr = %i, time = %f, bw = %f\n", i, buflen[i], unumitr[i], elapsed_time(&t1, &t2), (1.0 * j * ubuflen[i] / elapsed_time(&t1, &t2)) / (1024.0 * 1024.0));
            }
            /* if this a server, recv then send */
            else if(strcmp(cors, "server") == 0)
            {
                int j = 0;
                bmi_size_t bl = ubuflen[i];
                for(j = 0 ; j < unumitr[i] ; j++)
                {
                    bmi_comm_recvu(&peer_addr, &recvbuf, &bl, &tag);
                    BMI_memfree(peer_addr, recvbuf, ubuflen[i], BMI_RECV);
                    bmi_comm_sendu(peer_addr, sendbuf, bl, tag, context);
                }
            }
        }

        if(unexpected)
        {
            BMI_memfree(peer_addr, sendbuf, ubuflen[i], BMI_SEND);
            //BMI_memfree(peer_addr, recvbuf, ubuflen[i], BMI_RECV);
        }
        else
        {
            BMI_memfree(peer_addr, sendbuf, buflen[i], BMI_SEND);
            //BMI_memfree(peer_addr, recvbuf, buflen[i], BMI_RECV);
        }
    }

    /* shutdown bmi */
    bmi_test_finalize();

    return 0;    
}
