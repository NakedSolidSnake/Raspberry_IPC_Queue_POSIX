#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <syslog.h>
#include <led_interface.h>

bool Init(void *object);
bool Set(void *object, uint8_t state);

int main(int argc, char *argv[])
{   

    LED_Interface led_interface = 
    {
        .Init = Init,
        .Set = Set
    };
    
    POSIX_Queue posix_queue = 
    {
        .name = "/queue",
        .max_message = 10,
        .message_size = 256,
        .mode = read_mode 
    };

    LED_Run(NULL, &posix_queue, &led_interface);
    
    return 0;
}

bool Init(void *object)
{
    (void)object; 
    return true;
}

bool Set(void *object, uint8_t state)
{
    (void)object;    
    openlog("LED QUEUE POSIX", LOG_PID | LOG_CONS , LOG_USER);
    syslog(LOG_INFO, "LED Status: %s", state ? "On": "Off");
    closelog(); 
    return true;
}
