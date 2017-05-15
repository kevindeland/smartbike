//


#include "bike.h"



volatile int turn_dir;
volatile queue_t send_queue;
int breaking;


pthread_mutex_t turning_lock;

int main() {
    //initialization
    packet breaking_packet;
    packet left_turn_packet;
    packet right_turning_packet;

    breaking_packet.type = BREAK;
    breaking_packet.dest = JACKET;
    breaking_packet.data = FULL_RED;


    struct freespace_message message;
    FreespaceDeviceId device;
    int numIds; // The number of device ID found
    int rc; // Return code
    struct MultiAxisSensor angVel;
    struct MultiAxisSensor linVel;

    // Flag to indicate that the application should quit
    // Set by the control signal handler
    int quit = 0;
    addControlHandler(&quit);
    rc = freespace_init();
    if (rc != FREESPACE_SUCCESS) {
        printf("Initialization error. rc=%d\n", rc);
        return 1;
    }

    printf("Scanning for Freespace devices...\n");
     // Get the ID of the first device in the list of availble devices
    rc = freespace_getDeviceList(&device, 1, &numIds);
    if (numIds == 0) {
        printf("Didn't find any devices.\n");
        return 1;
    }

    printf("Found a device. Trying to open it...\n");
    // Prepare to communicate with the device found above
    rc = freespace_openDevice(device);
    if (rc != FREESPACE_SUCCESS) {
        printf("Error opening device: %d\n", rc);
        return 1;
    }

    // Display the device information.
    printDeviceInfo(device);

    // Make sure any old messages are cleared out of the system
    rc = freespace_flush(device);
    if (rc != FREESPACE_SUCCESS) {
        printf("Error flushing device: %d\n", rc);
        return 1;
    }

    // Configure the device for motion outputs
    printf("Sending message to enable motion data.\n");
    memset(&message, 0, sizeof(message)); // Make sure all the message fields are initialized to 0.

    // TODO: IDK IF THIS WORKS, I THINK I GOT THE PACKETS RIGHT BUT WHO KNOWS
    message.messageType = FREESPACE_MESSAGE_DATAMODECONTROLV2REQUEST;
    message.dataModeControlV2Request.packetSelect = 8;  // MotionEngine Outout
    message.dataModeControlV2Request.mode = 0;          // Set full motion
    message.dataModeControlV2Request.formatSelect = 0;  // MEOut format 0
    message.dataModeControlV2Request.ff0 = 1;           // Pointer fields
    message.dataModeControlV2Request.ff2 = 1;           // Linear velocity fields
    message.dataModeControlV2Request.ff4 = 1;           // Angular velocity fields
    
//  
//  setting up Ad-hoc network
//  
    
    int status = get_local_ip();
    if (status < 0){
        printf("Could not find local IP, check ifconfig. For given implementation, make sure to use: 162.105.1.X\n");
        return 1;
    }

    init_flood();
    // currently implementing DSDV so set up routing information
    init_routing_table()
    open_send_socket();

    if (pthread_mutex_init(&turning_lock, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }    

    send_queue = new_queue();
    pthread_t thread_receive;
    pthread_create(&thread_receive,NULL,socket_receive,NULL);
    pthread_t thread_send;
    pthread_create(&thread_send,NULL,socket_send,NULL);

    
    rc = freespace_sendMessage(device, &message);
    if (rc != FREESPACE_SUCCESS) {
        printf("Could not send message: %d.\n", rc);
    }
        
    

    // A loop to read messages
    printf("Listening for messages.\n");
    // main loop - look for updates and handle
    //
    //
    //         z|
    //          |___x
    //          / 
    //         /y
    //
    // for braking and abrupt changes in speed look at y axis
    // Cadence - x
    // Z - crashing
        
    while (!quit){
        //measure the current sample x[k]   
        rc = freespace_readMessage(device, &message, 100);
        if (rc == FREESPACE_ERROR_TIMEOUT ||
            rc == FREESPACE_ERROR_INTERRUPTED) {
            // unimportant
            // the manual indicates that these errors should be addressed but dont cause any issues to continued use
            // We will ignore them
            continue;
        } 
        if (rc != FREESPACE_SUCCESS) {
            printf("Error reading: %d. Quitting...\n", rc);
            break;
        }
        // freespace_printMessage(stdout, &message); // This just prints the basic message fields
        if (message.messageType == FREESPACE_MESSAGE_MOTIONENGINEOUTPUT) {
            rc = freespace_util_getLinearVelocity(&message.motionEngineOutput, &linVel);
            rc = freespace_util_getAngularVelocity(&message.motionEngineOutput, &angVel);    
            if (rc == 0) {
                printf ("X: % 6.2f, Y: % 6.2f, Z: % 6.2f\n", linVel.x, linVel.y, linVel.z);
                float x_lin =linVel.x;
                float y_lin =linVel.y;
                float z_lin =linVel.z;
                float ampLin = (x_lin*x_lin) + (y_lin*y_lin) + (z_lin*z_lin);
                
                // In general, for slowing down, it is important to indicate to both car
                // however, we need to avoid slowing down from hills.
                // Usually X-axis movement is larger on hills. We will use this + simple hysterisis

                if (y_lin < breaking_threashold){
                    //simple threasholding on y acceleration
                    breaking = 1;

                        pthread_mutex_lock(&send_lock);
                    enqueue(send_queue, breaking_packet,HIGH);
                        pthread_mutex_unlock(&send_lock);
                }

                if (ampLin > Thresh){
                    // crash detected
                    displayCrash();
                    broadcastCrash();
                }


                float x_ang =angVel.x;
                float y_ang =linVel.y;
                float z_ang =linVel.z;

                // Check User has indicated a turn via gesture recognition
                    pthread_mutex_lock(&turning_lock);
                turn_state = turn_dir;
                    pthread_mutex_unlock(&turning_lock);

                if (turn_state == -1){

                    //TODO: check for peak angle

                    //TODO: detect return to center
                    
                    packet* packet_turn = malloc(sizeof(packet));

                    pthread_mutex_lock(&send_lock);
                    enqueue(send_queue,)
                    pthread_mutex_unlock(&send_lock);

                } else if (turn_state == 1){

                    //TODO: check for peak angle

                    //TODO: detect return to center

                }

                calculateCadence(x,y,z);


            }
        }

    }
}
