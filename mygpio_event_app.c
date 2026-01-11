#include <stdio.h>   //funçoes padrao de entrada e saida
#include <unistd.h>  //funções POSIX básicas (close, read, write)
#include <fcntl.h>   //flags e funções para open()
#include <sys/ioctl.h>  //definições para ioctl
#include <poll.h>      //estruturas e constantes para poll()
#include <stdlib.h>    //funções utilitarias

//comandos ioctl devem coincidir com os do driver
#define MYGPIO_READ   0     //ler o estado de um GPIO
#define MYGPIO_WRITE  1     //escrever em um GPIO

struct mygpio_ioctl  //estrutura de dados compartilhada com o driver
{
    unsigned int index;
    unsigned int value;
};

static int gpio_read(int fd, int index)   //função para leitura de GPIO via ioctl
{
    struct mygpio_ioctl d = { index, 0 }; //inicializa a estrutura com o indice do GPIO
    
	if (ioctl(fd, MYGPIO_READ, &d) < 0)  //chamada ioctl para leitura
	{
        perror("ioctl(MYGPIO_READ)");  //em caso de erro imprime mensagem do sistema
        exit(1);  //encerra o app
    }
    return d.value;   //retorna o valor lido do GPIO 
}

static void gpio_write(int fd, int index, int value)   //função para escrita em GPIO via ioctl
{
    struct mygpio_ioctl d = { index, value };       //preenche a estrutura com indice e valor do GPIO
    
	if (ioctl(fd, MYGPIO_WRITE, &d) < 0)    //chamda ioctl para escrita
	{
        perror("ioctl(MYGPIO_WRITE)");   //mensagem de erro do sistema
        exit(1);  //finaliza app
    }
}
//*************************************
/*Programa Principal */
//*************************************


int main(void)
{
    int fd = open("/dev/mygpio", O_RDWR);  //abre o char dev criado pelo driver
    if (fd < 0) 
	{
        perror("open");  //falha na abertura do device
        return 1;
    }

    setvbuf(stdout, NULL, _IONBF, 0);   //desabilita buffer de stdout para impressao imediata

    struct pollfd pfd =   //estrutura usada para monitoramento de eventos via poll()
	{
        .fd = fd,         //arquivo monitorado
        .events = POLLIN   //interesse em dados disponiveis
    };

    /* mapa dos índices conforme DTO */
    int inputs[3]  = {3, 4, 5}; /* GPIO 26, 13, 19 */
    int outputs[3] = {0, 1, 2}; /* GPIO 20, 21, 16 */

    printf("mygpio event app iniciada\n");   //mensagem inicial no terminal para usuário

    while (1) //loop
	{
        poll(&pfd, 1, -1);  //bloqueia até o driver sinalizar um evento

        if (pfd.revents & POLLIN) //verifica se o evento foi de leitura
		{
            for (int i = 0; i < 3; i++)  //processa todos os GPIOs de entrada
			{
                int val = gpio_read(fd, inputs[i]);  //Lê o estado do GPIO de entrada

                /* pull-up externo: ativo em nível baixo */
                int pressed = (val == 0);    
				
				/* 
                 * Como há pull-up externo:
                 *  - nível baixo (0) = botão pressionado
                 *  - nível alto (1) = botão solto
                 */

                gpio_write(fd, outputs[i], pressed);  //atualiza o GPIO de saída correspondente

                printf("Tecla GPIO index %d %s -> LED GPIO index %d %s\n", inputs[i], pressed ? "PRESSIONADA" : "SOLTA", outputs[i], pressed ? "ON" : "OFF");
				//imprime no terminal o estado
            }
        }
    }

    close(fd);  //fecha o device (nunca alcançado)
    return 0;
}
