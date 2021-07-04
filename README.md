<p align="center">
  <img src="https://www.tutorialsdaddy.com/wp-content/uploads/2016/11/linux-mmap.png">
</p>

# _Queue POSIX_

## Tópicos
* [Introdução](#introdução)
* [Implementação](#implementação)
* [launch_processes](#launch_processes)
* [button_interface](#button_interface)
* [led_interface](#led_interface)
* [Compilando, Executando e Matando os processos](#compilando-executando-e-matando-os-processos)
* [Compilando](#compilando)
* [Clonando o projeto](#clonando-o-projeto)
* [Selecionando o modo](#selecionando-o-modo)
* [Modo PC](#modo-pc)
* [Modo RASPBERRY](#modo-raspberry)
* [Executando](#executando)
* [Interagindo com o exemplo](#interagindo-com-o-exemplo)
* [MODO PC](#modo-pc-1)
* [MODO RASPBERRY](#modo-raspberry-1)
* [Matando os processos](#matando-os-processos)
* [Conclusão](#conclusão)
* [Referência](#referência)

## Introdução
Preencher

## Implementação

### *posix_queue.h*
```c
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
```

```c
bool POSIX_Queue_Init(POSIX_Queue *posix_queue);
bool POSIX_Queue_Send(POSIX_Queue *posix_queue, const char *message, int message_size);
bool POSIX_Queue_Receive(POSIX_Queue *posix_queue, char *buffer, int buffer_size);
bool POSIX_Queue_Cleanup(POSIX_Queue *posix_queue);
```
### *posix_queue.c*
```c
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
```

```c
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
```

```c
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
```

```c
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
```

Para demonstrar o uso desse IPC, iremos utilizar o modelo Produtor/Consumidor, onde o processo Produtor(_button_process_) vai escrever seu estado interno no arquivo, e o Consumidor(_led_process_) vai ler o estado interno e vai aplicar o estado para si. Aplicação é composta por três executáveis sendo eles:
* _launch_processes_ - é responsável por lançar os processos _button_process_ e _led_process_ atráves da combinação _fork_ e _exec_
* _button_interface_ - é reponsável por ler o GPIO em modo de leitura da Raspberry Pi e escrever o estado interno no arquivo
* _led_interface_ - é reponsável por ler do arquivo o estado interno do botão e aplicar em um GPIO configurado como saída

### *launch_processes*

No _main_ criamos duas variáveis para armazenar o PID do *button_process* e do *led_process*, e mais duas variáveis para armazenar o resultado caso o _exec_ venha a falhar.
```c
int pid_button, pid_led;
int button_status, led_status;
```

Em seguida criamos um processo clone, se processo clone for igual a 0, criamos um _array_ de *strings* com o nome do programa que será usado pelo _exec_, em caso o _exec_ retorne, o estado do retorno é capturado e será impresso no *stdout* e aborta a aplicação. Se o _exec_ for executado com sucesso o programa *button_process* será carregado. 
```c
pid_button = fork();

if(pid_button == 0)
{
    //start button process
    char *args[] = {"./button_process", NULL};
    button_status = execvp(args[0], args);
    printf("Error to start button process, status = %d\n", button_status);
    abort();
}   
```

O mesmo procedimento é repetido novamente, porém com a intenção de carregar o *led_process*.

```c
pid_led = fork();

if(pid_led == 0)
{
    //Start led process
    char *args[] = {"./led_process", NULL};
    led_status = execvp(args[0], args);
    printf("Error to start led process, status = %d\n", led_status);
    abort();
}
```

### *button_interface.h*
```c
typedef struct 
{
    bool (*Init)(void *object);
    bool (*Read)(void *object);
    
} Button_Interface;
```

```c
bool Button_Run(void *object, POSIX_Queue *posix_queue, Button_Interface *button);
```
### *button_interface.c*
```c
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
```

### *led_interface.h*
```c
typedef struct 
{
    bool (*Init)(void *object);
    bool (*Set)(void *object, uint8_t state);
} LED_Interface;
```

```c
bool LED_Run(void *object, POSIX_Queue *posix_queue, LED_Interface *led);
```

### *led_interface.c*
```c
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
```

## Compilando, Executando e Matando os processos
Para compilar e testar o projeto é necessário instalar a biblioteca de [hardware](https://github.com/NakedSolidSnake/Raspberry_lib_hardware) necessária para resolver as dependências de configuração de GPIO da Raspberry Pi.

## Compilando
Para faciliar a execução do exemplo, o exemplo proposto foi criado baseado em uma interface, onde é possível selecionar se usará o hardware da Raspberry Pi 3, ou se a interação com o exemplo vai ser através de input feito por FIFO e o output visualizado através de LOG.

### Clonando o projeto
Pra obter uma cópia do projeto execute os comandos a seguir:

```bash
$ git clone https://github.com/NakedSolidSnake/Raspberry_IPC_Queue_POSIX
$ cd Raspberry_IPC_Queue_POSIX
$ mkdir build && cd build
```

### Selecionando o modo
Para selecionar o modo devemos passar para o cmake uma variável de ambiente chamada de ARCH, e pode-se passar os seguintes valores, PC ou RASPBERRY, para o caso de PC o exemplo terá sua interface preenchida com os sources presentes na pasta src/platform/pc, que permite a interação com o exemplo através de FIFO e LOG, caso seja RASPBERRY usará os GPIO's descritos no [artigo](https://github.com/NakedSolidSnake/Raspberry_lib_hardware#testando-a-instala%C3%A7%C3%A3o-e-as-conex%C3%B5es-de-hardware).

#### Modo PC
```bash
$ cmake -DARCH=PC ..
$ make
```

#### Modo RASPBERRY
```bash
$ cmake -DARCH=RASPBERRY ..
$ make
```

## Executando
Para executar a aplicação execute o processo _*launch_processes*_ para lançar os processos *button_process* e *led_process* que foram determinados de acordo com o modo selecionado.

```bash
$ cd bin
$ ./launch_processes
```

Uma vez executado podemos verificar se os processos estão rodando atráves do comando 
```bash
$ ps -ef | grep _process
```

O output 
```bash
cssouza  16871  3449  0 07:15 pts/4    00:00:00 ./button_process
cssouza  16872  3449  0 07:15 pts/4    00:00:00 ./led_process
```
## Interagindo com o exemplo
Dependendo do modo de compilação selecionado a interação com o exemplo acontece de forma diferente

### MODO PC
Para o modo PC, precisamos abrir um terminal e monitorar os LOG's
```bash
$ sudo tail -f /var/log/syslog | grep LED
```

Dessa forma o terminal irá apresentar somente os LOG's referente ao exemplo.

Para simular o botão, o processo em modo PC cria uma FIFO para permitir enviar comandos para a aplicação, dessa forma todas as vezes que for enviado o número 0 irá logar no terminal onde foi configurado para o monitoramento, segue o exemplo
```bash
echo "0" > /tmp/queue_posix_fifo
```

Output do LOG quando enviado o comando algumas vezez
```bash
Apr 15 22:22:54 cssouza-Latitude-5490 LED QUEUE POSIX[30421]: LED Status: Off
Apr 15 22:23:18 cssouza-Latitude-5490 LED QUEUE POSIX[30421]: LED Status: On
Apr 15 22:23:19 cssouza-Latitude-5490 LED QUEUE POSIX[30421]: LED Status: Off
Apr 15 22:23:20 cssouza-Latitude-5490 LED QUEUE POSIX[30421]: LED Status: On
Apr 15 22:23:20 cssouza-Latitude-5490 LED QUEUE POSIX[30421]: LED Status: Off
Apr 15 22:23:22 cssouza-Latitude-5490 LED QUEUE POSIX[30421]: LED Status: On
```

### MODO RASPBERRY
Para o modo RASPBERRY a cada vez que o botão for pressionado irá alternar o estado do LED.

## Matando os processos
Para matar os processos criados execute o script kill_process.sh
```bash
$ cd bin
$ ./kill_process.sh
```

## Conclusão
Preencher

## Referência
* [Link do projeto completo](https://github.com/NakedSolidSnake/Raspberry_IPC_Queue_POSIX)
* [Mark Mitchell, Jeffrey Oldham, and Alex Samuel - Advanced Linux Programming](https://www.amazon.com.br/Advanced-Linux-Programming-CodeSourcery-LLC/dp/0735710430)
* [fork, exec e daemon](https://github.com/NakedSolidSnake/Raspberry_fork_exec_daemon)
* [biblioteca hardware](https://github.com/NakedSolidSnake/Raspberry_lib_hardware)
