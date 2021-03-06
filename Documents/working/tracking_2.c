
#include <freespace/freespace.h>
#include <freespace/freespace_util.h>
#include "appControlHandler.h"

#include <math.h>
#include "math/quaternion.h"
#include "math/vec3.h"

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <wiringPiI2C.h>
#include <unistd.h>
#include <pthread.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>


#define SLEEP    sleep(1)


#define LEFT_ON    0
#define LEFT_OFF   1
#define RIGHT_ON   2
#define RIGHT_OFF  3
#define LOW_LIGHT  4
#define HIGH_LIGHT 5
#define BRAKE      6
#define BRAKE_OFF  7
#define OFF        8
#define ALPHA .01
#define RADIANS_TO_DEGREES(rad) ((float) rad * (float) (180.0 / M_PI))
#define DEGREES_TO_RADIANS(deg) ((float) deg * (float) (M_PI / 180.0))

struct InputLoopState {
    pthread_t thread_;     // A handle to the thread
    pthread_mutex_t lock_; // A mutex to allow access to shared data
    int quit_;             // An input to the thread

    // The shared data updated by the thread
    struct freespace_MotionEngineOutput meOut_; // Motion data
    int updated_; // A flag to indicate that the motion data has been updated
};


struct CommLoopState {
    pthread_t thread_;     // A handle to the thread
    pthread_mutex_t lock_; // A mutex to allow access to shared data
    int quit_;             // An input to the thread

    // The shared data updated by the thread
    char message_[1024];
    int connected_;
    int size_;
    int updated_; // A flag to indicate that the motion data has been updated
};

// ============================================================================
// Local function prototypes
// ============================================================================
static void* inputThreadFunction(void*);
static void* commThreadFunction(void*);

static int getUpdateFromCommThread(struct CommLoopState* state, char buf[]);
static int getMotionFromInputThread(struct InputLoopState* state,
                                    struct freespace_MotionEngineOutput* meOut);
static void getEulerAnglesFromMotion(const struct freespace_MotionEngineOutput* meOut,
                                     struct Vec3f* eulerAngles);

// ============================================================================
// Public functions
// ============================================================================

/******************************************************************************
 * main
 * This example uses the synchronous API to access the device.
 * It assumes the device is already connected. It uses pthreads to put the 
 * operation of handling incoming messages from the device in a separate thread
 * from the main thread. It configures the device to produce fused motion outputs.
 */

int neutral = 0;
int left_wide =0;
int right_high = 0;


int left = 0;
int left_flag = 0;
int right = 0;
int right_flag = 0;
int braking = 0;

int fallen = 0;
int low_light = 0;
float neutral_roll = 500;
float neutral_pitch = 0;
float neutral_yaw = 0;


int main(int argc, char* argv[]) {
    struct CommLoopState commLoop;
    struct InputLoopState inputLoop;
    struct freespace_MotionEngineOutput meOut;
    struct Vec3f eulerAngles;
    struct MultiAxisSensor accel;
    int rc;
    int fd, result;
    
    // Flag to indicate that the application should quit
    // Set by the control signal handler
    int quit = 0;

    printVersionInfo(argv[0]);

    addControlHandler(&quit);

    // Initialize the freespace library
    rc = freespace_init();
    if (rc != FREESPACE_SUCCESS) {
        printf("Initialization error. rc=%d\n", rc);
        exit(1);
    }
    
    fd = wiringPiI2CSetup(0x04);

    // Setup the input loop thread
    memset(&inputLoop, 0, sizeof(struct InputLoopState));                      // Clear the state info for the thread
    pthread_mutex_init(&inputLoop.lock_, NULL);                                // Initialize the mutex
    pthread_create(&inputLoop.thread_, NULL, inputThreadFunction, &inputLoop); // Start the input thread

    memset(&commLoop, 0, sizeof(struct CommLoopState));                      // Clear the state info for the thread
    pthread_mutex_init(&commLoop.lock_, NULL);                                // Initialize the mutex
    pthread_create(&commLoop.thread_, NULL, commThreadFunction, &commLoop); // Start the input thread
    int update_bt;
    
    wiringPiI2CWrite(fd, OFF);
    delay(30);
    int connected = 0;
    
    while (connected ==0){
        pthread_mutex_lock(&commLoop.lock_);
        connected = commLoop.connected_;
        pthread_mutex_unlock(&commLoop.lock_);
    }
    
    // Run the game loop
    while (!quit) {
        // Get input.
        char buf[1024];
        memset(buf, 0, sizeof(buf));    
        rc = getMotionFromInputThread(&inputLoop, &meOut);
        update_bt = getUpdateFromCommThread(&commLoop,buf);
        if (update_bt){
            printf("test: %s\n",buf);
            if (strcmp(buf,"brake") ==0){
                if (!braking){
                    braking = 1;
                    result = wiringPiI2CWrite(fd,BRAKE);
                    delay(30);
                }
            }
            else if (strcmp(buf,"nbrake")==0){
                if (braking){
                    braking = 0;
                    result = wiringPiI2CWrite(fd, BRAKE_OFF );
                    delay(30);
                }
            }
            else if (strcmp(buf,"left")==0){
                if (left && !left_flag){
                    left = 0;
                    result = wiringPiI2CWrite(fd,LEFT_OFF);
                    delay(30);
                }
            }
            else if (strcmp(buf,"right")==0){
                if (right && !right_flag){
                    printf("turning off right\n");
                    right = 0;
                    result = wiringPiI2CWrite(fd,RIGHT_OFF);
                    delay(30);
                }
            }
            else if (strcmp(buf,"bright") == 0){
                result = wiringPiI2CWrite(fd,HIGH_LIGHT);
                delay(30);
            }
            else if (strcmp(buf,"low")==0){
                result = wiringPiI2CWrite(fd,LOW_LIGHT);
                delay(30);
            }
        }
        // If new motion was available, use it
        if (rc) {
            // Run game logic.
            getEulerAnglesFromMotion(&meOut, &eulerAngles);
            freespace_util_getAcceleration(&meOut, &accel);
            float roll  = RADIANS_TO_DEGREES(eulerAngles.x);
            float pitch = RADIANS_TO_DEGREES(eulerAngles.y); 
            float yaw   = RADIANS_TO_DEGREES(eulerAngles.z);


            // // Render.
            //  printf("%d: roll: %0.4f, pitch: %0.4f, yaw: %0.4f\taccel\tx: %0.4f, y: %0.4f, z: %0.4f\n",
            //        meOut.sequenceNumber,
            //         RADIANS_TO_DEGREES(eulerAngles.x),
            //         RADIANS_TO_DEGREES(eulerAngles.y),
            //         RADIANS_TO_DEGREES(eulerAngles.y),
            //         accel.x,
            //         accel.y,
            //         accel.z);
            // // Render.
            //  printf("%d: roll: %0.4f, pitch: %0.4f, yaw: %0.4f\tbools\tn: %d, l: %d, r: %d\n",
            //        meOut.sequenceNumber,
            //         neutral_roll,
            //         neutral_pitch,
            //         neutral_yaw,
            //         neutral,
            //         left_wide,
            //         right_high);


            if (neutral_roll == 500){
                neutral_roll = roll;
                neutral_pitch =pitch;
                neutral_yaw = yaw;

            }
            else if (!left_wide && !right_high) {
                neutral_roll = ALPHA * roll + (1-ALPHA) * neutral_roll;
                neutral_pitch = ALPHA * pitch + (1-ALPHA) * neutral_pitch;
                neutral_yaw = ALPHA * yaw + (1-ALPHA) * neutral_yaw;

            }

             // printf("%d: roll: %0.4f, pitch: %0.4f, yaw: %0.4f\taccel\tx: %0.4f, y: %0.4f, z: %0.4f\n",
             //       meOut.sequenceNumber,
             //        neutral_roll,
             //        neutral_pitch,
             //        neutral_yaw,
             //        accel.x,
             //        accel.y,
             //        accel.z);

            neutral = ( (pitch < (float) (neutral_pitch+20)) &&  (pitch >= (float) (neutral_pitch-20)) 
                                && (yaw > (float) (neutral_yaw-20)) && (yaw <= (float)(neutral_yaw +20)));

            right_high = ((pitch > (float) neutral_pitch+30) );
                                //&& (yaw > (float) (neutral_yaw-20)) && (yaw <= (float) (neutral_yaw +20)));


            left_wide = ( //(pitch < (float) 10) &&  (pitch > (float) -30) &&
                             (yaw < (float) (neutral_yaw-40)) );



            if ((right_flag == 1) && neutral) {
                printf("RIGHT TURN HAND LOWERED\n");
               // result = wiringPiI2CWrite(fd,RIGHT_OFF);
               // delay(30); 
                right_flag = 0;
            }

            else if ((left_flag == 1) && neutral) {
                printf("LEFT TURN HAND LOWERED\n");
   // printf("%d: roll: %0.4f, pitch: %0.4f, yaw: %0.4f\taccel\tx: %0.4f, y: %0.4f, z: %0.4f\n",
   //                 meOut.sequenceNumber,
   //                 RADIANS_TO_DEGREES(eulerAngles.x),
   //                 RADIANS_TO_DEGREES(eulerAngles.y),
   //                 RADIANS_TO_DEGREES(eulerAngles.z),
   //                 accel.x,
   //                 accel.y,
   //                 accel.z);
             //    result = wiringPiI2CWrite(fd,LEFT_OFF);
               //  delay(30);
                 left_flag = 0;
            }
            
            else if ((right == 0) && (right_flag == 0) &&(!left)&& right_high ){
                printf("RIGHT TURN HAND DETECTED\n");
                result = wiringPiI2CWrite(fd,RIGHT_ON);
                delay(30);
                right_flag = 1;
                right = 1;
            }
            else if ((left == 0) && (!right) && left_wide){
                printf("LEFT TURN HAND DETECTED\n");
                result = wiringPiI2CWrite(fd,LEFT_ON);
                delay(30);
                left_flag = 1;
                left = 1;
            }
            else if (neutral){
 //               printf("NEUTRAL\n");
            }
                        
//            if pitch
            fflush(stdout);
        }

        // Wait for "vsync"
        // SLEEP;
    }

    // Cleanup the input loop thread
    inputLoop.quit_ = 1;                     // Signal the thread to stop
    commLoop.quit_ = 1;
    //pthread_join(commLoop.thread_,NULL);
    pthread_mutex_destroy(&commLoop.lock_);
    pthread_join(inputLoop.thread_, NULL);   // Wait until it does
    pthread_mutex_destroy(&inputLoop.lock_); // Get rid of the mutex

    // Finish using the library gracefully
    freespace_exit();

    return 0;
}

// ============================================================================
// Local functions
// ============================================================================

/******************************************************************************
 * getUpdateFromCommThread
 *
 * @param state a pointer the the shared state information for the input loop thread
 * @param meOut a pointer to where to copy the motin information retrieved from the inpuit loop thread
 * @param return the updated flag from the input loop thread state
 */
static int getUpdateFromCommThread(struct CommLoopState * state,
                                    char buf[]) {
    int updated;

    pthread_mutex_lock(&state->lock_); 
    strncpy(buf,state->message_,state->size_);
    updated = state->updated_;           // Remember the updated_ flag
    state->updated_ = 0;                 // Mark the data as read
    pthread_mutex_unlock(&state->lock_); // Release ownership of the input loop thread's shared state information

    return updated;
}


/******************************************************************************
 * getMotionFromInputThread
 *
 * @param state a pointer the the shared state information for the input loop thread
 * @param meOut a pointer to where to copy the motin information retrieved from the inpuit loop thread
 * @param return the updated flag from the input loop thread state
 */
static int getMotionFromInputThread(struct InputLoopState * state,
                                    struct freespace_MotionEngineOutput * meOut) {
    int updated;

    pthread_mutex_lock(&state->lock_);   // Obtain ownership of the input loop thread's shared state information
    *meOut = state->meOut_;              // Copy the motion packet to the main thread
    updated = state->updated_;           // Remember the updated_ flag
    state->updated_ = 0;                 // Mark the data as read
    pthread_mutex_unlock(&state->lock_); // Release ownership of the input loop thread's shared state information

    return updated;
}

/******************************************************************************
 * getEulerAnglesFromMotion
 */
static void getEulerAnglesFromMotion(const struct freespace_MotionEngineOutput* meOut,
                                     struct Vec3f* eulerAngles) {
    struct MultiAxisSensor sensor;
    struct Quaternion q;

    // Get the angular position data from the MEOut packet
    freespace_util_getAngPos(meOut, &sensor);

    // Copy the data over to because both the util API and quaternion.h each have their own structs
    q.w = sensor.w;
    q.x = sensor.x;
    q.y = sensor.y;
    q.z = sensor.z;

    // The Freespace quaternion gives the rotation in terms of
    // rotating the world around the object. We take the conjugate to
    // get the rotation in the object's reference frame.
    q_conjugate(&q, &q);

    // Convert quaternion to Euler angles
    q_toEulerAngles(eulerAngles, &q);
}

// ============================================================================
// Thread functions
// ============================================================================

/******************************************************************************
 * CommThreadFunction
 */
static void* commThreadFunction(void* arg) {
    struct CommLoopState* state = (struct CommLoopState*) arg;
    
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buf[1024] = { 0 };
    int s, client, bytes_read;
    socklen_t opt = sizeof(rem_addr);

    // allocate socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    char address[18] = "00:1A:7D:DA:71:13";
    // bind socket to port 1 of the first available 
    // local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    str2ba( address, &loc_addr.rc_bdaddr);
    loc_addr.rc_channel = (uint8_t) 1;
    bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

    // put socket into listening mode
    listen(s, 1);

    // accept one connection
    client = accept(s, (struct sockaddr *)&rem_addr, &opt);

    ba2str( &rem_addr.rc_bdaddr, buf );
    fprintf(stderr, "accepted connection from %s\n", buf);

    pthread_mutex_lock(&state->lock_);

    strncpy(state->message_,buf,bytes_read);
    state->connected_ = 1;

    pthread_mutex_unlock(&state->lock_);

    memset(buf, 0, sizeof(buf));    


    while (!state->quit_){
        memset(buf, 0, sizeof(buf));    
        bytes_read = read(client, buf, sizeof(buf));
        if( bytes_read > 0 ) {
            pthread_mutex_lock(&state->lock_);
            strncpy(state->message_,buf,bytes_read);
            state->size_ = bytes_read;
            state->updated_ = 1;
            pthread_mutex_unlock(&state->lock_);
        }
    }
    printf("closing connection\n");
    // close connection
    close(client);
    close(s);
    return 0;
}



/******************************************************************************
 * inputThreadFunction
 */
static void* inputThreadFunction(void* arg) {
    struct InputLoopState* state = (struct InputLoopState*) arg;
    struct freespace_message message;
    FreespaceDeviceId device;
    int numIds;
    int rc;

    /** --- START EXAMPLE INITIALIZATION OF DEVICE -- **/
    // This example requires that the freespace device already be connected
    // to the system before launching the example.
    rc = freespace_getDeviceList(&device, 1, &numIds);
    if (numIds == 0) {
        printf("freespaceInputThread: Didn't find any devices.\n");
        exit(1);
    }

    rc = freespace_openDevice(device);
    if (rc != FREESPACE_SUCCESS) {
        printf("freespaceInputThread: Error opening device: %d\n", rc);
        exit(1);
    }

    rc = freespace_flush(device);
    if (rc != FREESPACE_SUCCESS) {
        printf("freespaceInputThread: Error flushing device: %d\n", rc);
        exit(1);
    }

    // Put the device in the right operating mode
    memset(&message, 0, sizeof(message));
    message.messageType = FREESPACE_MESSAGE_DATAMODECONTROLV2REQUEST;
    message.dataModeControlV2Request.packetSelect = 8; // MEOut
    message.dataModeControlV2Request.mode = 0;         // Set full motion
    message.dataModeControlV2Request.formatSelect = 0; // MEOut format 0
    message.dataModeControlV2Request.ff1 = 1;          // Acceleration fields
    message.dataModeControlV2Request.ff6 = 1;          // Angular (orientation) fields
    
    rc = freespace_sendMessage(device, &message);
    if (rc != FREESPACE_SUCCESS) {
        printf("freespaceInputThread: Could not send message: %d.\n", rc);
    }
    /** --- END EXAMPLE INITIALIZATION OF DEVICE -- **/

    // The input loop
    while (!state->quit_) {
        rc = freespace_readMessage(device, &message, 1000 /* 1 second timeout */);
        if (rc == FREESPACE_ERROR_TIMEOUT ||
            rc == FREESPACE_ERROR_INTERRUPTED) {
            continue;
        }
        if (rc != FREESPACE_SUCCESS) {
            printf("freespaceInputThread: Error reading: %d. Trying again after a second...\n", rc);
            SLEEP;
            continue;
        }

        // Check if this is a MEOut message.
        if (message.messageType == FREESPACE_MESSAGE_MOTIONENGINEOUTPUT) {
            pthread_mutex_lock(&state->lock_);

            // Update state fields.
            state->meOut_ = message.motionEngineOutput;
            state->updated_ = 1;

            pthread_mutex_unlock(&state->lock_);
        }
    }

    /** --- START EXAMPLE FINALIZATION OF DEVICE --- **/
    printf("\n\nfreespaceInputThread: Cleaning up...\n");
    memset(&message, 0, sizeof(message));
    message.messageType = FREESPACE_MESSAGE_DATAMODECONTROLV2REQUEST;
    message.dataModeControlV2Request.packetSelect = 1;  // Mouse packets
    message.dataModeControlV2Request.mode = 0;          // Set full motion
    rc = freespace_sendMessage(device, &message);
    if (rc != FREESPACE_SUCCESS) {
        printf("freespaceInputThread: Could not send message: %d.\n", rc);
    }

    freespace_closeDevice(device);
    /** --- END EXAMPLE FINALIZATION OF DEVICE --- **/
    
    // Exit the thread.
    return 0;
}
