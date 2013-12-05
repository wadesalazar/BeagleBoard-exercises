
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)

#include "fire.h"
#include "gpio.h"

/****************************************************************
 * Constants
 ****************************************************************/
 
#define POLL_TIMEOUT (60 * 1000) /* 60 seconds */
#define MAX 50		// Brightness

/****************************************************************
 * Global variables
 ****************************************************************/
int string_len = 320;
static FILE *rgb_fp;
int keepgoing = 1;	// Set to 0 when ctrl-c is pressed

/* Global thread environment */
int fade_env = 0;

/* Thread handle */
pthread_t fadeThread;

/****************************************************************
 * signal_handler
 ****************************************************************/
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepgoing = 0;
}

void display() {
	  fprintf(rgb_fp, "0 0 0 -1\n");
}

void rgb(int red, int green, int blue, int index, int us) {
	fprintf(rgb_fp, "%d %d %d %d", red, green, blue, index);
	if(us) {
	        display();
//    		printf("sending %d %d %d %d for %d\n", red, green, blue, index, us);
        }
        usleep(us);
}

void *fade(void *env) {
	int *tmp = env;
	int led = *tmp;	// Initial direction
    int i;
    for(i=0; i<MAX; i++) {
//		printf("Fire: %d, %d!\n", count, i);
		rgb( i, i,  i, led, 20000);
	}
    for(i=MAX; i>=0; i--) {
//		printf("Fire: %d, %d!\n", count, i);
		rgb( i, i,  i, led, 20000);
	}
//	pthread_exit(NULL);
}

/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{
    int led;
	if (argc < 2) {
		printf("Usage: fade LED#<gpio-pin>\n\n");
		exit(-1);
	}

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	rgb_fp = fopen("/sys/firmware/lpd8806/device/rgb", "w");
	setbuf(rgb_fp, NULL);	// Turn buffering off
	if(rgb_fp == NULL) {
		printf("Opening rgb failed\n");
		exit(0);
	}
    
    led = atoi(argv[1]);

    string_len = atoi(getenv("STRING_LEN"));
    if(string_len == 0) {
      string_len = 160;
    }
  printf("string_len = %d\n", string_len);
 
	while (keepgoing) {
            fade_env = rand() % string_len;
    		pthread_create(&fadeThread, NULL, &fade, &fade_env);
            sleep(1);
	}
	return 0;
}
