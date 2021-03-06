#include <posix_queue.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <math.h>

#define QUEUE_PERMISSIONS 0660

bool POSIX_Queue_Init(POSIX_Queue *posix_queue)
{
    bool status = false;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = posix_queue->max_message;
    attr.mq_msgsize = posix_queue->message_size;
    attr.mq_curmsgs = 0;
    
    if(posix_queue->mode == write_mode)
    {
        posix_queue->id = mq_open(posix_queue->name, O_WRONLY, &attr);
    }
    else if(posix_queue->mode == read_mode)
    {   
        posix_queue->id = mq_open(posix_queue->name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr);
    }

    if(posix_queue->id >= 0)
        status = true;    

    return status;
}

bool POSIX_Queue_Send(POSIX_Queue *posix_queue, const char *message, int message_size)
{
    bool status = false;
    ssize_t written = 0;

    if(posix_queue && message_size > 0)
    {
        written = mq_send(posix_queue->id, message, fmin(message_size, posix_queue->message_size - 1), 0);
        if(written >= 0)
            status = true;
    }

    return status;
}

bool POSIX_Queue_Receive(POSIX_Queue *posix_queue, char *buffer, int buffer_size)
{
    bool status = false;
    ssize_t receive = 0;

    if(posix_queue && buffer && buffer_size > 0)
    {
        receive = mq_receive(posix_queue->id, buffer, fmax(buffer_size, posix_queue->message_size + 10), NULL);
        if(receive >= 0)
            status = true;
    }
        

    return status;
}

bool POSIX_Queue_Cleanup(POSIX_Queue *posix_queue)
{
    bool status = false;

    do 
    {
        if(!posix_queue)
            break;

        if(posix_queue->id <= 0)
            break;

        mq_close(posix_queue->id);
        mq_unlink(posix_queue->name);
        status = true;
    } while(false);

    return status;
}