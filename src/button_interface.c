#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <button_interface.h>

#define _1ms            1000

static void wait_press(void *object, Button_Interface *button)
{
    while (true)
    {
        if (!button->Read(object))
        {
            usleep(_1ms * 100);
            break;
        }
        else
        {
            usleep(_1ms);
        }
    }
}

bool Button_Run(void *object, POSIX_Queue *posix_queue, Button_Interface *button)
{
    char buffer[posix_queue->message_size];
    int state = 0;
    if(button->Init(object) == false)
        return false;

    if(POSIX_Queue_Init(posix_queue) == false)
        return false;

    while(true)
    {
        wait_press(object, button);

        state ^= 0x01;
        memset(buffer, 0, posix_queue->message_size);
        snprintf(buffer, posix_queue->message_size, "%d", state);
        POSIX_Queue_Send(posix_queue, buffer, strlen(buffer));
    }

    return false;
}
