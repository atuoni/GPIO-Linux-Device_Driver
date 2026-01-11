# Nome do módulo
obj-m := mygpio.o

# Diretório do kernel
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Compilar módulo
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Limpar arquivos de compilação
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

