#include <stdio.h>
#include "send_packet.h"
#include "utils.h"
#include "header.h"
#include "flood.h"
#include "routing_table.h"



typedef unsigned int address_array[MAX_ADDR_NUM];



void sendImmediateUpdate(RREQ request) {
      printf("Important Routing Update received.\n");
    pthread_mutex_lock(&request_lock);
    enq(request_lock, (void *) request);
    pthread_mutex_lock(&request_lock);
}

void sendPeriodicUpdate (){
    printf("Sending update\n");
    RREQ request;
    char type;
    int request_id; // initial sequence number
    unsigned int dest_ip; //dest ip
    routing_table[local_ip_num%MAX_ADDR_NUM].sequence_num += 2;
    for (int i = 0; i < MAX_INCOMING_PACKETS; i++){
      ROUTING_ENTRY entry = routing_table[i];
      request.updateTable[i].destination = entry.destination;
      request.updateTable[i].metric = entry.metric + 1;
      request.updateTable[i].sequence_num = entry.sequence_num;
    }
    request.priority = LOW;
    pthread_mutex_lock(&request_lock);
    enq(request_lock, (void *) request);
    pthread_mutex_lock(&request_lock);
}



int send_packet(unsigned int ip, char *data, int data_len){
    printf("Attempting to send packet.");
    //add casing on this
    char type = (char) data;
    switch(type){
        case TYPE_DATA:
            address_array addr;
            pthread_mutex_lock(&lock);
            unsigned int path_len = find_next_hop(ip);
            pthread_mutex_unlock(&lock);
            if (path_len == -1){
                printf("No path detected in cache\n");
                RREQ request;
                request.type = TYPE_RREQ;
                request.request_id = LOCAL_IP_NUM;
                request.dest_addr = ip;
                request.addr_num = MAX_ADDR_NUM;
                addr[0] = ip;
                request.addr= addr;
                // build some broadcast message using the RREQ struct
                send_broadcast((char *)request, sizeof(RREQ));
                do {
                    pthread_mutex_lock(&lock);
                    int path_len = find_path(ip, &addr);
                    pthread_mutex_unlock(&lock);

                } while (path_len == -1);
            } else {
                printf("Path found in Cache...\n");
                printf("Building packet...")
                DATA packet = (DATA) data;
                unsigned int next_ip = addr[0];
                packet.addr = addr[1:MAX_ADDR_NUM-1];
                int status = send_unicast(ip, (char *)packet, sizeof(packet));                    
                return status;
                //send
                // or we need to just calculate next address with each hop
            }            
        // all the other nodes need to re-send a ack thing
        case TYPE_RREQ:
            //broadcast message?
            

        case TYPE_RREP:
    }

    
    int status = send_unicast(ip, data, data_len);
    //Todo
    //coooooooool
    
    return status;
}

int find_path(unsigned int dst, unsigned int *addr){
    int i, j;
    for(i=0;i<MAX_IP_CACHE;++i){
        if(path_cache_table[i].dst == dst){
            for(j=0; j<path_cache_table[i].path_len; ++j){
                addr[j] = path_cache_table[i].addr[j];
            }
            return path_cache_table[i].path_len;
        }
    }
    return -1;
}


int send_unicast(unsigned int ip32, char *packet,int packet_len){
    char ip[20];
    char buffer[MAXBUF];
    memset(ip, 0, sizeof(ip));
    memset(buffer, 0, sizeof(buffer));
    
    unsigned char *bytes = (unsigned char *) &ip32;
    sprintf (ip, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
    
    strcpy(buffer, ip);
    strcpy(buffer+IP_LENGTH, packet);
    int status = send_socket_packet(buffer, packet_len+IP_LENGTH);
    return status;
}

int send_broadcast(char *packet,int packet_len){
    char buffer[MAXBUF];
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, BROADCAST_IP);
    strcpy(buffer+IP_LENGTH, packet);
    int status = send_socket_packet(buffer, packet_len+IP_LENGTH);
    return status;
}

