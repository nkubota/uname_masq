#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/utsname.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

static struct proc_dir_entry *uname_masq_proc = NULL;
static struct new_utsname backup_utsname;

static int show_release(char *page, char **start, off_t offset,
			int count, int *eof, void *data)
{
	return snprintf(page, count, "%s\n", system_utsname.release);
}

static int store_release(struct file *file, const char __user * buffer,
			 unsigned long count, void *data)
{
	char buf[sizeof(system_utsname.release)];
	unsigned long len = min((unsigned long)sizeof(buf) - 1, count);

	memset(buf, 0, sizeof(*buf));

	if (copy_from_user(buf, buffer, len))
		return count;

	down_write(&uts_sem);
	memset(system_utsname.release, 0, sizeof(system_utsname.release));
	strncpy(system_utsname.release, buf, len);
	up_write(&uts_sem);

	return strnlen(buf, len);
}

static int show_machine(char *page, char **start, off_t offset,
			int count, int *eof, void *data)
{
	return snprintf(page, count, "%s\n", system_utsname.machine);
}

static int store_machine(struct file *file, const char __user * buffer,
			 unsigned long count, void *data)
{
	char buf[sizeof(system_utsname.machine)];
	unsigned long len = min((unsigned long)sizeof(buf) - 1, count);

	memset(buf, 0, sizeof(*buf));

	if (copy_from_user(buf, buffer, len))
		return count;

	down_write(&uts_sem);
	memset(system_utsname.machine, 0, sizeof(system_utsname.machine));
	strncpy(system_utsname.machine, buf, len);
	up_write(&uts_sem);

	return strnlen(buf, len);
}

static int __init uname_masq_init(void)
{
	struct proc_dir_entry *e;

	printk(KERN_INFO "load uname masquarade\n");

	down_read(&uts_sem);
	memcpy(&backup_utsname, &system_utsname, sizeof(struct new_utsname));
	up_read(&uts_sem);

	uname_masq_proc = proc_mkdir("uname_masq", NULL);
	if (uname_masq_proc == NULL) {
		printk(KERN_ERR "Unable to create proc directory\n");
		return -EIO;
	}

	e = create_proc_entry("release", S_IFREG | S_IRUGO | S_IWUSR, uname_masq_proc);
	if (!e) {
		remove_proc_entry("uname_masq", NULL);
		uname_masq_proc = NULL;
		return -EIO;
	}
	e->read_proc = show_release;
	e->write_proc = store_release;
	e->data = NULL;

	e = create_proc_entry("machine", S_IFREG | S_IRUGO | S_IWUSR, uname_masq_proc);
	if (!e) {
		remove_proc_entry("release", uname_masq_proc);
		remove_proc_entry("uname_masq", NULL);
		uname_masq_proc = NULL;
		return -EIO;
	}
	e->read_proc = show_machine;
	e->write_proc = store_machine;
	e->data = NULL;

	return 0;
}

static void __exit uname_masq_exit(void)
{
	printk(KERN_INFO "un-load uname masquarade\n");

	if (uname_masq_proc) {
		remove_proc_entry("machine", uname_masq_proc);
		remove_proc_entry("release", uname_masq_proc);
		remove_proc_entry("uname_masq", NULL);
		uname_masq_proc = NULL;
	}

	down_write(&uts_sem);
	memcpy(system_utsname.machine, backup_utsname.machine, sizeof(system_utsname.machine));
	memcpy(system_utsname.release, backup_utsname.release, sizeof(system_utsname.release));
	up_write(&uts_sem);
}

module_init(uname_masq_init);
module_exit(uname_masq_exit);

