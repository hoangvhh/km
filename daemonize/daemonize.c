/*
 * daemonize - Daemonizing a kthread demo
 *
 * Written in 2012 by Prashant P Shah <pshah.mumbai@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated
 * all copyright and related and neighboring rights to this software
 * to the public domain worldwide. This software is distributed
 * without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software.
 * If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/kthread.h>

static int status = 1;

static DECLARE_WAIT_QUEUE_HEAD(wait);
static DECLARE_WAIT_QUEUE_HEAD(wait_once);

struct task_struct *hellot;
struct task_struct *oncet;
struct task_struct *endt;

static int hello_thread(void *data)
{
	int ret;

	/* Daemonize the thread */
	daemonize("hello_thread");

	/* Request delivery of SIGKILL */
	allow_signal(SIGKILL);

	/* Loop forever until status == 0 or SIGKILL signal received */
	for (;;) {
		/* wait till (2 second timeout or wake up received) AND (status = 0) */
		ret = wait_event_interruptible_timeout(wait, status == 0, HZ * 2);

		/* Die if I receive SIGKILL */
		if (signal_pending(current)) {
			printk(KERN_INFO "daemonize: SIGKILL signal received\n");
			break;
		}

		printk(KERN_INFO "daemonize: Hello World (%d)\n", ret);

		/* Exit if status is 0 */
		if (status == 0) {
			printk(KERN_INFO "daemonize: status == 0\n");
			break;
		}
	}

	printk(KERN_INFO "daemonize: end of daemonized thread\n");
	return 0;
}

static int once_thread(void *data)
{
	printk(KERN_INFO "daemonize: %s\n", __FUNCTION__);

	/* wait till wake_up received and status is 0 */
	wait_event_interruptible(wait_once, status == 0);

	printk(KERN_INFO "daemonize: end of once thread\n");
	return 0;
}


static int end_thread(void *data)
{
	printk(KERN_INFO "daemonize: %s\n", __FUNCTION__);

	/* wait for 15 seconds before setting the status to 0 */
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(HZ * 15);

	printk(KERN_INFO "daemonize: setting status to 0\n");
	status = 0;

	wake_up(&wait);		/* wake up the daemonize thread */

	wake_up(&wait_once);	/* wake up the once thread */

	return 0;
}

static int __init daemonize_init(void)
{
	printk(KERN_INFO "daemonize: %s\n", __FUNCTION__);

	/* start the daemonize thread */
	status = 1;
	hellot = kthread_run(&hello_thread, &status, "hello_thread");

	/* start the once thread */
	oncet = kthread_run(&once_thread, &status, "once_thread");

	/* start the end thread that will set the status to 0 after 10 seconds */
	endt = kthread_run(&end_thread, &status, "end_thread");

	return 0;
}

static void __exit daemonize_exit(void)
{
	printk(KERN_INFO "daemonize: %s\n", __FUNCTION__);
}

module_init(daemonize_init);
module_exit(daemonize_exit);

/* Note: Actual license is PUBLIC DOMAIN but since its compatible
 * with GPL so I have substituted "GPL" string here. Currently linux kernel
 * doesnt support public domain license
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Me");

