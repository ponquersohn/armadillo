#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>		/* open */
#include <unistd.h>		/* exit */
#include <sys/ioctl.h>		/* ioctl */
#include <string.h>
#include <linux/fs.h> 

#include "../module/command_ioctl.h"

#define COMMAND_SET_UNKILLABLE "set_unkillable"
#define COMMAND_LOCK "lock"
#define COMMAND_UNLOCK "unlock"
#define COMMAND_SET_DEBUG "set_debug"

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
    
    if (strncmp(command, COMMAND_SET_UNKILLABLE, strlen(COMMAND_SET_UNKILLABLE)) == 0) {
        if (argc != 4) {
            printf("usage: set_unkillable <pid> <on/off>\n");
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
        armadillo_ioctl_set_pid_unkillable armadillo_ioctl_set_pid_unkillable_params;
        armadillo_ioctl_set_pid_unkillable_params.pid = pid;
        armadillo_ioctl_set_pid_unkillable_params.new_status = status;
        int ret_val;
        ret_val = ioctl(armadillo_device_file, ARMADILLO_IOCTL_SET_PID_UNKILLABLE, &armadillo_ioctl_set_pid_unkillable_params);

        if (ret_val < 0) {
            
            printf("ioctl_set_msg failed:%d\n", ret_val);
            exit(-1);
        }
        printf ("pid: %d, new_status: %d\n", pid, status);
        return ret_val;
        
    } else if (strncmp(command, COMMAND_LOCK, strlen(COMMAND_LOCK)) == 0) {
        if (argc != 3) {
            printf("usage: lock <password>\n");
            close(armadillo_device_file);
            return -1;
        }

        struct armadillo_ioctl_lock armadillo_ioctl_lock_params;
        strncpy(armadillo_ioctl_lock_params.secret, argv[2], ARMADILLO_MAX_PASSWORD_LENGTH);

        int ret_val;

        ret_val = ioctl(armadillo_device_file, ARMADILLO_IOCTL_LOCK, &armadillo_ioctl_lock_params);
        if (ret_val !=0) {
            printf("Unable to lock error: %d\n", ret_val);
        }
        return ret_val;
    } else if (strncmp(command, COMMAND_UNLOCK, strlen(COMMAND_UNLOCK)) == 0) {
    
        if (argc != 3) {
            printf("usage: unlock <password>\n");
            close(armadillo_device_file);
            return -1;
        }

        struct armadillo_ioctl_lock armadillo_ioctl_unlock_params;
        strncpy(armadillo_ioctl_unlock_params.secret, argv[2], ARMADILLO_MAX_PASSWORD_LENGTH);

        int ret_val;

        ret_val = ioctl(armadillo_device_file, ARMADILLO_IOCTL_UNLOCK, &armadillo_ioctl_unlock_params);
        if (ret_val !=0) {
            printf("Unable to unlock error: %d\n", ret_val);
        }
        return ret_val;
    } else if (strncmp(command, COMMAND_SET_DEBUG, strlen(COMMAND_SET_DEBUG)) == 0) {
    
        int ret_val;

        ret_val = ioctl(armadillo_device_file, ARMADILLO_IOCTL_SET_DEBUG);
        if (ret_val !=0) {
            printf("Unable to set debug error: %d\n", ret_val);
        } 
        return ret_val;
    }
    
    printf ("unknown command: %s\n", command); 
	close(armadillo_device_file);
}
