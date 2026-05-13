# Linux 核心架构

我们通常所说的 Linux，严格意义上是 Linux 内核 (Kernel) 与 GNU 操作系统工具 的结合体，统称为 GNU/Linux。

一个完整的 Linux 操作系统通常如下图所示，其中 Application Software 也包括图形化桌面环境 (如 GNOME、KDE Plasma)。

![computer_system](../images/linkers-and-loaders/computer_system.png)

## 1 Linux Kernel 内核

> Linus Torvalds 于 1991 年发布了 Linux 内核的 0.01 版。内核位于计算机硬件与运行库之间，向下制定 Hardware Specification (硬件规范)，向上提供 System Call Interface (系统调用接口)。

内核主要有以下四个核心功能：

- **内存管理**：不仅管理**物理内存 (RAM)**，还负责**虚拟内存**的分配和管理。
- **进程管理**：Linux 将运行中的程序视为**进程 (Process)**，内核负责创建、调度和终止这些进程。
- **设备管理**：内核通过**设备驱动程序 (Driver)** 与计算机的各种硬件设备进行通信和控制。
- **文件系统管理**：通过**虚拟文件系统 (VFS)** 接口统一管理各种文件系统。

1. 内存管理 (Memory Management)

   - 内存管理单元 (MMU)：将物理内存划分为页 (Page)，并通过页表 (Page Table) 实现虚拟地址到物理地址的映射；
   - 缺页异常 (Page Fault)：访问未映射的虚拟地址时，MMU 触发缺页异常，内核会将所需页从磁盘加载回 RAM；
   - 交换区 (Swap Space)：当物理内存不足时，内核会将不常用的数据从 RAM 移动到交换区。

2. 进程管理 (Process Management)

    内核启动后，创建第一个进程 (PID 1)，由其负责拉起和管理所有随开机启动的系统级后台服务。这里经历了重要的历史演进：

    - SysVinit (经典)：通过 `/etc/init.d/` 下的 Shell 脚本串行启动。通过 Runlevel 0-6 来定义系统启动状态。
    - systemd (现代)：通过 `/etc/systemd/system/` 下的 Unit 文件实现服务并行启动。统一使用 `systemctl` 控制。

    > Linux 的思想是“万物皆文件，余者皆进程。” (Everything is a file, the rest are processes.)

3. 设备管理 (Device Management)

    Linux 将硬件设备也视为节点文件 (通常位于 `/dev`)，与设备驱动程序的交互通过读写这些文件来完成。

    - 字符设备 (Character)：每次只能处理一个字符的设备 (如终端、键盘、鼠标)。
    - 块设备 (Block)：每次处理一个数据块的设备 (如硬盘、USB 设备)。
    - 网络设备 (Network)：采用数据包发送和接收数据的设备 (如网卡、回环设备 loopback)。

    > `/dev` 目录下的设备文件通过主设备号 (Major) 和次设备号 (Minor) 来标识设备类型和具体设备。

4. 文件系统管理 (File System Management)

    Linux 有极强的文件系统兼容性。内核通过统一的虚拟文件系统 (VFS) 接口来管理不同类型的文件系统。

    <div align="center">
        <img src="../images/shell-scripting-bible/fhs&distribution-0.png" alt="Arch" width="80%">
    </div>

## 2 GNU Tools

Linux 内核提供底层系统 (内核空间)，GNU (GNU is Not Unix) 提供用户空间下的工具和库，两者共同构成 Linux 操作系统。

1. GNU 工具链 (GNU Toolchain)

    包括编译器 (GCC)、调试器 (GDB)、构建工具 (Make)、命令行工具 (coreutils)等，是开发 Linux 应用程序的基础。

    - 文件处理：`ls, cp, mv, rm, find` ...
    - 文本处理：`cat, grep, sed, awk` ...
    - 系统监控：`ps, top, free, df, kill` ...

2. Shell

    Shell 是用户与操作系统交互的命令行界面。常见有 Bash, Zsh, Fish 等。Shell 既是一个命令解释器，也是一种脚本语言。

3. Linux 桌面环境

    包括 GNOME, KDE Plasma, Xfce 等，本质是运行在 Linux 上的应用程序，采用“底层显示协议 + 上层窗口管理器”的架构。

    - 显示协议：X11 (传统)和 Wayland (现代)负责处理输入输出事件和图形渲染。
    - 窗口管理器：如 Mutter (GNOME)和 KWin (KDE)负责管理窗口的布局、装饰和交互。

    > X11 (X.org) 存在性能和安全问题，Wayland 直接由应用程序与显示服务器通信，提升了性能和安全性。

    <div align="center">
        <img src="../images/shell-scripting-bible/fhs&distribution-1.png" alt="Arch" width="80%">
    </div>

---

# Linux 终端与配置

早期用户与 Unix 系统交互只需要一个由键盘和单色显示器组成的哑终端 (Dumb Terminal)，即只负责输入输出，无计算能力。
Linux 保留了这种传统的终端概念，提供了虚拟终端 (Virtual Terminal) 和伪终端 (Pseudo Terminal) 两种进入 CLI 的方式。

但不管哪种终端，在 Linux 内核眼里都是一个字符设备 (Character Device)，设备文件都位于 `/dev` 目录下。

## 1 虚拟终端 (Virtual Terminal)

这是 Linux 内核原生的终端机制，直接与底层的显卡和键盘打交道。

- `/dev/ttyN` (1-6) 切换到不同的虚拟终端，每个终端都独立运行一个登录会话。通常在 tty1 运行图形桌面。
- `/dev/tty0` (全局广播) 映射到当前屏幕正在显示的那个控制台。并且内核崩溃信息会直接输出到 tty0。
- `/dev/tty` (当前终端) 这是一个动态映射针，永远指向当前进程所在的终端。

## 2 伪终端 (Pseudo Terminal)

伪终端是一种软件模拟的终端，由主设备 (`/dev/ptmx`) 和从设备 (`/dev/pts/N`) 组成。
这是为了在图形桌面 (如 Terminator) 或远程连接 (如 SSH) 环境下模拟终端交互而设计。

- `/dev/ptmx` (多路复用器入口)：每次终端软件打开它时，内核会在内存中动态生成一对主从设备，并将主设备描述符返回给软件。
- `/dev/pts/N` (伪终端从设备)：内核在 `/dev/pts/` 下自动生成的文件。Shell 进程通过它与外部通信，与原生虚拟终端等效。

## 3 Shell 进程

1. 父进程与子进程：在 Linux 中，除了系统初始进程 `systemd`，其余进程都是通过 `fork()` 由父进程克隆出来的**子进程**。
2. 父 Shell 与子 Shell：子 Shell 会继承父 Shell 的全局环境变量和工作目录，但两者**相互独立**。

子 Shell 触发场景：

- **执行 Shell 脚本 (`./script.sh`)**：克隆/开启一个非交互式的子 Shell 来逐行运行代码，不会污染当前终端。
- **进程列表 (`(cmd1; cmd2)`)**：克隆一个子 Shell 执行命令，父 Shell 阻塞等待。
- **后台作业 (`cmd &`)**：克隆一个子 Shell 后台执行命令，父 Shell 立即返回。
- **管道符 (`cmd1 | cmd2`)**：依次克隆两个子 Shell，分别执行 `cmd1` 和 `cmd2`，父 Shell 负责连接它们的输入输出。
- **命令替换 (`$(cmd)`)**：克隆一个子 Shell 执行命令后，将输出结果返回给父 Shell 进行替换。

> 如果 Shell 脚本第一行是 `#!/bin/bash`，则克隆子 Shell 通过 `exec()` 替换为指定的 Shell 解释器来执行脚本内容。

内置命令直接修改当前 Shell 进程的状态；而外部命令则在子 Shell 中执行，不影响父 Shell 环境。(用 `type` 看)

```sh
$ type cd
cd is a shell builtin

$ type ps
ps is /usr/bin/ps          # 外部命令
```

## 4 环境变量与配置文件

变量分为仅当前 Shell 可见的局部变量，以及所有子 Shell 可见的全局变量（需通过 `export` 声明）。

为了让环境配置永久生效，需将其写入配置文件。Shell 会根据启动方式的不同，加载不同的文件：

1. 登录 Shell（如 SSH 登录、Ctrl+Alt+F2 切换控制台）：
   - 1. 全局配置：`/etc/profile`（自动加载 `/etc/profile.d/*.sh`）
   - 2. 用户配置：`~/.bash_profile`（优先）`~/.bash_login`（次选）、`~/.profile`（兼容性考虑）

2. 非登录 Shell：
   - 1. 全局配置：`/etc/bash.bashrc`
   - 2. 用户配置：仅 `~/.bashrc`

> 个人的环境变量和别名（alias）建议统一写入 `~/.bashrc` 中。全局配置则建议在 `/etc/profile.d/` 目录下新建独立脚本。
