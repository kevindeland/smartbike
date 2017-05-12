#include <stdio.h>
#include <errno.h>
#include <wiringPiI2C.h>


int main(){
    int fd, result;
    fd = wiringPiI2CSetup(0x04);

    printf("initialize result: %d\n",fd);
    int x;
    char str_num[5];
    for (x =0; x<100; x++){
        //itoa(x,str_num,10);
        result = wiringPiI2CWrite(fd, x );
        delay(30);
        if (result == -1){ printf("fuck me\n");}
    }

}
