#include <stdio.h>
#include <string.h>
#include "cache.h"
#include "header.h"
#include "flood.h"
#include "utils.h"
#include "routing_table.h"

void receive_packet(char *packet, int packet_len){
    char req = packet[0];
    switch (req){
        case TYPE_RREQ:
            RREQ packet_update = (RREQ) packet;
            REQUEST_ENTRY update;
            for (int i =0;i< packet_update.num_packets; i++){
                update = packet_update.requests[i];
                update_routing_table(update.dest, update.next_hop, update.metric, 
                    update.sequence_num, update.time_stamp);
            }
            if (packet_update.priority == HIGH){
                pthread_mutex_lock(&request_lock);
                enq(request_queue, (void*) packet);
                pthread_mutex_unlock(&request_lock); 
            }
        case TYPE_DATA:
            DATA packet_data = (DATA) packet;
            if (packet_data.dest == local_ip){
                printf("the content of packet received is: %s\n",packet.data);
            }
            else {
                  // forwarding since not intended for us but we need to update packet which i forgot
                    packet_data.next_hop = routing_table[dest%10].next_hop;
                    packet_data.metric -=1;
                  if (packet_data.next_hop == -1){
                    // BAD PATH
                    // we dont have any routing for it. which shouldnt happen
                    // idk man
                    // broken link?
                  }
                        pthread_mutex_lock(&data_lock);
                    enq(data_queue, (void*) packet_data);
                        pthread_mutex_unlock(&data_lock); 
            }
                // pthread_mutex_lock(&request_lock);
                // RREP packet = (RREP) enq(request_queue, (void*) packet);
                // pthread_mutex_unlock(&request_lock); 
           }
        case TYPE_RREP:
            //idk man
}  




