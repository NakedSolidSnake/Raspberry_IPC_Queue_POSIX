#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <button_interface.h>

#define BUFFER_SIZE     5
#define _1ms            1000

bool Button_Run(void *object, POSIX_Queue *posix_queue, Button_Interface *button)
{
    char buffer[BUFFER_SIZE];
    int state = 0;
    if(button->Init(object) == false)
        return false;

    if(POSIX_Queue_Init(posix_queue) == false)
        return false;

    while(true)
    {
        // while(true)
        // {
        //     if(!button->Read(object)){
        //         usleep(_1ms * 100);
        //         break;
        //     }else{
        //         usleep( _1ms );
        //     }
        // }

        sleep(1);

        state ^= 0x01;
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "%d", state);
        printf("Sending: %d\n", state);
        POSIX_Queue_Send(posix_queue, buffer, strlen(buffer));
    }

    printf("Exiting.\n");

    return false;
}
