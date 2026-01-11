#include <linux/module.h>     //macros básicas para módulos do kernel
#include <linux/platform_device.h>  //infraestrutura de platform driver
#include <linux/of.h>         //suporte a device tree
#include <linux/gpio/consumer.h>   //GPIO descriptor API
#include <linux/interrupt.h>     //infraestrutura de interrupçoes
#include <linux/fs.h>         //infraestrutura de sistemas de arquivos (char device)
#include <linux/cdev.h>       //infraestrutura de sistemas de arquivos (char device)
#include <linux/device.h>     //infraestrutura de sistemas de arquivos (char device)
#include <linux/uaccess.h>    //comunicação segura com user space
#include <linux/poll.h>       //suporte a poll/select
#include <linux/wait.h>       //suporte a fila de espera (wait queue)
#include <linux/slab.h>       //alocação dinâmica de memória

#define DRIVER_NAME "mygpio"    //nome interno do driver
#define CLASS_NAME  "mygpio"    //nome da classe em /sys/class

#define MYGPIO_READ   0         //leitura de um GPIO
#define MYGPIO_WRITE  1         //Escrita em um GPIO

#define MAX_GPIOS 8             //número máximo de GPIO suportados

/* ================= IOCTL ================= */

struct mygpio_ioctl   //estrutura usada para troca de dados via ioctl 
{
    unsigned int index;   //indice do GPIO no Device Tree
    unsigned int value;   //valor lógico (0 ou 1)
};

/* ================= DEVICE ================= */

struct mygpio_dev   //struct que representa o estado interno do driver
{
    struct gpio_desc *gpios[MAX_GPIOS];   //descritores de GPIO (abstração do hardware)
    int irq[MAX_GPIOS];                   //numero de IRQ associado a cada GPIO de entrada  
    bool is_output[MAX_GPIOS];            //indica se o GPIO é saída (true) ou entrada (false)
    int ngpios;                           //quantidade total de GPIOs configurados

    struct cdev cdev;                     //estrutura do character device
};

static struct mygpio_dev *mygpio;         //ponteiro global para o dispositivo
static dev_t mygpio_devno;                //numero major/minor do character device
static struct class *mygpio_class;        //classe para criação para /dev/mygpio

static DECLARE_WAIT_QUEUE_HEAD(mygpio_wq);    //fila de espera usada pelo poll()
static atomic_t mygpio_event = ATOMIC_INIT(0);     //flag atômica para sinalização de eventos

/* ================= IRQ HANDLER ================= */

static irqreturn_t mygpio_irq_handler(int irq, void *dev_id)   //handler de interrupção associado aos GPIOs de entrada
{
    atomic_set(&mygpio_event, 1);  // indica que ocorreu um evento de hardware
    wake_up_interruptible(&mygpio_wq);   //acorda processos bloqueados em poll()
    return IRQ_HANDLED;              //informa ao kernel que a IRQ foi tratada
}

/* ================= POLL ================= */

static unsigned int mygpio_poll(struct file *file, poll_table *wait)  //implementação do método poll
{
    poll_wait(file, &mygpio_wq, wait);     //coloca processo na fila de espera

    if (atomic_read(&mygpio_event))      //se ocorreu evento, sinaliza dados disponiveis
        return POLLIN | POLLRDNORM;

    return 0;                           //caso contrário permanece bloqueado
}

/* ================= IOCTL ================= */

static long mygpio_ioctl(struct file *file,  unsigned int cmd, unsigned long arg) //função ioctl: canal de comunicação app -> driver
{
    struct mygpio_ioctl data;     //instancia ioctl

    if (copy_from_user(&data, (void __user *)arg, sizeof(data)))  //copia dados do espaço de usuário para o kernel
        return -EFAULT;   //função falhou (acesso inválido)

    if (data.index >= mygpio->ngpios)  //verifica se o indice do gpio é válido
        return -EINVAL;        //valor do argumento passado invalido

    switch (cmd)   //seleciona o comando ioctl
	{

		case MYGPIO_READ:   
			data.value = gpiod_get_value(mygpio->gpios[data.index]);   //lê o valor lógico do GPIO
			atomic_set(&mygpio_event, 0);   //consome evento (limpa a flag)
			break;

		case MYGPIO_WRITE:               
			if (!mygpio->is_output[data.index])    //impede escrita em GPIO configurado como entrada
				return -EPERM;     //operação não permitida

			gpiod_set_value(mygpio->gpios[data.index], data.value);   //escreve valor lógico no GPIO
			break;

		default:
			return -EINVAL;  //valor do argumento passado invalido   
    }

    if (copy_to_user((void __user *)arg, &data, sizeof(data)))  //copia dados de volta para o espaço do usuário
        return -EFAULT;  //função falhou (acesso inválido)

    return 0;
}

/* ================= FILE OPS ================= */

static const struct file_operations mygpio_fops =  //estrutura com operações suportadas pelo drive de caracter
{
    .owner          = THIS_MODULE,            //referência ao módulo
    .unlocked_ioctl = mygpio_ioctl,           //comunicação por ioctl
    .poll           = mygpio_poll,            //suporte a poll/select
};

/* ================= PROBE ================= */

static int mygpio_probe(struct platform_device *pdev)  //função chamada qdo o kernel encontra o dispositivo no DT
{
    struct device *dev = &pdev->dev;
    int i, ret;

    mygpio = devm_kzalloc(dev, sizeof(*mygpio), GFP_KERNEL);  //aloca e zera a estrutura do dispositivo
    if (!mygpio)
        return -ENOMEM;  //erro de memória insuficiente

    /* Conta GPIOs definidos em gpios = < ... > */
    mygpio->ngpios = gpiod_count(dev, NULL);  //conta quantos GPIOs foram definidos no device tree
    if (mygpio->ngpios < 0)     
        return mygpio->ngpios;

    if (mygpio->ngpios > MAX_GPIOS)     //limita o número máximo de GPIO suportados
        mygpio->ngpios = MAX_GPIOS;

    for (i = 0; i < mygpio->ngpios; i++)  //inicializa cada GPIO
	{

			const char *dir;

			mygpio->gpios[i] = devm_gpiod_get_index(dev, NULL, i, GPIOD_ASIS);  //obtem o GPIO pelo indice no Device Tree
			if (IS_ERR(mygpio->gpios[i]))
				return PTR_ERR(mygpio->gpios[i]);

			of_property_read_string_index(dev->of_node, "directions", i, &dir);  // lê a direção do GPIO 'in' ou 'out'

			if (!strcmp(dir, "out")) // configura GPIO como saída
			{
				mygpio->is_output[i] = true;
				gpiod_direction_output(mygpio->gpios[i], 0);

			} 
			else     //configura GPIO como entrada
			{
				mygpio->is_output[i] = false;
				gpiod_direction_input(mygpio->gpios[i]);

				mygpio->irq[i] = gpiod_to_irq(mygpio->gpios[i]);     //converte GPIO para IRQ
				if (mygpio->irq[i] < 0)
					return mygpio->irq[i];

				ret = devm_request_irq(dev, mygpio->irq[i], mygpio_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, DRIVER_NAME, mygpio); //registra handle da interrupção
				if (ret)
					return ret;
			}
	}

    /* Character device */
    ret = alloc_chrdev_region(&mygpio_devno, 0, 1, DRIVER_NAME);  //aloca major / minor number
    if (ret)
        return ret;

    cdev_init(&mygpio->cdev, &mygpio_fops);   //inicializa o char device
    cdev_add(&mygpio->cdev, mygpio_devno, 1);  //registra o char dev

    mygpio_class = class_create(CLASS_NAME);   //cria /sys/class/mygpio
    device_create(mygpio_class, NULL, mygpio_devno, NULL, DRIVER_NAME);   //cria /dev/mygpio

    dev_info(dev, "mygpio driver carregado com sucesso\n");   //mensagem no dmesg
    return 0;
}

/* ================= REMOVE ================= */

static void mygpio_remove(struct platform_device *pdev)  //função chamada quando o driver é removido
{
    device_destroy(mygpio_class, mygpio_devno); 
    class_destroy(mygpio_class);
    cdev_del(&mygpio->cdev);
    unregister_chrdev_region(mygpio_devno, 1);

}

/* ================= DEVICE TREE ================= */

static const struct of_device_id mygpio_of_match[] =   //compatibilidade com device tree
{
    { .compatible = "amauri,mygpio" },
    { }
};
MODULE_DEVICE_TABLE(of, mygpio_of_match);

/* ================= PLATFORM DRIVER ================= */

static struct platform_driver mygpio_driver =  //registro no platform driver
{
    .probe  = mygpio_probe,
    .remove = mygpio_remove,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = mygpio_of_match,
    },
};

module_platform_driver(mygpio_driver); //macro que registra o driver no kernel

/* ================= MODULE INFO ================= */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amauri Tuoni");
MODULE_DESCRIPTION("Driver GPIO com IRQ + poll usando GPIO Descriptor API");
