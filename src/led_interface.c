#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <led_interface.h>

#define BUFFER_SIZE		5

bool LED_Run(void *object, POSIX_Queue *posix_queue, LED_Interface *led)
{
	char buffer[BUFFER_SIZE];
	int state;

	if(led->Init(object) == false)
		return false;

	if(POSIX_Queue_Init(posix_queue) == false)
		return false;

	while(true)
	{
		if(POSIX_Queue_Receive(posix_queue, buffer, BUFFER_SIZE) == true)
		{
			sscanf(buffer, "%d", &state);
			// led->Set(object, state);
			printf("Read: %s\n", buffer);
		}
	}

	POSIX_Queue_Cleanup(posix_queue);

	return false;	
}
