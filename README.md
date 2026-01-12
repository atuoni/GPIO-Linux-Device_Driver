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
For the development of the project, the Raspberry Pi Zero 2W board was used with the Linux Debian Trixie operating system (Kernel 6.12.47+rpt-rpi-v8) installed.

The device driver and the application for accessing the Raspberry's GPIO were developed in C language and compiled on the Raspberry itself with the correct headers installed.

The LEDs and push-buttons needed to activate the driver were mounted on a breadboard and connected to the Raspberry Pi Zero 2W board, as can be seen in the electrical schematic in the Figure.

<img src="Circuit Picture.png" alt="Circuit">  

To enable the kernel to identify the GPIO pins used in the project, a Device Tree Overlay (.dts) file was developed with a declarative description of the hardware used.

The kernel reads the main Device Tree and the Device Tree Overlay created by the mygpio-overlay.dts file. After rebooting the board, the driver consults the new device tree to access the hardware and finds the "mygpio" node.

The code in the mygpio-overlay.dts file contains the GPIO configurations requested in the project, with 3 GPIOs as input pins (26, 13, and 19) and 3 GPIOs as output pins (20, 21, and 16).

The mygpio-overlay.dts file must be compiled using the following command in the terminal:

    dtc -@ -I dts -O dtb -o mygpio.dtbo mygpio-overlay.dts

The generated mygpio.dtbo file must be copied to the following Linux folder:

      sudo cp mygpio.dtbo /boot/firmware/overlays/

The following line should be added to the end of the /boot/firmware/config.txt file:

      dtoverlay=mygpio

Next, the Raspberry Pi board must be rebooted:

      sudo reboot

The mygpio.c file contains the driver code and is compiled using the Make command in the terminal.

The driver allows you to control the GPIOs on the Raspberry Pi in a generic way. 

The driver interacts with the hardware through the following functions:

    gpio_request() → Reserve the GPIO for the driver.
    gpio_direction() → defines the direction of the GPIO.
    gpio_get_value() / gpio_set_value() → read and write values
    gpio_to_irq() → Captures events on the GPIO.

In the driver, the Device Tree is read in probe() using:

    mygpio_pins[i] = of_get_named_gpio(np, "gpios", i);
    of_property_read_string_index(np, "directions", i, &dir);

This makes the driver flexible, since you only need to change the DTO for the driver to work without recompiling.

The driver registers a character device with:

    major = register_chrdev(0, DEVICE_NAME, &mygpio_fops);
    major → número principal do device (ex.: 236)
    mygpio_fops → estrutura que define funções que o app pode chamar:

    .open  -->  open the device
    .release --> close the device
    .unlocked_ioctl  --> Reads and writes GPIOs via APP.
    .poll         --> Allows the app to wait for events at the inputs.

Additionally, the driver automatically creates /dev/mygpio:

    mygpio_class = class_create("mygpio_class");
    mygpio_device = device_create(mygpio_class, NULL, MKDEV(major,0), NULL, "mygpio");

After compiling the driver, it must be loaded using the following command in the terminal:

    sudo insmod mygpio.ko
    
The `mygpio_event_app` application in userspace opens `/dev/mygpio` and blocks `poll()` and, upon receiving events from the inputs, updates the outputs.

The application must be compiled by GCC using the following command in the terminal:

    gcc -Wall -O2 mygpio_event_app.c -o mygpio_event_app

When the application is running in the terminal, the user activating SW1 turns on the red LED, SW2 turns on the yellow LED, and SW3 turns on the green LED.

### Files:
- mygpio-overlay.dts     (Device Tree Overlay File)
- mygpio.c  (driver source code)
- mygpio_event_app.c       (application source code)
- mygpio.ko       (object driver code)


## DEMONSTRATION VIDEO

[![Watch the video.](https://i.ytimg.com/an_webp/SFuYGhjP5fw/mqdefault_6s.webp?du=3000&sqp=CMiMkcsG&rs=AOn4CLCU3rtM4LuagC8opic7JR4njxSmoQ)](https://youtu.be/SFuYGhjP5fw)
