# DEVICE DRIVER PROJECT FOR GPIO CONTROL ON RASPBERRY PI ZERO 2W

## About
This work presents the development of a character device driver for controlling GPIO
pins in Linux-based embedded systems, using the Raspberry Pi platform.
The driver was designed to operate in an event-driven manner, exploiting the hardware
interrupt (IRQ) mechanism to detect state changes in pins configured as inputs, reducing
system resource consumption compared to continuous polling techniques. Hardware
configuration is performed using Device Tree Overlay (DTO), allowing flexibility and
decoupling between hardware and software. In addition, the interaction with a user-space
application is presented, which uses ioctl() system calls and the poll() mechanism for efficient
communication with the driver. The results demonstrate a modular, scalable solution aligned
with best practices for driver development in the Linux kernel.

## How it works
Para o desenvolvimento do projeto foi utilizado a placa Raspberry Pi Zero 2W com o
sistema operacional Linux Debian Trixie (Kernel 6.12.47+rpt-rpi-v8) instalado. 
O device driver e o aplicativo para acesso ao GPIO da Raspberry foram desenvolvidos em Linguagem
C e compilados na própria Raspberry com os headers corretos instalados.
Os leds e push-buttons necessários para acionamento do driver foram montados em
protoboard, e conectados a placa Raspberry Pi Zero 2W, como pode ser observado no
esquema elétrico da Figura.


Para que o kernel identifique os pinos de GPIO usados no projeto, foi desenvolvido
um arquivo de Device Tree Overlay (.dts) com uma decrição declarativa do hardware
utilizado.

O kernel lê o Device Tree principal e o Device Tree Overlay criado pelo arquivo
mygpio-overlay.dts, assim após reboot da placa o driver consulta a nova árvore de
dispositivos para acesso ao hardware e encontra o nó “mygpio”.

O código do arquivo mygpio-overlay.dts possui as configurações de GPIOs solicitadas
no projeto, sendo 3 GPIOs como pino de entrada (26, 13 e 19) e 3 GPIOs como pino de saída
(20, 21 e 16).

O arquivo mygpio-overlay.dts deve ser compilado através do seguinte comando no terminal:

    dtc -@ -I dts -O dtb -o mygpio.dtbo mygpio-overlay.dts

E o arquivo mygpio.dtbo gerado deve ser copiado para a seguinte pasta do Linux:

      sudo cp mygpio.dtbo /boot/firmware/overlays/

A linha abaixo deve ser adicionada ao final do arquivo /boot/firmware/config.txt:

      dtoverlay=mygpio

Em seguida deve ser realizado o reboot da placa Raspberry:

      sudo reboot

O arquivo mygpio.c contém o código do driver e é compilado através do comando
Make no terminal. 
O driver permite controlar os GPIOs no Raspberry Pi de forma genérica. O
driver interage com o hardware através da seguintes funções:

    gpio_request() → reserva o GPIO para o driver
    gpio_direction() → define a direção do GPIO
    gpio_get_value() / gpio_set_value() → lê e escreve valores
    gpio_to_irq() → captura eventos no GPIO

No driver, o Device Tree é lido no probe() usando:

    mygpio_pins[i] = of_get_named_gpio(np, "gpios", i);
    of_property_read_string_index(np, "directions", i, &dir);

Isso torna o driver flexível, pois basta alterar o DTO que o driver funciona sem
recompilar.
O driver registra um character device com:

    major = register_chrdev(0, DEVICE_NAME, &mygpio_fops);
    major → número principal do device (ex.: 236)
    mygpio_fops → estrutura que define funções que o app pode chamar:

    .open  -->  abre o dispositivo
    .release --> fecha o dispositivo
    .unlocked_ioctl  --> Lê e escreve GPIOs via APP
    .poll         --> Permite o APP esperar os eventos nas entradas

Além disso, o driver cria automaticamente /dev/mygpio:

    mygpio_class = class_create("mygpio_class");
    mygpio_device = device_create(mygpio_class, NULL, MKDEV(major,0), NULL, "mygpio");

Após a compilação do driver, esse deve ser carregado utilizando o seguinte comando no terminal:

    sudo insmod mygpio.ko
    
O aplicativo mygpio_event_app em espaço do usuário abre /dev/mygpio e bloqueia
em poll() e ao receber evento das entradas, atualiza as saídas. 

O aplicativo deve ser compilado pelo GCC através do seguinte comando no terminal:

    gcc -Wall -O2 mygpio_event_app.c -o mygpio_event_app

Com a execução do aplicativo no terminal, o acionamento de SW1 pelo usuário liga o
LED vermelho, SW2 o liga o led amarelo e SW3 liga o led verde.

### Files:
- mygpio-overlay.dts     (Device Tree Overlay File)
- mygpio.c  (driver source code)
- mygpio_event_app.c       (application source code)
- mygpio.ko       (object driver code)


## DEMONSTRATION VIDEO

[![Watch the video.](https://i9.ytimg.com/vi_webp/j767Vno2sWc/mq1.webp?sqp=CNj8xMcG&rs=AOn4CLA35gouhf2GBEex7KSgODKhKKkQ0g)](https://youtu.be/j767Vno2sWc)
