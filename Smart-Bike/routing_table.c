#include <stdio.h>
#include "utils.h"
#include "header.h"
#include "routing_table.h"


void init_routing_table(){
    memset(routing_table, 0, sizeof(routing_table));
    for (int i = 0; i < MAX_ADDR_NUM; i++) {
        routing_table[i].metric = -1;
    }
    routing_table[local_ip_num%MAX_ADDR_NUM].metric = 0;
    routing_table[local_ip_num%MAX_ADDR_NUM].sequence_num = 100;
    routing_table[local_ip_num%MAX_ADDR_NUM].next_hop = LOCAL_IP_NUM;
    routing_table[local_ip_num%MAX_ADDR_NUM].time_stamp = clock();
    routing_table[local_ip_num%MAX_ADDR_NUM].destination = LOCAL_IP_NUM;
}

void update_routing_table(unsigned int dest, unsigned int next_hop, unsigned int metric, unsigned int sequence_num, unsigned int time_stamp) {
    ROUTING_ENTRY dest_entry = routing_table[dest%10];
    if (is_more_recent(dest%10, sequence_number)) {
        if (dest_entry.metric < metric){
            dest_entry.dest = dest;
            dest_entry.next_hop = next_hop;
            dest_entry.metric = metric;
            dest_entry.sequence_num = sequence_num;
            dest_entry.time_stamp = time_stamp;
        }
    }
    else {
        dest_entry.metric = -1;
    }   
}

int find_next_hop(unsigned int dest) {
    ROUTING_ENTRY dest_entry = routing_table[dest%10];
    if (dest_entry.metric == -1){
        return -1;
    } else if (/*is broken?*/) {
        return -1;
    } else {
        return dest_entry.next_hop;
    }
               
/* the sequence number must be equal or greater than
the int sequence number of the corresponding node already
in the routing table, or else the newly received routing
information in the update packet is stale and should be
discarded. */
}

int is_more_recent(int table_index, int sequence_num) {
   if (sequence_num >= routing_table[table_index].sequence_num) {
       return 1;
   }
   return 0;
}

//return packet with min hop
int calc_min_hop(unsigned int broadcast_src_ip) {
   ROUTING_ENTRY src = routing_table[broadcast_src_ip%10];
   int min_hop = INT_MAX;
   int index = -1;
   for (int i = 0; i < MAX_INCOMING_PACKETS; i++) {
       int metric = src.table[i].metric;
       if ((metric != -1) && (metric < min_hop)) {
           min_hop = metric;
           index = i;
       }
       // set to invalidate packet on next calc_min_hop
       src.table[i].metric = -1;
   }
   if (min_hop == INT_MAX){
       return -1;
   } else {
       src.dest = src.table[index].dest;
       src.next_hop = src.table[index].next_hop;
       src.metric = min_hop;
       src.sequence_num = src.table[index].sequence_num;
       return 0;
   }
}