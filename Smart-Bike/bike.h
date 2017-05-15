#include <stdio.h>
#include <stdlib.h>     
#include <time.h>     
#include <signal.h>    
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/vfs.h>    
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>        
#include <string.h>        
#include <unistd.h>     

#include "socket.h"
#include "receive_packet.h"
#include "header.h"
#include "flood.h"
#include "cache.h"

#include <freespace/freespace.h>
#include <freespace/freespace_util.h>
#include <freespace/freespace_printers.h>
#include "appControlHandler.h"

typedef struct  {
  char* data;
  unsigned int IP;
} packet;
