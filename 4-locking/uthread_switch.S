	.text

/* Switch from current_thread to next_thread. Make next_thread
 * the current_thread, and set next_thread to 0.
 * Use eax as a temporary register; it is caller saved.
 */
	.globl thread_switch
thread_switch:
	/* YOUR CODE HERE */
  pushal # push all register (eax, ebx, ecx, edx, esp, ebp, esi, edi)
  movl current_thread, %eax
  movl %esp, (%eax) # Save current thread sp
  movl next_thread, %eax # switching 
  movl %eax, current_thread # switching
  movl $0, next_thread
  movl current_thread, %eax
  movl (%eax), %esp # Restore next thread sp
  popal # pop all register (eax, ebx, ecx, edx, esp, ebp, esi, edi)
	ret				/* pop return address from stack */
