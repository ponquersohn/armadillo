#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>		/* open */
#include <unistd.h>		/* exit */
#include <sys/ioctl.h>		/* ioctl */
#include <string.h>
#include <linux/fs.h> 

#include "../module/command_ioctl.h"

#define COMMAND_TOGGLE_UNKILLABLE "toggle_unkillable"

char convert_on_off(char * word) {
    int mytrue = strncmp(word, "on", strlen("on"));
    int myfalse = strncmp(word, "off", strlen("off"));
    if ((mytrue != 0) && ( myfalse != 0)) {
        return -1;
    }
    if (mytrue == 0) {
        return 1; //return true
    }
    
    return 0; //false
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
        printf("need commands\n");
        return -1;
    }

    int armadillo_device_file, ret_val;
	
    armadillo_device_file = open(ARMADILLO_DEVICE_FILE_NAME, 0);
    if (armadillo_device_file < 0) {
        printf("Can't open device file: %s\n", ARMADILLO_DEVICE_FILE_NAME);
        exit(-1);
    }
    
    char * command = argv[1];
    
    if (strncmp(command, COMMAND_TOGGLE_UNKILLABLE, strlen(COMMAND_TOGGLE_UNKILLABLE)) == 0) {
        if (argc != 4) {
            printf("usage: toggle_pid_unkillable <pid> <on/off>\n");
            close(armadillo_device_file);
            return -1;
        }
        
        unsigned int pid = atoi(argv[2]);
        if ((pid == 0)||(pid==1)) {
            printf("couldnt convert or pid is 0 or 1\n");
            return -1;
        }
        int status = convert_on_off(argv[3]);
        if (status < 0 ) {
            printf ("couldnt convert status or status not <on/off>");
            close(armadillo_device_file);
            return (-1);
        } 
        struct armadillo_ioctl_toggle_pid_unkillable armadillo_ioctl_toggle_pid_unkillable_params;
        armadillo_ioctl_toggle_pid_unkillable_params.pid = pid;
        armadillo_ioctl_toggle_pid_unkillable_params.new_status = status;
        int ret_val;
        ret_val = ioctl(armadillo_device_file, ARMADILLO_IOCTL_TOGGLE_PID_UNKILLABLE, &armadillo_ioctl_toggle_pid_unkillable_params);

        if (ret_val < 0) {
            
            printf("ioctl_set_msg failed:%d\n", ret_val);
            exit(-1);
        }
        printf ("pid: %d, new_status: %d\n", pid, status);
        return 0;
        
    }
    
    printf ("unknown command: %s\n", command); 
	close(armadillo_device_file);
}
