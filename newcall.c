#include <linux/highmem.h>
#include <linux/sched/mm.h>

asmlinkage long new_call_get_size(int pid)
{
	struct task_struct *task;
	struct mm_struct *mm;
	printk(KERN_INFO "MYCALL pid %d\n", pid);
	task = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
	if (task == NULL)
	{
		// Print if task with the provided pid is not exists
		printk(KERN_INFO "Cannot find a process with that PID: %d\n", pid);
		return -1;
	}

	// Get mm_struct
	mm = task->mm;

	// Check mm_struct is not NULL
	if (!mm || !mm->env_end)
	{
		printk(KERN_INFO "NEWSYSCALL mm_struct = NULL");
		return -1;
	}

	return mm->env_end - mm->env_start;
}

asmlinkage long new_call(int pid, char __user *buf)
{
	struct task_struct *task;
	struct mm_struct *mm;
	char *page;
	unsigned long env_start, env_end, len, src = 0;
	int ret = 0;

	// Print pid
	printk(KERN_INFO "MYCALL pid %d\n", pid);
	task = get_pid_task(find_get_pid(pid), PIDTYPE_PID);

	if (task == NULL)
	{
		// Print if task with the provided pid is not exists
		printk(KERN_INFO "Cannot find a process with that PID: %d\n", pid);
		return -1;
	}

	// Get mm_struct
	mm = task->mm;

	// Check mm_struct is not NULL
	if (!mm || !mm->env_end)
	{
		printk(KERN_INFO "NEWSYSCALL mm_struct = NULL");
		return -1;
	}

	page = (char *)__get_free_page(GFP_KERNEL);

	if (!page)
	{
		return -ENOMEM;
	}

	if (!atomic_inc_not_zero(&mm->mm_users))
	{
		goto free;
	}

	// Reading start and end of environ
	down_read(&mm->mmap_sem);
	env_start = mm->env_start;
	env_end = mm->env_end;
	up_read(&mm->mmap_sem);

	printk(KERN_INFO "START NEWSYSCALL ENV:%ld\n", env_start);
	printk(KERN_INFO "END NEWSYSCALL ENV:%ld\n", env_end);
	len = env_end - env_start;
	printk(KERN_INFO "MYSYSCALL ENV_SIZE: %ld", len);

	// Reading into buffer
	while (src < len)
	{
		size_t this_len;
		int retval;

		this_len = min_t(size_t, PAGE_SIZE, len - src);

		retval = access_remote_vm(mm, (env_start + src), page, this_len, 0);

		if (retval <= 0)
		{
			ret = retval;
			break;
		}

		if (copy_to_user(buf, page, retval))
		{
			ret = -EFAULT;
			break;
		}

		ret += retval;
		src += retval;
		buf += retval;
	}
	mmput(mm);
free:
	free_page((unsigned long)page);
	printk(KERN_INFO "ENDED NEWSYSCALL");
	return ret;
}
