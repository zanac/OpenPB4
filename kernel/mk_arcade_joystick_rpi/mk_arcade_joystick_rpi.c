/*
 *  Arcade Joystick Driver for RaspberryPi
 *
 *  Copyright (c) 2014 Matthieu Proucelle
 *
 *  Based on the gamecon driver by Vojtech Pavlik, and Markus Hiienkari
 */


/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include <linux/ioport.h>
#include <asm/io.h>


MODULE_AUTHOR("Matthieu Proucelle");
MODULE_DESCRIPTION("GPIO and MCP23017 Arcade Joystick Driver");
MODULE_LICENSE("GPL");

#define MK_MAX_DEVICES		9

#define GPIO_BASE        0x01c20800 /* GPIO controller */

/*#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define GPIO_READ(g)  *(gpio + 13) &= (1<<(g))*/

//#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

//#define GPIO_SET *(gpio+7)
//#define GPIO_CLR *(gpio+10)

//#define BSC1_BASE		(PERI_BASE + 0x804000)


/*
 * MCP23017 Defines
 */
#define MPC23017_GPIOA_MODE		0x00
#define MPC23017_GPIOB_MODE		0x01
#define MPC23017_GPIOA_PULLUPS_MODE	0x0c
#define MPC23017_GPIOB_PULLUPS_MODE	0x0d
#define MPC23017_GPIOA_READ             0x12
#define MPC23017_GPIOB_READ             0x13

/*
 * Defines for I2C peripheral (aka BSC, or Broadcom Serial Controller)
 */


#define START_READ	BSC_C_I2CEN|BSC_C_ST|BSC_C_CLEAR|BSC_C_READ
#define START_WRITE	BSC_C_I2CEN|BSC_C_ST

#define BSC_S_CLKT	(1 << 9)
#define BSC_S_ERR	(1 << 8)
#define BSC_S_RXF	(1 << 7)
#define BSC_S_TXE	(1 << 6)
#define BSC_S_RXD	(1 << 5)
#define BSC_S_TXD	(1 << 4)
#define BSC_S_RXR	(1 << 3)
#define BSC_S_TXW	(1 << 2)
#define BSC_S_DONE	(1 << 1)
#define BSC_S_TA	1

#define CLEAR_STATUS	BSC_S_CLKT|BSC_S_ERR|BSC_S_DONE

static volatile unsigned int gpio = 0xffffffff;
static volatile unsigned int reg = 0x01c20800; 
static volatile unsigned int dev_base = 0x0;
static volatile unsigned int offset;


struct mk_config {
    int args[MK_MAX_DEVICES];
    unsigned int nargs;
};

static struct mk_config mk_cfg __initdata;

module_param_array_named(map, mk_cfg.args, int, &(mk_cfg.nargs), 0);
MODULE_PARM_DESC(map, "Enable or disable GPIO, MCP23017, TFT and Custom Arcade Joystick");

struct gpio_config {
    int mk_arcade_gpio_maps_custom[12];
    unsigned int nargs;
};

static struct gpio_config gpio_cfg __initdata;

module_param_array_named(gpio, gpio_cfg.mk_arcade_gpio_maps_custom, int, &(gpio_cfg.nargs), 0);
MODULE_PARM_DESC(gpio, "Numbers of custom GPIO for Arcade Joystick");

enum mk_type {
    MK_NONE = 0,
    MK_ARCADE_GPIO,
    MK_ARCADE_GPIO_BPLUS,
    MK_ARCADE_MCP23017,
    MK_ARCADE_GPIO_TFT,
    MK_ARCADE_GPIO_CUSTOM,
    MK_MAX
};

#define MK_REFRESH_TIME	HZ/100

struct mk_pad {
    struct input_dev *dev;
    enum mk_type type;
    char phys[32];
    int mcp23017addr;
    int gpio_maps[12]
};

struct mk_nin_gpio {
    unsigned pad_id;
    unsigned cmd_setinputs;
    unsigned cmd_setoutputs;
    unsigned valid_bits;
    unsigned request;
    unsigned request_len;
    unsigned response_len;
    unsigned response_bufsize;
};

struct mk {
    struct mk_pad pads[MK_MAX_DEVICES];
    struct timer_list timer;
    int pad_count[MK_MAX];
    int used;
    struct mutex mutex;
};

struct mk_subdev {
    unsigned int idx;
};

static struct mk *mk_base;

static const int mk_data_size = 16;

static const int mk_max_arcade_buttons = 12;
static const int mk_max_mcp_arcade_buttons = 16;

// Map of the gpios :                     up, down, left, right, start, select, a,  b,  tr, y,  x,  tl
static const int mk_arcade_gpio_maps[] = { 0x49,  0x48,    0x47,  0x46,    0x49,    0x45,      0x44, 0x43, 0x42, 0x41, 0x40, 0x4b };
// 2nd joystick on the b+ GPIOS                 up, down, left, right, start, select, a,  b,  tr, y,  x,  tl
static const int mk_arcade_gpio_maps_bplus[] = { 0x49,  0x48,    0x47,  0x46,    0x49,    0x45,      0x44, 0x43, 0x42, 0x41, 0x40, 0x4b };
// Map of the mcp23017 on GPIOA            up, down, left, right, start, select, a,	 b
static const int mk_arcade_gpioa_maps[] = { 0,  1,    2,    3,     4,     5,	6,	 7 };

// Map of the mcp23017 on GPIOB            tr, y, x, tl, c, tr2, z, tl2
static const int mk_arcade_gpiob_maps[] = { 0, 1, 2,  3, 4, 5,   6, 7 };

// Map joystick on the b+ GPIOS with TFT      up, down, left, right, start, select, a,  b,  tr, y,  x,  tl
static const int mk_arcade_gpio_maps_tft[] = { 0x49,  0x48,    0x47,  0x46,    0x49,    0x45,      0x44, 0x43, 0x42, 0x41, 0x40, 0x4b };

static const short mk_arcade_gpio_btn[] = {
	BTN_START, BTN_SELECT, BTN_A, BTN_B, BTN_TR, BTN_Y, BTN_X, BTN_TL, BTN_C, BTN_TR2, BTN_Z, BTN_TL2
};

static const char *mk_names[] = {
    //NULL, "GPIO Controller 1", "GPIO Controller 2", "MCP23017 Controller", "GPIO Controller 1" , "GPIO Controller 1"
    NULL, "japb JAMMA Controller", "GPIO Controller 2", "MCP23017 Controller", "japb JAMMA Controller" , "japb JAMMA Controller"
};

/* GPIO UTILS */
/*static void setGpioPullUps(int pullUps) {
    *(gpio + 37) = 0x02;
    udelay(10);
    *(gpio + 38) = pullUps;
    udelay(10);
    *(gpio + 37) = 0x00;
    *(gpio + 38) = 0x00;
}*/

static void setGpioAsInput(int gpioNum) {
    //INP_GPIO(gpioNum);
}

/*static int getPullUpMask(int gpioMap[]){
    int mask = 0x0000000;
    int i;
    for(i=0; i<12;i++) {
        if(gpioMap[i] != -1){   // to avoid unused pins
            int pin_mask  = 1<<gpioMap[i];
            mask = mask | pin_mask;
        }
    }
    return mask;
}*/

/* I2C UTILS */

// Function to write data to an I2C device via the FIFO.  This doesn't refill the FIFO, so writes are limited to 16 bytes
// including the register address. len specifies the number of bytes in the buffer.

/*  ------------------------------------------------------------------------------- */


static void mk_gpio_read_packet(struct mk_pad * pad, unsigned char *data) {
    int i;

    for (i = 0; i < mk_max_arcade_buttons; i++) {
        if(pad->gpio_maps[i] != -1){    // to avoid unused buttons

            int key_code = pad->gpio_maps[i];
            //printk("key_code:%02x\n", key_code); //000007ff
            int port = (key_code>>5) & 0xf;
            int bit = (key_code>>0) & 0x1f;
            //printk("port:%08x, bit:%08x\n", port, bit); //000007ff
            //printk("addr:%08x\n", (gpio+0x10+0x24*port)); //000007ff
            unsigned int regData =  *((volatile unsigned int *) (gpio+0x10+0x24*port));
            //printk("regData:%08x\n", regData);
            //printk("regData (shift):%08x\n", (regData >> bit));

            int read = (regData>>bit)&0x1;

            //int read = GPIO_READ(pad->gpio_maps[i]);
            if (read == 0) data[i] = 1;
            else data[i] = 0;
        }else data[i] = 0;
    }

}

static void mk_input_report(struct mk_pad * pad, unsigned char * data) {
    struct input_dev * dev = pad->dev;
    int j;
    input_report_abs(dev, ABS_Y, !data[0]-!data[1]);
    input_report_abs(dev, ABS_X, !data[2]-!data[3]);
    for (j = 4; j < mk_max_arcade_buttons; j++) {
	input_report_key(dev, mk_arcade_gpio_btn[j - 4], data[j]);
    }
    input_sync(dev);
}

static void mk_process_packet(struct mk *mk) {

    unsigned char data[mk_data_size];
    struct mk_pad *pad;
    int i;

    for (i = 0; i < MK_MAX_DEVICES; i++) {
        pad = &mk->pads[i];
        if (pad->type == MK_ARCADE_GPIO || pad->type == MK_ARCADE_GPIO_BPLUS || pad->type == MK_ARCADE_GPIO_TFT || pad->type == MK_ARCADE_GPIO_CUSTOM) {
            mk_gpio_read_packet(pad, data);
            mk_input_report(pad, data);
        }
    }

}

/*
 * mk_timer() initiates reads of console pads data.
 */

static void mk_timer(unsigned long private) {
    struct mk *mk = (void *) private;
    mk_process_packet(mk);
    mod_timer(&mk->timer, jiffies + MK_REFRESH_TIME);
}

static int mk_open(struct input_dev *dev) {
    struct mk *mk = input_get_drvdata(dev);
    int err;

    err = mutex_lock_interruptible(&mk->mutex);
    if (err)
        return err;

    if (!mk->used++)
        mod_timer(&mk->timer, jiffies + MK_REFRESH_TIME);

    mutex_unlock(&mk->mutex);
    return 0;
}

static void mk_close(struct input_dev *dev) {
    struct mk *mk = input_get_drvdata(dev);

    mutex_lock(&mk->mutex);
    if (!--mk->used) {
        del_timer_sync(&mk->timer);
    }
    mutex_unlock(&mk->mutex);
}


int readKey(int key_code) {
  int port = (key_code>>5) & 0xf;
  int bit = (key_code>>0) & 0x1f;
  printk("mk_port:%08x, bit:%08x\n", port, bit); //000007ff
  printk("mk_addr:%08x\n", (gpio+0x10+0x24*port)); //000007ff
  unsigned int regData =  *((volatile unsigned int *) (gpio+0x10+0x24*port));
  printk("mk_regData:%08x\n", regData);
  printk("mk_regData (shift):%08x\n", (regData >> bit));
  
  return   (regData>>bit)&0x1;
}

static int __init mk_setup_pad(struct mk *mk, int idx, int pad_type_arg) {
    struct mk_pad *pad = &mk->pads[idx];
    struct input_dev *input_dev;
    int i, pad_type;
    int err;
    //char FF = 0xFF;
    pr_err("pad type : %d\n",pad_type_arg);

    pad_type = pad_type_arg;

    if (pad_type < 1 || pad_type >= MK_MAX) {
        pr_err("Pad type %d unknown\n", pad_type);
        return -EINVAL;
    }

    if (pad_type == MK_ARCADE_GPIO_CUSTOM) {

        // if the device is custom, be sure to get correct pins
        if (gpio_cfg.nargs < 1) {
            pr_err("Custom device needs gpio argument\n");
            return -EINVAL;
        } else if(gpio_cfg.nargs != 12){
             pr_err("Invalid gpio argument\n");
             return -EINVAL;
        }
    
    }

    pr_err("pad type : %d\n",pad_type);
    pad->dev = input_dev = input_allocate_device();
    if (!input_dev) {
        pr_err("Not enough memory for input device\n");
        return -ENOMEM;
    }

    pad->type = pad_type;
    pad->mcp23017addr = pad_type_arg;
    snprintf(pad->phys, sizeof (pad->phys),
            "input%d", idx);

    input_dev->name = mk_names[pad_type];
    input_dev->phys = pad->phys;
    input_dev->id.bustype = BUS_PARPORT;
    input_dev->id.vendor = 0x0001;
    input_dev->id.product = pad_type;
    input_dev->id.version = 0x0100;

    input_set_drvdata(input_dev, mk);

    input_dev->open = mk_open;
    input_dev->close = mk_close;

    input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);

    for (i = 0; i < 2; i++)
        input_set_abs_params(input_dev, ABS_X + i, -1, 1, 0, 0);
	for (i = 0; i < mk_max_arcade_buttons; i++)
		__set_bit(mk_arcade_gpio_btn[i], input_dev->keybit);

    mk->pad_count[pad_type]++;

    // asign gpio pins
    switch (pad_type) {
        case MK_ARCADE_GPIO:
            memcpy(pad->gpio_maps, mk_arcade_gpio_maps, 12 *sizeof(int));
            break;
        case MK_ARCADE_GPIO_BPLUS:
            memcpy(pad->gpio_maps, mk_arcade_gpio_maps_bplus, 12 *sizeof(int));
            break;
        case MK_ARCADE_GPIO_TFT:
            memcpy(pad->gpio_maps, mk_arcade_gpio_maps_tft, 12 *sizeof(int));
            break;
        case MK_ARCADE_GPIO_CUSTOM:
            memcpy(pad->gpio_maps, gpio_cfg.mk_arcade_gpio_maps_custom, 12 *sizeof(int));
            break;
    }

    // initialize gpio 
    for (i = 0; i < mk_max_arcade_buttons; i++) {
        printk("GPIO = %d\n", pad->gpio_maps[i]);
        if(pad->gpio_maps[i] != -1){    // to avoid unused buttons
             setGpioAsInput(pad->gpio_maps[i]);
             printk("zanac: qui manca inizializzazione\n");
        }                
    }
    //setGpioPullUps(getPullUpMask(pad->gpio_maps));
    printk("GPIO configured for pad%d\n", idx);

    readKey(0x49);





    err = input_register_device(pad->dev);
    if (err)
        goto err_free_dev;

    return 0;

err_free_dev:
    input_free_device(pad->dev);
    pad->dev = NULL;
    return err;
}

static struct mk __init *mk_probe(int *pads, int n_pads) {
    struct mk *mk;
    int i;
    int count = 0;
    int err;

    mk = kzalloc(sizeof (struct mk), GFP_KERNEL);
    if (!mk) {
        pr_err("Not enough memory\n");
        err = -ENOMEM;
        goto err_out;
    }

    mutex_init(&mk->mutex);
    setup_timer(&mk->timer, mk_timer, (long) mk);

    for (i = 0; i < n_pads && i < MK_MAX_DEVICES; i++) {
        if (!pads[i])
            continue;

        err = mk_setup_pad(mk, i, pads[i]);
        if (err)
            goto err_unreg_devs;

        count++;
    }

    if (count == 0) {
        pr_err("No valid devices specified\n");
        err = -EINVAL;
        goto err_free_mk;
    }

    return mk;

err_unreg_devs:
    while (--i >= 0)
        if (mk->pads[i].dev)
            input_unregister_device(mk->pads[i].dev);
err_free_mk:
    kfree(mk);
err_out:
    return ERR_PTR(err);
}

static void mk_remove(struct mk *mk) {
    int i;

    for (i = 0; i < MK_MAX_DEVICES; i++)
        if (mk->pads[i].dev)
            input_unregister_device(mk->pads[i].dev);
    kfree(mk);
}

static int __init mk_init(void) {
    /* Set up gpio pointer for direct register access */
    void* mem = NULL;
    printk("GPIO_BASE%08x\n", GPIO_BASE);
    dev_base = reg & (~(0x1000-1));
    printk("mk_dev_base:%08x\n", dev_base); //01c20000
    if ((mem = ioremap(dev_base, 0x1000)) == NULL) {
        pr_err("io remap failed\n");
        return -EBUSY;
    }
    offset = reg & ( (0x1000-1));
    gpio = ((unsigned int)mem)+offset;
 
    printk("mk_gpio_reg:%08x\n", gpio); //000007ff
    /* Set up i2c pointer for direct register access */
    if (mk_cfg.nargs < 1) {
        pr_err("at least one device must be specified\n");
        return -EINVAL;
    } else {
        mk_base = mk_probe(mk_cfg.args, mk_cfg.nargs);
        if (IS_ERR(mk_base))
            return -ENODEV;
    }
    return 0;
}

static void __exit mk_exit(void) {
    if (mk_base)
        mk_remove(mk_base);

    iounmap(gpio);
}

module_init(mk_init);
module_exit(mk_exit);
