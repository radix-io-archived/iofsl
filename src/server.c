#include "bmi.h"
#include "bmi_comm.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define BUFSIZE 1024
#define MAX_IDLE_TIME 10


inline int handle_error (int ret, const char * str)
{
   if (ret >= 0) return ret;
   errno = -ret; 
   perror (str); 
   exit (1); 
}

const char *  get_network(const char *id)
{
   int i; 
   const char * names[] = 
     { "tcp://", "bmi_tcp", "gm://", "bmi_gm", "mx://", "bmi_mx" };

   for (i=0; i<(sizeof(names)/sizeof(names[0])); i+=2)
   {
      if (strncmp (names[i], id, strlen(names[i]))==0)
      {
         return names[i+1]; 
      }
   }
   fprintf (stderr, "Unknown network string: %s", id); 
   return 0; 
}



int init_bmi(const char *hostid) 
{
    /* get the network from the hostid, e.g "bmi_tcp" */
    const char * network = get_network(hostid); 
    if (!network)
       exit (1); 
    printf ("Using network type: %s, addr: %s\n", network, hostid); 

    /* Initialize BMI */
    handle_error(BMI_initialize(network, hostid , BMI_INIT_SERVER), 
          "BMI_initialize");
    return 0;
}


void finalize_bmi() 
{
    handle_error(BMI_finalize(), "BMI_finalize");
}


int iod_bmi_handshake(BMI_addr_t *peer_addr, bmi_msg_tag_t tag,
                          bmi_context_id context) {
    void *sendbuf = NULL;
    int max_bytes = 4;
    int ret, outcount = 0;
    struct BMI_unexpected_info request_info;

    /* Wait for an initial request from client */
    do {
        ret = BMI_testunexpected(1, &outcount, &request_info, MAX_IDLE_TIME);
    } while (ret == 0 && outcount == 0);

    if (ret < 0) {
        fprintf(stderr,
                "iod_bmi_handshake: Request recv failure (bad state).\n");
        errno = -ret;
        perror("iod_bmi_handshake: BMI_testunexpected");
        exit(1);
    }

    if (request_info.error_code != 0) {
        fprintf(stderr,
                "iod_bmi_handshake: Request recv failure (bad state).\n");
        exit(1);
    }

    *peer_addr = request_info.addr;

    printf("zoidfs_op_id: %d\n", *(int *)request_info.buffer);
    printf("tag: %d\n", request_info.tag);

    BMI_unexpected_free(*peer_addr, request_info.buffer);

    /* create an ack */
    sendbuf = BMI_memalloc(*peer_addr, max_bytes, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "iod_bmi_handshake: BMI_memalloc failed().\n");
        exit(1);
    }
    memset(sendbuf, 0, max_bytes);

    /* post the ack */
    ret = bmi_comm_send(*peer_addr, sendbuf, max_bytes, tag+1, context);

    /* free up the message buffers */
    BMI_memfree(*peer_addr, sendbuf, max_bytes, BMI_SEND);

    return ret;
}



int main(int argc, char **argv) {
    int ret;
    void *recvbuf;
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    bmi_context_id context;
    bmi_size_t recvbuflen = BUFSIZE;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        exit(1);
    }

    /* Initialize BMI */
    ret = init_bmi(argv[1]);

    handle_error(BMI_open_context(&context), "BMI_open_context"); 

    ret = iod_bmi_handshake(&peer_addr, tag, context);

    /* Create a buffer to recv into */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "iod: BMI_memalloc() failed.\n");
        exit(1);
    }
    memset(recvbuf, 0, recvbuflen);

    /* Recv the data from the client */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+2, context);

    printf("%s", (char *)recvbuf);

    /* Cleanup */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);

    /* Finalize BMI */
    BMI_close_context(context);
    finalize_bmi();

    return EXIT_SUCCESS;
}



