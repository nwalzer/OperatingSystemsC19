//Nathan Walzer - nwalzer

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/pid.h>
//#include <asm/current.h>

struct ancestry {
	pid_t ancestors[10];
	pid_t siblings[100];
	pid_t children[100];
};

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall2)(void);

void get_ancestors(struct task_struct *curr, struct ancestry *history, int i, pid_t orig){ //recursively gets ancestors of curr
	if(curr->pid >= 1 && i < 10 && curr->pid != orig){ 
	//if a valid pid AND there's space in the array AND pid isn't the original calling pid
		history->ancestors[i] = curr->pid;
		printk(KERN_INFO "Found ancestor of %d, ID: %d\n", orig, curr->pid);
		i++; //increment index counter
		get_ancestors(curr->parent, history, i, orig); //get next parent
	}
	return;
}

asmlinkage long new_sys_cs3013_syscall2(unsigned short *target_pid, struct ancestry *response) {
	struct task_struct *found; //struct of target pid
	struct task_struct *tsPtr; //used in loops
	struct ancestry useless; //used for allocating space
	struct ancestry* toRet = &useless; //an ancestry "buffer"
	
	unsigned short target;
	int i = 0;
	
	if(copy_from_user(&target, target_pid, sizeof(unsigned short))){ //copy target pid from user
		return 1;
	}

	if(copy_from_user(toRet, response, sizeof(struct ancestry))){ //copy response struct from user
		return 1;
	}

	found = pid_task(find_vpid(target), PIDTYPE_PID);
	if(found == NULL){ //if not found, exit
		printk(KERN_INFO "Target Process not found\n");
		return 1;
	}

	printk(KERN_INFO "Target process %d found!\n", found->pid); //print success to syslog
	
	i = 0;
	list_for_each_entry(tsPtr, &(found->children), sibling){ //iterate through children list
		pid_t tmp = tsPtr->pid;
		if(i < 100 && tmp != 0){ //if space to store children, store it
			toRet->children[i] = tmp; //put into ancestry struct
			printk(KERN_INFO "Child #%d for process %u found, ID: %u\n", i, target, tmp);
			i++; //increment index
		} else if (i >= 100){ //if not space to store children
			printk(KERN_INFO "Child process of %u found, but children field is full, ID: %u\n", target, tmp);
		}
	}
	
	i = 0;
	list_for_each_entry(tsPtr, &(found->sibling), sibling){ //iterate through siblings list
		pid_t tmp = tsPtr->pid;
		if(i < 100 && tmp != 0){ //if space to store sibling, store it
			toRet->siblings[i] = tmp; //put into ancestry struct
			printk(KERN_INFO "Sibling #%d for process %u found, ID: %u\n", i, target, tmp);
			i++; //increment index
		} else if (i >= 100){ //if not space to store sibling
			printk(KERN_INFO "Sibling process of %u found, but siblings field is full, ID: %u\n", target, tmp);
		}
	}
	
	i = 0;
	if(found->pid > 1){//if target is capable of having a parent, find its ancestors
		get_ancestors(found->parent, toRet, i, target);
	}
	
	if(copy_to_user(response, toRet, sizeof(struct ancestry))){//return ancestry "buffer" to user
		return 1;
	}
	
  	return 0;
}


static unsigned long **find_sys_call_table(void) {
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;
  
  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;

    if (sct[__NR_close] == (unsigned long *) sys_close) {
      printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
	     (unsigned long) sct);
      return sct;
    }
    
    offset += sizeof(void *);
  }
  
  return NULL;
}

static void disable_page_protection(void) {
  /*
    Control Register 0 (cr0) governs how the CPU operates.

    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.

    It's good to be the kernel!
  */
  write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
  /*
   See the above description for cr0. Here, we use an OR to set the 
   16th bit to re-enable write protection on the CPU.
  */
  write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
  /* Find the system call table */
  if(!(sys_call_table = find_sys_call_table())) {
    /* Well, that didn't work. 
       Cancel the module loading step. */
    return -1;
  }
  
  /* Store a copy of all the existing functions */
  ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];

  /* Replace the existing system calls */
  disable_page_protection();

  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;
  
  enable_page_protection();
  
  /* And indicate the load was successful */
  printk(KERN_INFO "Loaded interceptor!");

  return 0;
}

static void __exit interceptor_end(void) {
  /* If we don't know what the syscall table is, don't bother. */
  if(!sys_call_table)
    return;
  
  /* Revert all system calls to what they were before we began. */
  disable_page_protection();
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
