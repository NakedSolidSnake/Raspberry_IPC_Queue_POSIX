#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <led_interface.h>

#define BUFFER_OFFSET	10

bool LED_Run(void *object, POSIX_Queue *posix_queue, LED_Interface *led)
{
	char buffer[posix_queue->message_size + BUFFER_OFFSET];
	int state;

	if(led->Init(object) == false)
		return false;

	if(POSIX_Queue_Init(posix_queue) == false)
		return false;

	while(true)
	{
		memset(buffer, 0, posix_queue->message_size + BUFFER_OFFSET);
		if(POSIX_Queue_Receive(posix_queue, buffer, posix_queue->message_size + BUFFER_OFFSET) == true)
		{
			sscanf(buffer, "%d", &state);
			led->Set(object, state);
		}
	}

	POSIX_Queue_Cleanup(posix_queue);

	return false;	
}
