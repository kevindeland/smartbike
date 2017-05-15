#define INT_MAX 10
#define MAX_INCOMING_PACKETS 10
            
typedef struct{
    unsigned int destination;
    unsigned int next_hop;
    unsigned int metric;
    unsigned int sequence_num;
} ADVERTISED_ENTRY;

typedef struct{
    unsigned int destination;
    unsigned int next_hop;
    unsigned int metric;
    unsigned int sequence_num;
    unsigned int time_stamp;
} ROUTING_ENTRY;

ROUTING_ENTRY routing_table[MAX_ADDR_NUM];
int sequence_number = 0;



//brief: init the table
//called in socket.c
void init_routing_table();

//brief: insert a path information into the cache
void update_routing_table(unsigned int dest, unsigned int next_hop, unsigned int metric, unsigned int sequence_num, unsigned int time_stamp);


//brief: find next hop to get to final dest
//parameter dest: destination IP address
int find_next_hop(unsigned int dest);

//brief: check if the given update is newer/equal to current sequence number

int is_more_recent(int table_index, int sequence_num);


//brief: given list of packets choose shortest path

int calc_min_hop(unsigned int broadcast_src_ip);