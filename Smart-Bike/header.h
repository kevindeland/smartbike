#include <stdio.h>

/*
* There are structures of the header in different packets. 
* This is a really tricky way to use the struct below.
* You can use it directly or change it as you like.
*/

#ifndef __header__
#define __header__

//type of packets
#define TYPE_DATA 1
#define TYPE_RREQ 2
#define TYPE_RREP 3
#define TYPE_RERR 4

//There are only three computers are required in this project, 
//so we assume that setting the address list length to 8 is enough. 
//You can change it if you work in a bigger ad-hoc network
#define MAX_ADDR_NUM 8
#define MAX_DATA_LEN 1024

typedef struct{
    unsigned int destination;
    unsigned int metric;
    unsigned int sequence_num;
} ADVERTISED_ENTRY;
            
//Route discovery
typedef struct{
    char type;
    int request_id; // initial sequence number
    unsigned int dest_ip; //dest ip
    int num_packets;
    int priority;
    ADVERTISED_ENTRY[MAX_INCOMING_PACKETS] updateTable;
} RREQ;

//Route reply
typedef struct{
    char type;
    unsigned int dest_ip;
    ADVERTISED_ENTRY[MAX_INCOMING_PACKETS] updateTable;
} RREP;

//Data packet
typedef struct{
    char type;
    int addr_num;
    int data_len;
    unsigned int dest_ip;
    void *data;
} DATA;
#endif
