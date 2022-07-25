#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/switch_to.h>
#include <linux/uaccess.h>

#include <linux/cdev.h>

#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/highmem.h>
//#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <linux/export.h>

#include "../../ledDefs.h"


int led_init(void);
void led_exit(void);
int led_open(struct inode *node, struct file *filp);
int led_release(struct inode *node, struct file *filp);
ssize_t led_read(struct file *, char *, size_t, loff_t *f_pos);
ssize_t led_write(struct file *, const char *, size_t, loff_t *f_pos);
long int led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

//Removed as it can be done by simply providing the physical address through a ioctl call
/*
int led_mmap(struct file *filp, struct vm_area_struct *vma); 

void led_vm_open(struct vm_area_struct *vma);
void led_vm_close(struct vm_area_struct *vma);*/


static struct class *c1;

int led_dd_major = LED_DD_MAJOR;
int led_dd_minor = LED_DD_MINOR;
int led_dd_count = LED_DD_COUNT;

int led_row_n = LED_DEFAULT_ROW_N;
int led_col_n = LED_DEFAULT_COL_N;

//module_param(led_dd_major, int, S_IRUGO);
//module_param(led_dd_minor, int, S_IRUGO);
//module_param(led_dd_count, int, S_IRUGO);

module_param(led_row_n, int, 0);
module_param(led_col_n, int, 0);

MODULE_AUTHOR("ZeD4805");
MODULE_LICENSE("Dual BSD/GPL");


struct led_dev{
    uint32_t* buffer;

    uint32_t** rows;

    position_t pos;

    dimensions_t dim;

    struct cdev chardev;

    int inUse;
};

typedef struct led_dev led_dev;

led_dev* led_devs = NULL;

struct page *bufferPages = NULL;
int pageOrder = 0;
unsigned long physAddr = 0;
uint32_t* totalBuffer = NULL;

struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .read = led_read, //led_read,
    .write = led_write, //led_write,
    .open = led_open,
    .release = led_release,
    .unlocked_ioctl = led_ioctl,
    //.mmap = led_mmap,
};


void setup_cdev(led_dev* dev, int index){
    int error;
    dev_t dev_no;
    
    dev_no = MKDEV(led_dd_major, led_dd_minor + index);

    cdev_init(&dev->chardev, &led_fops);
    dev->chardev.owner = THIS_MODULE;
    dev->chardev.ops = &led_fops;
    error = cdev_add(&dev->chardev, dev_no, 1);

    if(error != 0){
        printk(KERN_NOTICE "Error %d when adding %s%d\n", error, LED_DD_NAME, index);
    }
}

int __init led_init(void){
    dev_t dev = 0;
    int ret = 0;

    int i;

    int sizeNeeded;

    led_dev* device;

    if(led_dd_major != 0){ //if statically set
        dev = MKDEV(led_dd_major, led_dd_minor);
        
        ret = register_chrdev_region(dev, led_dd_count, LED_DD_NAME);
        
        printk(KERN_NOTICE "Registered chrdev with %d:%d, return is %d\n", MAJOR(dev), MINOR(dev), ret);

    }
    else{
        ret = alloc_chrdev_region(&dev, led_dd_minor, led_dd_count, LED_DD_NAME);
        led_dd_major = MAJOR(dev);

        dev = MKDEV(led_dd_major, led_dd_minor);

        printk(KERN_NOTICE "Registered chrdev with %d:%d, return is %d\n", MAJOR(dev), MINOR(dev), ret);
    }
    if(ret < 0){ //chrdev error
        printk(KERN_WARNING "Failed to register %s character device\n", LED_DD_NAME);
        return ret;
    }

    c1 = class_create(THIS_MODULE, LED_DD_NAME);

    sizeNeeded = led_row_n * led_col_n * sizeof(uint32_t);
    while (sizeNeeded > (PAGE_SIZE << pageOrder))    
        pageOrder++;

    bufferPages = alloc_pages(GFP_KERNEL | __GFP_COMP, pageOrder);
    if(bufferPages == NULL){
        printk(KERN_WARNING "Failed to allocate %d pages\n", 1 << pageOrder);
        return -ENOMEM;
    }

    printk(KERN_WARNING "Allocated %d pages\n", 1 << pageOrder);

    totalBuffer = (uint32_t *)kmap(bufferPages); //sleeps if none is available //maps the pages to kernel virtual address

    led_devs = (led_dev *)kcalloc(led_dd_count, sizeof(led_dev), GFP_KERNEL);
    if(led_devs == NULL){
        ret = -ENOMEM;
        led_exit();
        return ret;
    }

    for(i = 0; i < led_dd_count; i++){
        //TODO fill this out
        // buffer:  pointer to mapped pixel buffer
        // rows:    array to the rows of the matrix
        // row_n:   number of rows
        // col_n:   number of columns
        device = led_devs + i;

        device->buffer = totalBuffer;
        device->rows = NULL;

        device->pos.x = 0;
        device->pos.y = 0;

        device->dim.row_n = 0;//led_row_n;
        device->dim.col_n = 0;//led_col_n;
        //spin_lock_init(&device->lock);
        device->inUse = 0;

        //allocate a pointer array to each row
        
        setup_cdev(led_devs + i, i);

        device_create(c1, NULL, MKDEV(led_dd_major, led_dd_minor + i), NULL, LED_DD_NAME_FMT, i);

        printk(KERN_INFO "Cdev of %s%d setup\n", LED_DD_NAME, i);
    }

    printk(KERN_INFO "Led Driver started!\nCreated device with numbers %d:%d-%d\n", led_dd_major, led_dd_minor, led_dd_minor + led_dd_count);

    return 0;
}

void led_exit(void){
    int i;

    for(i = 0; i < led_dd_count; i++){
        led_dev* device = led_devs + i;
        if(device->rows != NULL){
            kfree(device->rows);
            //spin_unlock(&device->lock);
        }
        device_destroy(c1, MKDEV(led_dd_major, led_dd_minor + i));
        
    }

    kfree(led_devs);
    class_destroy(c1);

    if(totalBuffer != NULL){
        kunmap(bufferPages);
        //free_pages(bufferPages, pageOrder); //struct page has an atomic_t to count accesses, frees the page when it is not needed anymore
    }

    unregister_chrdev_region(MKDEV(led_dd_major, led_dd_minor), led_dd_count);

    printk(KERN_INFO "Exited from Led driver\n");
}

int led_open(struct inode *node, struct file* filp){
    led_dev *dev;
    //int returnOfLock;

    dev = container_of(node->i_cdev, led_dev, chardev);
    if(dev->inUse == 1){ //already gotten
        return -EBUSY;
    }
    dev->inUse = 1;
    filp->private_data = dev;
    


    /*returnOfLock = spin_trylock(&(dev->lock)); 
    if(returnOfLock == 0){
        printk(KERN_WARNING "Failed to obtain lock (%d)!\n", returnOfLock);
        return -1; 
    }

    printk(KERN_INFO "Obtained lock (%d)!\n", returnOfLock);*/
    return 0;
}

int led_release(struct inode *node, struct file* filp){
    led_dev *dev = filp->private_data;

    dev->inUse = 0;
    //spin_unlock(&(dev->lock));

    return 0;
}

ssize_t led_read(struct file *filp, char __user *buf, size_t count , loff_t *f_pos){
    led_dev *dev = filp->private_data;
    char* destination = buf;
    
    int maxCount = (dev->dim.col_n - dev->pos.y) *  (dev->dim.row_n - dev->pos.x) * sizeof(uint32_t); //buffer maximum
    
    int clip_width = (dev->dim.col_n - dev->pos.y) * sizeof(uint32_t);
    //int clip_height = dev->dim.row_n - dev->pos.x;

    int readN = 0;
    int rows = 0;
    
    /*if(buf < 0x10000){
        printk(KERN_WARNING "Tried to read from forbidden addresses! %p\n", buf);
        return -1;
    }*/

    count = (count) > maxCount? maxCount : (count);

    //printk(KERN_INFO "ledmatrix: led_read to\n\t*buf %llx\n\tcount %d (max count %d)\n\tx %d / y %d\n\trow_n %d / col_n %d\n", buf, count, maxCount, dev->pos.x, dev->pos.y, dev->dim.row_n, dev->dim.col_n);

    while(count > 0){
        clip_width = clip_width < count? clip_width : count;

        if(copy_to_user(destination, dev->rows[rows]/*+ dev->pos.y*/, clip_width)){ //the row already has the correct offset
            printk(KERN_WARNING "copy_to_user failed for (%p, %p, %ld)\n", destination, dev->rows[rows]/* + dev->pos.y*/, count);
            return -EFAULT;
        }
        destination += clip_width;
        readN += clip_width;
        count -= clip_width;
        rows++;

        //printk(KERN_INFO "\tTesting %d > 0\n", count);

    }

    return readN;
}

ssize_t led_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos){
    //do not record f_pos
    led_dev *dev = filp->private_data;
    char* source = (char *)buf;


    int maxCount = (dev->dim.col_n - dev->pos.y) *  (dev->dim.row_n - dev->pos.x) * sizeof(uint32_t);    

    int clip_width = (dev->dim.col_n - dev->pos.y) * sizeof(uint32_t);
    //int clip_height = dev->dim.row_n - dev->pos.x;


    //count = 0;
    int writeN = 0;
    int rows = 0;

    /*if(buf < 0x10000){
        printk(KERN_WARNING "Tried to write to forbidden addresses! %p\n", buf);
        return -1;
    }*/

    count = count > maxCount? maxCount : count;

    //printk(KERN_INFO "ledmatrix: led_write from\n\t*buf %llx\n\tcount %d (max count %d)\n\tx %d / y %d\n\trow_n %d / col_n %d\n", buf, count, maxCount, dev->pos.x, dev->pos.y, dev->dim.row_n, dev->dim.col_n);

    while(count > 0){
        clip_width = clip_width < count? clip_width : count;

        //printk(KERN_INFO "\tClip width %d, rows %d, dev->rows[rows] + dev->pos.y %d, source %d, count %d", clip_width, rows, dev->rows[rows] + dev->pos.y, source, count);

        if(copy_from_user(dev->rows[rows]/* + dev->pos.y*/, source, clip_width)){
            printk(KERN_INFO "copy_from_user failed for (%p, %p, %ld)\n", dev->rows[rows]/* + dev->pos.y*/, source, count);
            return -EFAULT;
        }
        source += clip_width;
        writeN += clip_width;
        count -= clip_width;
        rows++;

        //printk(KERN_INFO "\tTesting %d > 0\n", count);
    }

    return writeN;
}


long int led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
    led_dev* dev = filp->private_data;
    to_ioctl ioctlArg = {0};

    int j;
    int resizeReturn = 0;

    uint32_t** rowsTMP;

    switch (cmd)
    {
    case LED_IORPOS: //read position
        ioctlArg.pos = dev->pos;

        return put_user(ioctlArg.pos, (position_t *)arg);
    case LED_IOWPOS: //write position, never internally fails
        ioctlArg.uns64 = arg;

        //sanitize
        if(ioctlArg.pos.x >= led_row_n){
            dev->dim.row_n = 0;
        }
        else if(ioctlArg.pos.x + dev->dim.row_n > led_row_n){ //fix height
            dev->dim.row_n = led_row_n - ioctlArg.pos.x;
        }

        if(ioctlArg.pos.y >= led_col_n){
            dev->dim.col_n = 0;
        }
        else if(ioctlArg.pos.y + dev->dim.col_n > led_col_n){ //fix width
            dev->dim.col_n = led_col_n - ioctlArg.pos.y;
        }

        dev->pos = ioctlArg.pos;
        return 0;
    case LED_IORDIM: //read dimensions
        ioctlArg.dim = dev->dim;

        return put_user(ioctlArg.dim, (dimensions_t *)arg);
    case LED_IOWDIM: //write dimensions
        ioctlArg.uns64 = arg;
        //printk(KERN_NOTICE "Resize request, from %dx%d to %dx%d\n", dev->dim.row_n, dev->dim.col_n, ioctlArg.dim.row_n, ioctlArg.dim.col_n);

        //printk("Total buffer: %llx dim: (%d, %d)->(%d, %d)\n", totalBuffer, dev->dim.row_n, dev->dim.col_n, ioctlArg.dim.row_n, ioctlArg.dim.col_n);

        if(dev->pos.x < led_row_n){ //only allow changing if the value makes sense
            if(dev->pos.x + ioctlArg.dim.row_n > led_row_n){ //clip width
                ioctlArg.dim.row_n = led_row_n - dev->pos.x;

                resizeReturn = 1;
            } 
        }
        else
            ioctlArg.dim.row_n = 0;

        if(dev->pos.y < led_col_n){ 
            if(dev->pos.y + ioctlArg.dim.col_n > led_col_n){ //clip height
                ioctlArg.dim.col_n = led_col_n - dev->pos.y;
            
                resizeReturn = 1;
            }
        }
        else
            ioctlArg.dim.col_n = 0;
            
        //if they are actually different:
        if(dev->dim.row_n != ioctlArg.dim.row_n || dev->dim.col_n != ioctlArg.dim.col_n){ 
            rowsTMP = (uint32_t **)krealloc(dev->rows, sizeof(uint32_t *) * ioctlArg.dim.row_n, GFP_KERNEL);
            if(rowsTMP == NULL){
                return -1;
            }
            dev->rows = rowsTMP;
            //printk("[Corrected] Total buffer: %llx dim: (%d, %d)->(%d, %d)\n", totalBuffer, dev->dim.row_n, dev->dim.col_n, ioctlArg.dim.row_n, ioctlArg.dim.col_n);
            for(j = 0; j < ioctlArg.dim.row_n; j++){ //fill in the pointers
                dev->rows[j] = totalBuffer + ((dev->pos.x + j) * led_col_n + dev->pos.y);
                //printk("\tRows[%d] = %llx\n", j, dev->rows[j]);
            }
        }

        dev->dim = ioctlArg.dim;
        
        printk("Final dimensions were %dx%d\n", dev->dim.row_n, dev->dim.col_n);
        return resizeReturn;
    case LED_IORGDIM:
        ioctlArg.dim = (dimensions_t){led_row_n, led_col_n};
        return put_user(ioctlArg.dim, (dimensions_t *)arg);
    //case LED_IORPHYS:
        //return put_user(virt_to_phys(totalBuffer), (phys_addr_t *)arg);
    default:
        return -EINVAL;
    }

    return 0;
}

/*
struct vm_operations_struct led_vm_ops = {
    open: led_vm_open,
    close: led_vm_close,
};*/
/*
int led_mmap(struct file *filp, struct vm_area_struct *vma){
    led_dev* dev = filp->private_data;
    int ret = remap_pfn_range(vma, vma->vm_start, 
        virt_to_phys(dev->buffer) >> PAGE_SHIFT,//vma->vm_pgoff, //
        vma->vm_end - vma->vm_start,
        vma->vm_page_prot);

    printk(KERN_NOTICE "remap_pfn_range returned %d, with vma_start:%ld pgoff:%lld size %ld pageprot %lld\n", ret, vma->vm_start, virt_to_phys(dev->buffer) >> PAGE_SHIFT, vma->vm_end - vma->vm_start);
    if(ret == 0)
        return -EAGAIN; 

    vma->vm_ops = &led_vm_ops;
    return 0;
}

void led_vm_open(struct vm_area_struct *vma){

}

void led_vm_close(struct vm_area_struct *vma){

}*/

module_init(led_init);
module_exit(led_exit);