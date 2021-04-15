#ifndef POSIX_QUEUE_H_
#define POSIX_QUEUE_H_

#include <stdbool.h>

typedef enum 
{
    write_mode,
    read_mode
} Mode;

typedef struct 
{
    char *name;
    int id;
    int max_message;
    int message_size;
    Mode mode;
} POSIX_Queue;

bool POSIX_Queue_Init(POSIX_Queue *posix_queue);
bool POSIX_Queue_Send(POSIX_Queue *posix_queue, const char *message, int message_size);
bool POSIX_Queue_Receive(POSIX_Queue *posix_queue, char *buffer, int buffer_size);
bool POSIX_Queue_Cleanup(POSIX_Queue *posix_queue);

#endif /* POSIX_QUEUE_H_ */
