## Configuration
[前人经验](https://blog.csdn.net/John_Snowww/article/details/129972288?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522169777031516800192255070%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=169777031516800192255070&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_ecpm_v1~rank_v31_ecpm-1-129972288-null-null.nonecase&utm_term=%E8%B8%A9%E5%9D%91%E4%B9%8B%E6%97%85&spm=1018.2226.3001.4450)

### Lab grade
1. Add the program to `UPROGS` in Makefile:  
![](https://cdn.jsdelivr.net/gh/PsyLinkist/LearningBlogPics@main/Materials/LearningBlogPics202310201055868.png)
2. run the program from the xv6 shell for self-test:
```shell
$ make qemu
...
init: starting sh
$ sleep 10
(nothing happens for a little while)
$
```
3. Check your work.
```shell
// runs all tests in one lab
$ make grade

// runs the grade tests for one assignment
$ ./grade-lab-util sleep
```

## Xv6 manual
### File system
Q: inode 是什么？为什么需要有inode？
A: inode 是文件最底层的对象。有了 inode 可以省下许多重复打开和关闭的麻烦。

Q: 什么叫做" run as a regular command "？为什么嵌入到 shell 运行的话就不需要新建子进程？为什么其他命令就采用需要新建子进程的方法？
A: " run as a regular command "在用户层面运行，shell 会调用`fork()`新建子进程，也因此可以更方便地扩展用户程序的功能。而嵌入到 shell 运行，则是使用内存、执行位置替换的方法，这是 shell 运行的逻辑。
```c
// sh.c
// Read and run input commands.
while(getcmd(buf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
        // Chdir must be called by the parent, not the child.
        buf[strlen(buf)-1] = 0;  // chop \n
        if(chdir(buf+3) < 0)
            fprintf(2, "cannot cd %s\n", buf+3);
        continue;
    }
    if(fork1() == 0)
        runcmd(parsecmd(buf));
    wait(0);
}
```

## C
### Local vars and stack-clearing
Variables defined in a function are local function and stored in stack. The storage would be released when the function's execution scope ends.

## Q&As
### Engineering
- Q: shell 的 command 及其变量是怎么传递并执行的？
每个 command 只接收到附在 command 后的变量？
那么 shell 对通过管道的变量与附在其后的变量接收顺序是什么样的？可以一试。  
    A: 在 bash 检测到对应的 command 名字，例如 xargs 后，从 xargs 开始的所有被空格分隔的变量都会传入到该 command 的函数中。

- Q: How to accept the output from left through `|` ? Give a test about the standard input 0.  
    A: Yes, the output from left would be written into `1` standard output, therefore, can be read from standard input `0`.

- Q: Execution sequence?  
    A: The xargs-ed command execution sequence would be: `<command> <origin var> <left output>`
### Supervisor mode & user mode
- Q: xv6 称应用（只能在 user 模式下执行。）如果想要执行硬件（如CPU）在 supervisor 下的功能，硬件就会把这个应用关掉。那如果应用需要用到这些功能呢？转为 supervisor 模式后执行和 user 模式执行有什么区别？多此一举转为 supervisor 模式再执行这些底层功能有什么好处？  
    A: 在 xv6 的系统调用中，会有对传入数据的检测，可能是这些检测为supervisor模式增添了一道防御。

### Read & write
- Q: 在读写 pipe 的时候，关闭了父母进程的 write 端，但为什么子进程的read 端还是被阻塞了?  
    A: 因为在 fork 的时候， fd 也被 fork 出一套，虽然 fd 的名字不同，但指向的 inode 是同一个，所以相当于 read 所读取的 pipe 还有一个 write 端没有关闭。

### Recursive function return
Recursive function will **automatically return to the previous recusive call** when finished execution.

## Run GDB
1. In first terminal, Cd xv6-labs-2023, run `make qemu-gdb`;
2. Open another terminal, run `gdb-multiarch`, wait for response;
3. Back to the first terminal, run `make qemu-gdb`;
4. Back to another terminal, run `target remote localhost:26000` and run `file kernel/kernel`, set break point.
- It is frustrating indeed. I believe the reason to blame is in some code in the qemu, the port is set to `25000`.
    

### Say you wanted to break every time the kernel enters the function syscall from kernel/syscall.c
1. Inside the gdb prompt: file kernel/kernel (this is a binary that has all kernel code)
2. Inside the gdb prompt: b syscall
3. Hit c. At this point you will start hitting the breakpoint above
4. Keep hitting c to see where the kernell hits the syscall function. You will how the output in the first window is progressing.

### Exception|Interrupt registers
- Scause register. Storing the cause(name) of the exception.
- Sepc register. Storing the address of the instruction that caused the exception.
- Stval register. Additional information, ussually the comment about the instruction that caused the exception.

### GDB command: 
- `x/3i 0xe16`:
    - `x`: Examine menory.
    - `/3i`: Display three instructions.
    - `0xe16`: Memory address.

- `p $pc`: 
    - `p`: Print the value of the proogram counter.

- `p/x $a1`:
    - `/x`: Format the output in hexadecimal.

- `x/2c $a1`:
    - `/2c`: Display two units of memory, each interpreted as a character (`c`).

# Reading and Lec notes
## xv6-Chapter4
### Process of a system call
#### `ecall`:
- Enter supervisor mode;
- Save `pc`;
- Get to stvec, start trap handler code (by fetching TRAMPOLINE page);
- Set `SIE`, prevent further interrupt.

#### `trampoline`: 
Mapped at both user page table and kernel page table.  
Having trap handler code `uservec` and `userret`, `uservec`:  
- Save 32 user registers into TRAPFRAME page;
- Switch to kernel page table (the only kernel data mapped is the `trapframe`, so everything has to be loaded from there);
- Set up stack for kernel C code;
- Jump to kernel C code (`usertrap`);

##### trampoline
The start of the kernel's trap handling code. Must be in user page table, since ecall doesn't change satp. And also mapped at the same virtual address in the kernel page table. 

##### trapframe
A kernel page into the user page table at a known virtual address, always the same: 0x3fffffe000.

##### switch to kernel page table.
- Q: What does kernel page table store? what is the difference between kernel page table and user pagetable?  
    A: Usually kernel page initialization would be done during the system's initialization. It manage the virtual address of kernel space.

#### `usertrap`
Execute the actual func code and save result into `a0`.

#### `usertrapret`
Prepare for next user->kernel transition. Set those trap handler related register back to normal.

#### `userret`
Jump to userret in `trampoline.S`, which mapped in both user and kernel page table. Thus we can switch back to user page table.
Restore 32 registers from trapframe. `a0` is the last, after which we can no longer get at TRAPFRAME page.

#### `sret`
- Copies `sepc` to `pc`;
- Changes mode to user
- Re-enables interrupts (copies `SPIE` to `SIE`)

## xv6-Chapter5
### Device drivers
#### 新硬件，新问题
- 硬件本身的交互困难；
- 如何让新硬件与cpu并行；
- 新硬件怎么使用中断来获取资源：  
    软件需要把当前工作暂停，并响应新硬件的请求。RISC-V这里就用跟系统调用一样的trap机制就行。

##### kernel 怎么看到中断请求的
device(send) -> PLIC(which dev) -> trap(save current info) -> usertrap()/kerneltrap()(handling) -> devintr(call corresponding hardware intr)

##### 通过 uart 发送字符到 console 的过程 (硬件代码下半部分)
kerneltrap() -> 
devintr() - plic_claim() -> 
uartintr() [read waiting input chars from UART hardware] - 
uartgetc() [check LSR, if ready, read RHR] -> 
consoleintr() [store input chars for consumption by consoleread()] -  
wakeup() [wake up consoleread() if a whole line or end-of-life] -> 
plic_complete()

##### scheduler 现在开始跑上半部分（现在开始需要cpu）
-- sh's read():
wakeup() -> consoleread() [return from sleep, see input char in cons.buf[cons.r]]

#### 关闭中断
intr_off(): w_sstatus(r_sstatus() & ~SSTATUS_SIE)

# Lab notes
Some notes when doing lab exercises.

## Lab: pgtbl
### Speed up system calls
#### Goal
Learn how to insert mappings into a pagetable.

#### Do
- Map one read-only page at USYSCALL when **each process** is created. (check `memlayout.h`)
   - Store a `struct usyscall` at the start.
   - Initialize, to store the PID of the current process.

#### Hints
- Choose read-only permission.
- Lifecycle of a new page to be done:
- Read through trapframe handling in `kernel/proc.c`
- Do.

#### Key words in Code
tag:  
#Speed up system calls  

keyword:  
trapframe  
struct usyscall  

test func:  
pgtbltest  

#### Q&As
- >Q: Which other xv6 system call(s) could be made faster using this shared page? Explain how.  
    A: Any system call purely retrieve information from kernel. Or any system call invokes the `copyout` function.  
    - Q: Which system call is accelerated because of this page? And why?
        A: `SYS_getpid`. The `ugetpid()` didn't call syscall to fetch the pid, which does not trap into kernel.

- Q: How to Speed up system calls?  
    A: By sharing data in a read-only region between userspace and the kernel, which **eliminates the need for kernel crossings.**

Map a page at USYSCALL (a virtual address defined in `memlayout.h`) when each process is created.
- Q: Where is the place process being created?  
    A: **allocproc** in `kernel/proc.c`. Do the allocation work. Seems an OS already has limited processes.

- Q: How to map a page? Where does it map to?  
    A: Map page:  
    `mappages()`  
    Where?  
    `p->usyscall`, this is the PA.  

- Q: How to initialize a page?  
    Follow the `trapframe` in `kernel/proc.c`. (And this is included in hint.)

- Q: Now I have mapped a usyscall page which has similar (not VA) location and other similar features to trapframe, How does it help with the speed of system calls?  
    A: Maybe it is applied exceptionally to specified function?

### Print a page table
#### Goal
Visualize RISC-V page tables, aid future debugging.

#### Do
- Define function `void vmprint(pagetabl_t pgtbl)` which print the pagetable.
- Insert `if(p->pid==1) vmprint(p->pagetable)` in `exec.c` just before `return argc`.

#### Hints
- Put `vmprint()` in `kernel/vm.c`.
- Use macros at the end of the file kernel/riscv.h
- Inspirational `freewalk`
- Remember prototype of `vmprint` in kernel/defs.h.
- `%p` to print out full 64-bit hex PTEs and addresses.
- The 1st number in each line represents the PTE index in its page-table page.

#### keywords
#Print a page table

#### Q&As
- > Q: For every leaf page in the vmprint output, explain what it logically contains and what its permission bits are. Figure 3.4 in the xv6 book might be helpful, although note that the figure might have a slightly different set of pages than the init process that's being inspected here.  

    A: 
    - Contents and Permissions: 
        - trampoline: r,x,v
        - trapframe: r,w,v
        - heap: r,w,u,v
        - stack: r,w,u,v
        - data: r,w,u,v
        - text: r,x,v


- Q: What is macros in kernel/riscv.h?  
    A: #define XXXX 

- Q: The 1st number in each line represents the PTE index in its page-table page. Where can I find the PTE index?  
    A: A page-table page can hold 512 PTEs, which means there are at most 512 indexs(0~511) of PTE. The Index should be **in the virtual address** and 9 bits in 27 bits of index for storing them. For example:  
    ![](https://cdn.jsdelivr.net/gh/PsyLinkist/LearningBlogPics@main/Materials/LearningBlogPics202312201743908.png) 

- Q: Where do I get the index then?  
    A: It is the index of PTEs, so it's the index of the PTE array. 

### Detect which pages have been accessed
#### Goal
- Aid some garbage collectors.

#### Do
- Implement `pgaccess()`, a system call that reports which pages have been accessed. (Via flag A) 
![flag A](https://cdn.jsdelivr.net/gh/PsyLinkist/LearningBlogPics@main/Materials/LearningBlogPics202312221158463.png)

#### Hints
- Three arguments: Starting virtual address of the first user page; Number of the pages; A user address as buffer.
- Read `pgaccess_test()` in `user/pgtbltest.c`, to see how `pgaccess` is used.
- Start by implementing `sys_pgaccess()` in `kernel/sysproc.c`.
- Parse arguments using `argaddr()` and `argint()`.
- Store a temporary buffer in the kernel, filling it with the right bits, then copy it to the user (via `copyout()`).
- An upper limit on the number of can-be-scanned pages. (Okay)
- `walk()` in `kernel/vm.c` is helpful for finding the right PTEs.
- Define `PTE_A` in `kernel/riscv.h`, Consult the "RISC-V privileged architecture manual" to determine its value.
- Clear `PTE_A` after checking.
- `vmprint()` may come in handy to debug.

#### keywords
#Detect accessed pages

#### Q&As
- Q: How does `pgaccess_test()` work?  
    A: By checking the abits argument after accessing, pgaccess_test() get to know whether or not the answer is correct.  
    ![](https://cdn.jsdelivr.net/gh/PsyLinkist/LearningBlogPics@main/Materials/LearningBlogPics202312221537512.png)

- Q: How to pass arguments to sys_pgaccess()?  
    A: `argint()` and `argaddr()`.

- Q: a0 - a5 register, what data do they store respectively?  
    A: No restrictions.

- Q: Fill kernel temp buf with right bits. How can I get the right bits? And how does it relate to the 3rd argument which is an address?  
    A:  
    - Right bits:  Compare every page PTE with PTE_A, and modify abits depends on the result.
    - Relation: Pass the content in the buffer to the address using copyout.

- Q: `Walk()`?  
    A: Knowing VA, return PTE. Then we could compare it with PTE_A to get to know whether or not it is accessed.

## Lab: traps
### RISC-V assembly
#### Goal
Understand a bit of RISC-V assembly

#### Do
- `make fs.img`
- Read the code in the generated call.asm for the functions `g`, `f` and `main`. Answer questions.

#### Q&As
##### Assembly code
- `addi rd rs1 imm`: rs1+imm -> rd.
   - `addiw`: 'w' stands for "wide", indicating a wider result compared to the regular `addi`.
- `sd rs2, offset(rs1)`: Store double word, from register to memory. rs2 -> memory caled through rs1.
- `ld rd, offset(rs1) `: Load double word, from memory to register. Memory caled through rs1 -> rd.
- `li rs1 imme`: Load an immediate value into register rs1.
- `auipc rs1 imme`:  Immediate being shifted left by 12 bits + pc -> rs1.
- `jalr 1554(ra)`: Jump and Link Register, Jump to the address in ra + 1554.

##### Register
- ra register: Store return address, so it ussually store the address of instruction after the current function call.

##### Standard function prologue and epilogue
Responsible for setting up and tearing down the function's stack frame.  
Stack frame is a block of memory on the call stack that is used for storing local variables, preserving register, and managing the flow of control betweem functions. 

###### Function Prologue
1. Allocate Stack Space: `addi sp, sp, -frame_size`
2. Save Registers (if needed): `sd s0, offset(p)`
3. Set up the Base Pointer (optional) to reference local variables within the stack frame: `addi s0, sp, frame_size`

###### Function Epilogue
1. Restore Registers: `ld s0, offset(sp)`
2. Deallocate Stack Space: `addi sp, sp, frame_size`
3. Return transfer control back to the calling function: `ret`

### Backtrace 
#### Goal
Useful for debugging. List the function calls on the stack above the point where the error occured.

#### Do 
- Use frame pointer in `s0` to walk up the current stack. - Print the saved return address in each stack frame.
- Implement a `backtrace()` in `kernel/printf.c`.
- Insert the call to `backtrace()` in `sys_sleep`, then run `bttest`.
- Add prototype to `kernel/defs.h`.
- Add function `r_fp()` to `kernel/riscv.h` which reads `s0`. It should be called in backtrace.
- Use `PGROUNDDOWN (fp)` to identify the page, if changes, then stop tracing.
- After finish `backtrace`, add it to `panic` in `kernel/printf.c`.

#### Keywords
#backtracing

#### Q&As
- Q: How to fetch the first (current) stack frame pointer?  
   A: Stored in `s0`, read it through `r_fp`.
- Q: Where is the saved return address?  
   A: **Return address** lives at a fixed offset (-8) from the frame pointer of a stack frame, **saved frame pointer** lives at fixed offset (-16) from the frame pointer.
- Q: The last stack frame?  
   A: The saved frame pointer.
- Q: Fetch the content of an address?  
   A: For example, **return address** is the content of an another address, which is the offset (-8) from the frame pointer. So `return_addr_ptr = (uint64*)(stack_frame_pointer - 8)` there is.

### Alarm
#### Goal
A feature that periodically alerts a process based on its' CPU time usage.  

#### Do
##### test0
- [x] Modify Makefile to cause `alarmtest.c` to be compiled as an xv6 user program.
- [x] Put declarations in `user/user.h`: `int sigalarm(int ticks, void (*handler)());`, `int sigreturn(void);`
- [x] To allow `alarmtest` to invoke the sigalarm and sig return system calls, Update `user/usys.pl`, `kernel/syscall.h`, `kernel/syscall.c`.
- [x] Just return 0 in `sys_sigreturn` for now.
- [x] Modify `proc` structure (in kernel/proc.h), add new field **alarm interval** and **handler pointer**, `sys_sigalarm(interval, handler pointer)` should store the **alarm interval** and the **pointer to the handler function** in these new features.
- [x] Add new field **nticks** in `struct proc` to store how many ticks have passed between **last call to a process's alarm handler - current**.
- [x] Initialize `proc` fields in `allocproc()` in `proc.c`.
- [x] Only update a process's **nticks** when there's a timer interrupt: `if(which_dev == 2)...` (if this is a timer interrupt).
- [x] Modify `usertrap()` to make process execute the handler function when the process's alarm interval expires.
- [x] Run `make CPUS=1 qemu-gdb` to do alarmtest. Haven't gotten used to gdb, so just `make CPUS=1 qemu`.

##### test1-test3
- [x] Prevent re-entrant calls by setting a new attribute in proc.
- [x] Save registers into `struct proc` in `usertrap` when interrupted to execute handler.
- [x] Restore them in `sys_sigreturn`.
- [x] Return origin a0 in `sys_sigreturn`.
#### Keywords
#alarm
#### Q&As
- Q: How to run functions by pointer pointing to the function in kernel?  
    A: Modify pc register. (Check the structure trapframe)

- Q: What registers do I need to save and restore to resume the interrupted code correctly?  
    A: Just store the entire trapframe.

- Q: Save to where?  
    A: Proc itself. Allocate a new trapframe structure.

- Q: Why return a0 in sys_sigreturn? Or another question, why return 0 in syscall? What will happen if not returning 0?  
    A: Return 0 is a convention which signifys that the peration completed without errors. In this function, there is no check phase in test3, so a0 could be a0 itself.

## Lab: 
### Copy-On-Write fork()
#### Goal
Improve preformence when doing `fork()`.
Impelementing copy-on-write fork(), which does not copy page at init. Instead it marks PTEs in both parent and child as read-only.   
When either process tries to write one of these COW pages, CPU invokes a page fault, **kernel page-fault handler** detects this case, allocates a new page, copies the original page into the new page, and modifies the relevant PTEs in the faulting process to refer to the new page with marked writeable.

#### Do
- Check macros and definitions for page table flags at the end of `kernel/riscv.h`.

- Use RSW bits in the RISC-V PTE to record whether a PTE is a COW mapping.
   - [x] Add PTE_COW as flag.

- Modify `uvmcopy()`.
   - [x] Map the parent's physical pages into the child instead of allocating; 
   - [x] Clear `PTE_W` in the PTEs of both child and parent for pages that have `PTE_W` set.

- Modify `usertrap()`.
   - [x] Recognize **write page-fault occers on a COW page that was originally writeable**;
   - [x] Allocate a new page with `kalloc()`, copy the old page, install it in the PTE with `PTE_W` set. 
   - [x] Kill process when a COW page fault occurs while there's no free memory.
   - [x] Pages originally read-only should remain read-only (figure out how to recogniaze it), kill process that tries to write such a page.

- Physical page management.
   - [x] Keep each physical page a "reference count" in a fixed-size array of integers. (Index the array with the page's physical address devided by 4096, initiate the array by `kinit()` in `kalloc.c` using highest physical address of any page on the free list)
   - [x] Modify mappages() to increment reference count.
   - [x] Modify `kalloc()` to set a page's reference count to 1 when `kalloc()` allocates it.
   - [x] Modify `fork()` to increment reference count. Actually it's with `mappages()`
   - [x] Modify any func drops a page with decrement reference count. Any func else except `uvunmap()`?
   - [x] Modify `kfree()` to only place a page back on the free list when its reference count is zero.

- [x] Modify `copyout()` to use the same scheme as page faults when it encounters a COW page. (figure out what `copyout()` does, and how does it relate to COW page)

#### Keywords
#COW fork

#### Q&As
- Q: What is PTE? Where does it store?  
    A: PTE (page table entry), stored in a page table (an address), each contains a 44-bit physical page number (PPN) and some flags.  
    It's actually an pointer point to pa in the code.

- Q: What's the relationship between page table and process?  
    A: Each process has its own page table.

- Q: How's the flags of every pte when initiated, 0 or 1?  
    A: 0, they are designated by `mappages()`.

- Q: Where the f is the scause table??  
    A: https://pdos.csail.mit.edu/6.1810/2023/lec/l-usingvm.pdf

- Q: RISC-V distinguishes three kinds of page faults, does xv6 do the same? If so, how? And what kind of page fault is it when write the un-writeable?  
    A: No, xv6 just kills them all. Store/AMO page fault.

- Q: How to fetch the corresponding pte to the current page fault?  
    A: `stval` stores the virtual address that caused the fault, fetch it by `r_stval()`. `walk()` find the pte.

- Q: Can we use same pte in different pagetable?  
    A: No.

- Q: Then how do I find the parents' pte and set it writeable again?  
    A: No, you can't set it writeable again, accept it. Just allocate a new page when write the unwriteable.

- Q: How to install a new page in a existing pte?  
    A: Use `PA2PTE()` to replace the old pa.

- Q: Decide the reference counter array's size?  
    A: Since we are indexing the array with the page's physical address divided by 4096, We could use the ceiling(highest address / 4096) as the size.

- Q: When do we need malloc for array? And when do we not?  
    A: when the array's size is not known. And is known.

- Q: Why `kfree()` old page even when parent may need it?  
    A: Because old page has been set reference count when `uvmcopy()`.