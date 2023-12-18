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

### Run GDB
1. In first terminal, Cd xv6-labs-2023, run `make qemu-gdb`;
2. Open another terminal, run `gdb-multiarch`, wait for response;
3. Back to the first terminal, run `make qemu-gdb`;
4. Back to another terminal, run `target remote localhost:26000` and run `file kernel/kernel`, set break point.
- It is frustrating indeed. I believe the reason to blame is in some code in the qemu, the port is set to `25000`.
    

#### Say you wanted to break every time the kernel enters the function syscall from kernel/syscall.c
1. Inside the gdb prompt: file kernel/kernel (this is a binary that has all kernel code)
2. Inside the gdb prompt: b syscall
3. Hit c. At this point you will start hitting the breakpoint above
4. Keep hitting c to see where the kernell hits the syscall function. You will how the output in the first window is progressing.

#### Exception|Interrupt registers
- Scause register. Storing the cause(name) of the exception.
- Sepc register. Storing the address of the instruction that caused the exception.
- Stval register. Additional information, ussually the comment about the instruction that caused the exception.

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

#### Q&As
- Q: Which other xv6 system call(s) could be made faster using this shared page? Explain how.  
    A:

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