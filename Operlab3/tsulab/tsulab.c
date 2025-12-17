#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/timekeeping.h>
#include <linux/time.h>
#define procfs_name "tsu"
MODULE_LICENSE("GPL");

static ssize_t tsulab_read(struct file *filp, char __user *buf,
                          size_t count, loff_t *offp)
{

    char data[256];
    int len;

    struct timespec64 current_time;
    ktime_get_real_ts64(&current_time);

    // Устанавливаем время последнего полнолуния 5 декабря 2025 г.
    const time64_t last_time = 1764892800;
    // Синодический месяц в секундах

    const long long synodic_month_sec = 29 * 86400 + 12 * 3600 + 44 * 60;

    long long seconds_since = current_time.tv_sec - last_time;
    long long days_until = (synodic_month_sec - seconds_since % synodic_month_sec) / 86400;

    len = snprintf(data, sizeof(data),
               "Текущее время в секундах: %lld\n"
               "До следующей встречи Луны и Солнца: %lld дней\n",
               (long long)current_time.tv_sec, 
               (long long)days_until);

    if (*offp >= len)
        return 0;
    
    if (count > len - *offp)
        count = len - *offp;
    
    if (copy_to_user(buf, data + *offp, count))
        return -EFAULT;
    
    *offp += count;    
    return count;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops tsulab_fops = {
    .proc_read = tsulab_read,
};
#else
static const struct file_operations tsulab_fops = {
    .owner = THIS_MODULE,
    .read = tsulab_read,
};
#endif

static struct proc_dir_entry *tsulab_entry;

static int __init tsulab_init(void)
{
    pr_info("Да зравствует ТГУ\n");
    
    tsulab_entry = proc_create("tsulab", 0444, NULL, &tsulab_fops);
    if (!tsulab_entry) {
        pr_err("Не удалось создать /proc/tsulab\n");
        return -ENOMEM;
    }
    
    pr_info("/proc/tsulab создан\n");
    return 0;
}

static void __exit tsulab_exit(void)
{
    if (tsulab_entry)
        proc_remove(tsulab_entry);
    
    pr_info("tsulab: exit\n");
}

module_init(tsulab_init);
module_exit(tsulab_exit);