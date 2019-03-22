#include "GPIO.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cmath>
//#include <thread>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
using namespace std;

// Physical base address of GPIO
const unsigned gpio_address = 0x400d0000;
 
// Length of memory-mapped IO window
const unsigned gpio_size = 0xff;

#define NUM_THREADS 5

int RegisterRead(char *pBase, int offset)
{
    return * (int *) (pBase + offset);
}

int PushButtonGet(char *pBase)
{
   static bool state[5];
   for (int i = 0; i < 5; i++) {
	    int k = RegisterRead(pBase,0x16c+(4*i)); 
	    if (k == 1 && !state[i]) {
		    state[i] = true;
      cout << i+1 << endl;
		    return i+1;
	    } else if (k == 0) {
		    state[i] = false;
	    }
   }
   return 0;
}

char *Initialize(int *fd)
{
    *fd = open( "/dev/mem", O_RDWR);
    return (char *) mmap(
            NULL,
            gpio_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            *fd,
            gpio_address);
}

void Finalize(char *pBase, int fd)
{
    munmap(pBase, gpio_size);
    close(fd);
}

struct thread_data{
       GPIO* g; // the part to move
       int angle; // the angle we want to move the part to
};

void *createThreads(void *threadarg) {
     struct thread_data *my_data;
     my_data = (struct thread_data *) threadarg;
     GPIO* gpioObject = my_data->g;
     // Initialize
     while (1) {
       int angle = my_data->angle;
       int period = (angle * 10) + 600;
       gpioObject->GeneratePWM(20000, period, 1);
     }
     
      
//     while (PushButtonGet(pBase) != 5) {
//       gpioObject->GeneratePWM(20000, (1 * 10) + 600, 1);
//     }
     
     pthread_exit(NULL);
}    

int main() {
    int fd;
    char *pBase = Initialize(&fd);
    pthread_t threads[NUM_THREADS];
    struct thread_data td[NUM_THREADS];
    int rc;
    pthread_attr_t attr;
    void *status;
    // Initialize and set thread joinable
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 0; i < NUM_THREADS; i++) {
        if (i == 0) {
          td[i].g = new GPIO(13);
        } else if (i == 1) {
          td[i].g = new GPIO(10);
        } else if (i == 2) {
          td[i].g = new GPIO(11);
        } else if (i == 3) {
          td[i].g = new GPIO(12);
        } else if (i == 4) {
          td[i].g = new GPIO(0);
        }
        rc = pthread_create(&threads[i], NULL, createThreads, (void *)&td[i]);
        // error flag
        if (rc) {
           cout << "Error:unable to create thread," << rc << endl;
           exit(-1);
        }
        // move everything upright for initial position
        td[0].angle = 90;
        td[1].angle = 80;
        td[2].angle = 100;
        td[3].angle = 90;
        td[4].angle = 0;
        
    }
    
    // 13--base, 10--bicep, 11-elbow, 12-wrist, 0-gripper
    // 0-base, 1-bicep, 2-elbow, 3-wrist, 4-gripper
    while (PushButtonGet(pBase) != 5) {
    
    }
        //
        td[2].angle = 90;
        //turn base to 120
        td[0].angle = 120;
        td[4].angle = 180;
        usleep(500000);
        
        // move bicep and elbow down (prepare for pick up)
        td[1].angle = 110;
        usleep(300000);
        td[1].angle = 130;
        usleep(300000);
        td[1].angle = 160;
        td[2].angle = 110;
        usleep(500000);
        
        // close grip on bottle
        td[4].angle = 0;
        sleep(1);
        
        //move bicep back to 30 (wind back)
        td[1].angle = 100;
        usleep(300000);
        td[1].angle = 70;
        usleep(300000);
        td[1].angle = 30;
        usleep(500000);
        
        // turn base to prepare to throw bottle
        td[0].angle = 103;
        usleep(500000);
        
        //throw
        td[1].angle = 90;
        usleep(320000);
        td[4].angle = 180;
        usleep(500000);
        
        // back to upright
        td[0].angle = 90;
        td[1].angle = 90;
        td[2].angle = 90;
        td[3].angle = 90;
        td[4].angle = 180;
        usleep(500000);
        
        // turn base 
        td[0].angle = 50;
        usleep(500000);
        
        // move bicep and elbow down (prepare for pick up)
        td[1].angle = 110;
        usleep(300000);
        td[1].angle = 130;
        usleep(300000);
        td[1].angle = 160;
        td[2].angle = 110;
        usleep(500000);
        
        // close grip 
        td[4].angle = 0;
        sleep(1);
        
        //move bicep back to 30 (wind back)
        td[1].angle = 100;
        usleep(300000);
        td[1].angle = 70;
        usleep(300000);
        td[1].angle = 30;
        usleep(500000);
        
        // turn base to aim
        td[0].angle = 88;
        usleep(500000);
        
        //throw
        td[1].angle = 90;
        usleep(300000);
        td[4].angle = 180;
        
        usleep(500000);

    
    pthread_exit(NULL);
}



