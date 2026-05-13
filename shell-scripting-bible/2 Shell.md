# Linux 终端发展史与底层交互机制（TTY/PTY）

在图形化桌面出现之前，用户与 Unix 系统交互的唯一方式就是基于 shell 提供的**命令行界面（Command Line Interface, CLI）**。CLI 只处理纯文本输入输出，用户通常只需要一个简单的**哑终端**（Dumb Terminal）就能操作 Unix 远端主机。

> 哑终端是早期一种由键盘、单色显示器和串口通信模块（如 RS-232）组成的硬件设备。它本身不具备计算、存储或复杂处理能力，所以又被称为“玻璃电传打字机”（Glass Teletype）。

现代 Linux 发行版都配备了某种图形化桌面环境，但底层仍保留了两种进入 CLI 的方式：

1. **控制台终端（虚拟控制台）**：运行在 Linux 内核中的原生终端会话（无需物理终端设备），可以通过 `Ctrl + Alt + F1` 到 `F6` 进行切换”。
2. **图形化的终端仿真器**：如 GNOME Terminal、Konsole、xterm 等，它们作为普通的窗口软件运行在图形桌面环境（GUI）中，功能更为丰富。

## 1. 控制台终端

在 Linux 系统启动时，内核通常会在内存中自动创建 5~6 个虚拟控制台。每个控制台都提供一个独立的登录会话，用户可以通过 `Ctrl + Alt + F1`~`F6` 在不同的控制台之间切换。登录成功后，每个控制台都会运行一个独立的 Shell 进程（默认通常为 bash）。

> 旧版 X11 图形化桌面环境会在启动时寻找第一个空闲的控制台（通常是 `tty7`），而现代的 GNOME 和 KDE 则直接在 `tty1` 上运行图形化的登录界面（Login Screen）；登录成功后，为了安全和会话隔离，系统会在 `tty2` 启动一个全新的图形桌面会话（User Session）。

虚拟控制台采用全屏方式显示登录界面。界面第一行一般会显示 tty 号（如 `tty3`），这对应了 Linux 内核为该控制台分配的底层设备文件名（如 `/dev/tty3`）。

> `/dev` 目录里的 `tty` 和 `tty0` 是干嘛的？
> `/dev/tty0` 固定映射至当前处于活动状态的控制台（前台控制台），用于系统内核和守护服务向“当前用户正在看着的屏幕”发送广播或紧急消息。
> `/dev/tty` 固定映射至当前运行进程所在的终端（无论是底层虚拟控制台还是桌面终端仿真器）。用于确保程序发出的消息，能准确回显在启动该程序的对应的终端。

一旦登录完成，你可以在不中断当前会话的同时切换到其他虚拟控制台。
此外，尽管是纯文本环境，依然可以通过 `setterm` 命令直接向底层发送控制码，自定义控制台的背景色、前景色等外观，选项如下：

![setterm](shell-scripting-bible/images/2. Shell/setterm.png)

## 2. 图形化的终端仿真器

> 不同的桌面环境通常会自带其专属的图形化终端仿真器。主流的有：GNOME Terminal、Konsole、xterm，以及第三方工具 Terminator 等。

为了在软件层面模拟终端，Linux 内核提供**伪终端（Pseudo-Terminal, PTY）**机制，`PTY` 是一对在内核中通过管道连接的虚拟字符设备：
- **主设备（Master）**：通常是 `/dev/ptmx`（x 代表 multiplexer），由终端仿真器软件（如 GNOME Terminal）或服务端进程（如 SSH 守护进程）打开、持有并读写。
- **从设备（Slave）**：通常挂载在 `/dev/pts/` 目录下（如 `/dev/pts/0`）。它会被内核分配给 Shell 进程（如 bash）。在 Shell 看来，`/dev/pts/0` 就和原生终端（如 `/dev/tty3`）逻辑等效，负责处理用户的输入与输出。

![terminator](<images/2. Shell/terminator.png>)

Terminator 等高级终端仿真器在基础功能之上提供了**分屏（Split Screen）**功能，允许用户将单一窗口水平或垂直分割成多个独立的终端区域，每个区域均可运行独立的 Shell 会话，主要特性有：

1. **原生网格切分（Tiling）**：无需借助 `tmux` 或 `screen` 等复杂的命令行复用工具，支持通过快捷键（如 Ctrl+Shift+O/E）在单个物理窗口内进行无限层级的横向/纵向终端切分。
2. **命令广播机制（Broadcast）**：支持将当前焦点窗口的键盘输入，同步广播到同一个网格（或全部分屏）内的所有终端中。
3. **高自由度布局留存**：支持将复杂的分屏布局和对应的初始化命令保存为 Profile 配置文件。

## 3. Shell 运行机制

在用户登录虚拟控制台终端或图形化终端仿真器后，系统会启动默认的**交互式 Shell 进程**（可以查看“用户 ID 配置表” `/etc/passwd`，通常默认是 `/bin/bash`）。此外还有系统级 Shell（`/bin/sh`，通常是 dash 的符号链接），由系统自动化脚本默认使用。
# Linux 终端发展史与底层交互机制（TTY/PTY）

在图形化桌面出现之前，用户与 Unix 系统交互的唯一方式就是基于 shell 提供的**命令行界面（Command Line Interface, CLI）**。CLI 只处理纯文本输入输出，用户通常只需要一个简单的**哑终端**（Dumb Terminal）就能操作 Unix 远端主机。

> 哑终端是早期一种由键盘、单色显示器和串口通信模块（如 RS-232）组成的硬件设备。它本身不具备计算、存储或复杂处理能力，所以又被称为“玻璃电传打字机”（Glass Teletype）。

现代 Linux 发行版都配备了某种图形化桌面环境，但底层仍保留了两种进入 CLI 的方式：

1. **控制台终端（虚拟控制台）**：运行在 Linux 内核中的原生终端会话（无需物理终端设备），可以通过 `Ctrl + Alt + F1` 到 `F6` 进行切换”。
2. **图形化的终端仿真器**：如 GNOME Terminal、Konsole、xterm 等，它们作为普通的窗口软件运行在图形桌面环境（GUI）中，功能更为丰富。

## 1. 控制台终端

在 Linux 系统启动时，内核通常会在内存中自动创建 5~6 个虚拟控制台。每个控制台都提供一个独立的登录会话，用户可以通过 `Ctrl + Alt + F1`~`F6` 在不同的控制台之间切换。登录成功后，每个控制台都会运行一个独立的 Shell 进程（默认通常为 bash）。

> 旧版 X11 图形化桌面环境会在启动时寻找第一个空闲的控制台（通常是 `tty7`），而现代的 GNOME 和 KDE 则直接在 `tty1` 上运行图形化的登录界面（Login Screen）；登录成功后，为了安全和会话隔离，系统会在 `tty2` 启动一个全新的图形桌面会话（User Session）。

虚拟控制台采用全屏方式显示登录界面。界面第一行一般会显示 tty 号（如 `tty3`），这对应了 Linux 内核为该控制台分配的底层设备文件名（如 `/dev/tty3`）。

> `/dev` 目录里的 `tty` 和 `tty0` 是干嘛的？
> `/dev/tty0` 固定映射至当前处于活动状态的控制台（前台控制台），用于系统内核和守护服务向“当前用户正在看着的屏幕”发送广播或紧急消息。
> `/dev/tty` 固定映射至当前运行进程所在的终端（无论是底层虚拟控制台还是桌面终端仿真器）。用于确保程序发出的消息，能准确回显在启动该程序的对应的终端。

一旦登录完成，你可以在不中断当前会话的同时切换到其他虚拟控制台。
此外，尽管是纯文本环境，依然可以通过 `setterm` 命令直接向底层发送控制码，自定义控制台的背景色、前景色等外观，选项如下：

![setterm](shell-scripting-bible/images/2. Shell/setterm.png)

## 2. 图形化的终端仿真器

> 不同的桌面环境通常会自带其专属的图形化终端仿真器。主流的有：GNOME Terminal、Konsole、xterm，以及第三方工具 Terminator 等。

为了在软件层面模拟终端，Linux 内核提供**伪终端（Pseudo-Terminal, PTY）**机制，`PTY` 是一对在内核中通过管道连接的虚拟字符设备：
- **主设备（Master）**：通常是 `/dev/ptmx`（x 代表 multiplexer），由终端仿真器软件（如 GNOME Terminal）或服务端进程（如 SSH 守护进程）打开、持有并读写。
- **从设备（Slave）**：通常挂载在 `/dev/pts/` 目录下（如 `/dev/pts/0`）。它会被内核分配给 Shell 进程（如 bash）。在 Shell 看来，`/dev/pts/0` 就和原生终端（如 `/dev/tty3`）逻辑等效，负责处理用户的输入与输出。

![terminator](<images/2. Shell/terminator.png>)

Terminator 等高级终端仿真器在基础功能之上提供了**分屏（Split Screen）**功能，允许用户将单一窗口水平或垂直分割成多个独立的终端区域，每个区域均可运行独立的 Shell 会话，主要特性有：

1. **原生网格切分（Tiling）**：无需借助 `tmux` 或 `screen` 等复杂的命令行复用工具，支持通过快捷键（如 Ctrl+Shift+O/E）在单个物理窗口内进行无限层级的横向/纵向终端切分。
2. **命令广播机制（Broadcast）**：支持将当前焦点窗口的键盘输入，同步广播到同一个网格（或全部分屏）内的所有终端中。
3. **高自由度布局留存**：支持将复杂的分屏布局和对应的初始化命令保存为 Profile 配置文件。

## 3. Shell 运行机制

在用户登录虚拟控制台终端或图形化终端仿真器后，系统会启动默认的**交互式 Shell 进程**（可以查看“用户 ID 配置表” `/etc/passwd`，通常默认是 `/bin/bash`）。此外还有系统级 Shell（`/bin/sh`，通常是 dash 的符号链接），由系统自动化脚本默认使用。

登录启动的默认交互 Shell 是父进程。在其内部执行外部命令时，会衍生出子进程；可以借助 `ps -f` 和 `ps --forest` 查看进程树：

```sh
$ bash
$ ps -f
UID          PID    PPID  C STIME TTY          TIME CMD
gew       278360  278347  0 21:15 pts/5    00:00:00 /usr/bin/zsh
gew       278452  278360  0 21:15 pts/5    00:00:00 bash
gew       278563  278452  0 21:15 pts/5    00:00:00 ps -f
$ ps --forest
    PID TTY          TIME CMD
 278360 pts/5    00:00:00 zsh
 278452 pts/5    00:00:00  \_ bash
 278666 pts/5    00:00:00      \_ ps
$ exit  # exit 可以有序地关闭当前 Shell 进程，并返回父进程（如果有）
```

**手动启动 bash**或执行**进程列表**、**后台作业**、**协程**时会衍生出子 Shell。子 Shell 有自己的变量空间（包括从父 Shell 继承的局部变量和环境变量）、工作目录、文件描述符（包括 stdin、stdout、stderr）等，与父 Shell 相互独立，互不干扰。

> 子进程指由父进程通过 `fork()` 系统调用衍生出来的进程。子 Shell 是一种特殊的子进程，它是父 Shell 进程的克隆副本。**进程间可通过发送信号（signaling）通信**，比如 `kill -SIGUSR1 <PID>`，但通常不建议在 Shell 脚本中使用信号进行进程间通信，因为它比较复杂且不够可靠。

1. 进程列表 `(cmd1; cmd2)`：在一个独立的子 Shell 中依次执行命令。

> 作为对比，命令分组 `{ cmd1; cmd2;}` 直接在当前 Shell 中执行，不产生子进程。注意 `{ ` 之后有空格，末尾命令需加分号。

```sh
$ { echo $BASH_SUBSHELL; (echo $BASH_SUBSHELL); } # 查看子 Shell 的层级
0
1
```

1. 后台作业 `command &` 在命令或进程列表末尾添加 `&`，会在后台子 Shell 中异步执行，并立即返回父 Shell 提示符；
```sh
$ (sleep 10; echo "Done") &
[1] 300528  # 1 是作业号，300528 是子 Shell 的 PID，可以用 `jobs` 命令查看当前 Shell 的后台作业列表
```

1. 协程 `coproc command` 在后台子 Shell 中执行，并创建匿名管道连接父子 Shell 的输入/输出流。

```sh
$ coproc MYCAT { cat; }
$ echo "Hello" >&"${MYCAT[1]}"      # 子 Shell 的输入流连接到 MYCAT[1]
$ read -r response <&"${MYCAT[0]}"  # 子 Shell 的输出流连接到 MYCAT[0]
$ echo "$response"
Hello
```

Linux 命令分为**内置命令**（Built-in Command）和**外部命令**。可以通过 `type` 查看命令类型。`which` 查找外部命令的路径。

- 内置命令和 Shell 编译为一体，直接在当前 Shell 进程内执行，无额外开销。
- 外部命令则是独立的可执行文件（通常位于 `/bin`、`/usr/bin` 等），通过衍生（Fork）和替换（Exec）机制在**子进程**中执行。

Shell 采用内存与磁盘分离机制来管理历史记录：

1. 暂存：输入的命令立即存入当前 Shell 所占用的内存中。我们可以用 `history` 命令查看当前会话的历史记录。
2. 同步：在 Shell 会话正常关闭时，内存中的新记录会同步到磁盘文件 `~/.bash_history` 中。

另一个有用的内置命令是 `alias`，可以为常用命令创建简短的别名。可以用 `alias -p` 查看当前发行版预设的别名列表。

事件指示符（Event Designators）：
- `!!`：执行上一条
- `!n`：执行历史第 n 条
- `!str`：执行最近一条以 str 开头的命令
- `!?str`：执行最近一条包含 str 的命令
- `!$`：引用上一条最后一个参数，比如 `cd !$`。
- `^old^new`：替换修正。将上一条中的 old 换成 new 并执行。

此外还有 `Ctrl + R` 反向搜索历史记录，输入关键词后会显示匹配的命令，按 `Enter` 执行，或 `Ctrl + G` 退出搜索。

## 4. 环境变量

Bash Shell 通过环境变量来存储与会话和工作环境有关的配置信息（因此叫环境变量）。环境变量分为“全局变量”和“局部变量”两类：
全局变量对所有 Shell 进程可见（包括子 Shell），局部变量仅在当前 Shell 进程内可见。可以通过 `export` 命令将局部提升为全局。

变量查看命令：

- `env` 或 `printenv`：仅显示全局环境变量。

- `set`：显示当前 Shell 的所有变量（包含全局、局部及自定义函数）。

- `declare -p`：精确查看变量的属性与底层定义。

比如 `PATH` 是 Linux 中最重要的环境变量，它决定了 Shell 去哪里寻找外部命令。`PATH` 变量中定义的目录路径以冒号 `:` 分隔，Shell 会按照顺序在这些目录中查找用户输入的命令，找不到则返回“command not found”错误。
```sh
export PATH=$PATH:/home/XXX/my_bin
```

在命令行直接定义的环境变量随 Shell 进程关闭而销毁。要实现持久化，需将其写入**配置文件**。根据 Shell 启动方式的不同，它会读取不同的配置文件：

1. **登录 Shell**（比如 SSH 登录、虚拟控制台登录或 `su - username` 切换用户登录）

首先加载执行**全局配置文件** `/etc/profile`，该文件会自动遍历执行 `/etc/profile.d/*.sh`。
然后按顺序寻找**用户配置文件** `~/.bash_profile`、`~/.bash_login`、`~/.profile`。找到第一个存在的即执行。

2. **非登录 Shell**（比如在图形化终端仿真器中打开一个新窗口，或在当前 Shell 中直接输入 `bash` 启动子 Shell）

首先加载执行**全局配置文件** `/etc/bash.bashrc`，然后寻找**用户配置文件** `~/.bashrc`。如果存在则执行。

3. **非交互式 Shell**（在系统执行 Shell 脚本时，比如 `./script.sh`）

这种方式**不读取任何上述配置文件**。它仅继承父 Shell 已经 export 的全局环境变量。除非 `export BASH_ENV=/path/to/my_env_file`，那么脚本会在运行前先执行 `source $BASH_ENV`，现代开发极少使用。

根据上述加载机制，日常配置应遵循以下严格规范：
- 个人配置修改写入 `~/.bashrc`（Zsh 用户为 ~/.zshrc），`~/.bash_profile` 一般会包含一行 `source ~/.bashrc` 来确保登录 Shell 也能加载个人配置。
- 全局配置修改在 `/etc/profile.d/` 目录下新建自定义的 .sh 脚本。严禁直接修改 `/etc/profile` 主文件。


$ ps --forest
    PID TTY          TIME CMD
 278360 pts/5    00:00:00 zsh
 278452 pts/5    00:00:00  \_ bash
 278666 pts/5    00:00:00      \_ ps
$ exit  # exit 可以有序地关闭当前 Shell 进程，并返回父进程（如果有）
```

**手动启动 bash**或执行**进程列表**、**后台作业**、**协程**时会衍生出子 Shell。子 Shell 有自己的变量空间（包括从父 Shell 继承的局部变量和环境变量）、工作目录、文件描述符（包括 stdin、stdout、stderr）等，与父 Shell 相互独立，互不干扰。

> 子进程指由父进程通过 `fork()` 系统调用衍生出来的进程。子 Shell 是一种特殊的子进程，它是父 Shell 进程的克隆副本。**进程间可通过发送信号（signaling）通信**，比如 `kill -SIGUSR1 <PID>`，但通常不建议在 Shell 脚本中使用信号进行进程间通信，因为它比较复杂且不够可靠。

1. 进程列表 `(cmd1; cmd2)`：在一个独立的子 Shell 中依次执行命令。

> 作为对比，命令分组 `{ cmd1; cmd2;}` 直接在当前 Shell 中执行，不产生子进程。注意 `{ ` 之后有空格，末尾命令需加分号。

```sh
$ { echo $BASH_SUBSHELL; (echo $BASH_SUBSHELL); } # 查看子 Shell 的层级
0
1
```

1. 后台作业 `command &` 在命令或进程列表末尾添加 `&`，会在后台子 Shell 中异步执行，并立即返回父 Shell 提示符；
```sh
$ (sleep 10; echo "Done") &
[1] 300528  # 1 是作业号，300528 是子 Shell 的 PID，可以用 `jobs` 命令查看当前 Shell 的后台作业列表
```

1. 协程 `coproc command` 在后台子 Shell 中执行，并创建匿名管道连接父子 Shell 的输入/输出流。

```sh
$ coproc MYCAT { cat; }
$ echo "Hello" >&"${MYCAT[1]}"      # 子 Shell 的输入流连接到 MYCAT[1]
$ read -r response <&"${MYCAT[0]}"  # 子 Shell 的输出流连接到 MYCAT[0]
$ echo "$response"
Hello
```

Linux 命令分为**内置命令**（Built-in Command）和**外部命令**。可以通过 `type` 查看命令类型。`which` 查找外部命令的路径。

- 内置命令和 Shell 编译为一体，直接在当前 Shell 进程内执行，无额外开销。
- 外部命令则是独立的可执行文件（通常位于 `/bin`、`/usr/bin` 等），通过衍生（Fork）和替换（Exec）机制在**子进程**中执行。

Shell 采用内存与磁盘分离机制来管理历史记录：

1. 暂存：输入的命令立即存入当前 Shell 所占用的内存中。我们可以用 `history` 命令查看当前会话的历史记录。
2. 同步：在 Shell 会话正常关闭时，内存中的新记录会同步到磁盘文件 `~/.bash_history` 中。

另一个有用的内置命令是 `alias`，可以为常用命令创建简短的别名。可以用 `alias -p` 查看当前发行版预设的别名列表。

事件指示符（Event Designators）：
- `!!`：执行上一条
- `!n`：执行历史第 n 条
- `!str`：执行最近一条以 str 开头的命令
- `!?str`：执行最近一条包含 str 的命令
- `!$`：引用上一条最后一个参数，比如 `cd !$`。
- `^old^new`：替换修正。将上一条中的 old 换成 new 并执行。

此外还有 `Ctrl + R` 反向搜索历史记录，输入关键词后会显示匹配的命令，按 `Enter` 执行，或 `Ctrl + G` 退出搜索。

## 4. 环境变量

Bash Shell 通过环境变量来存储与会话和工作环境有关的配置信息（因此叫环境变量）。环境变量分为“全局变量”和“局部变量”两类：
全局变量对所有 Shell 进程可见（包括子 Shell），局部变量仅在当前 Shell 进程内可见。可以通过 `export` 命令将局部提升为全局。

变量查看命令：

- `env` 或 `printenv`：仅显示全局环境变量。

- `set`：显示当前 Shell 的所有变量（包含全局、局部及自定义函数）。

- `declare -p`：精确查看变量的属性与底层定义。

比如 `PATH` 是 Linux 中最重要的环境变量，它决定了 Shell 去哪里寻找外部命令。`PATH` 变量中定义的目录路径以冒号 `:` 分隔，Shell 会按照顺序在这些目录中查找用户输入的命令，找不到则返回“command not found”错误。
```sh
export PATH=$PATH:/home/XXX/my_bin
```

在命令行直接定义的环境变量随 Shell 进程关闭而销毁。要实现持久化，需将其写入**配置文件**。根据 Shell 启动方式的不同，它会读取不同的配置文件：

1. **登录 Shell**（比如 SSH 登录、虚拟控制台登录或 `su - username` 切换用户登录）

首先加载执行**全局配置文件** `/etc/profile`，该文件会自动遍历执行 `/etc/profile.d/*.sh`。
然后按顺序寻找**用户配置文件** `~/.bash_profile`、`~/.bash_login`、`~/.profile`。找到第一个存在的即执行。

2. **非登录 Shell**（比如在图形化终端仿真器中打开一个新窗口，或在当前 Shell 中直接输入 `bash` 启动子 Shell）

首先加载执行**全局配置文件** `/etc/bash.bashrc`，然后寻找**用户配置文件** `~/.bashrc`。如果存在则执行。

3. **非交互式 Shell**（在系统执行 Shell 脚本时，比如 `./script.sh`）

这种方式**不读取任何上述配置文件**。它仅继承父 Shell 已经 export 的全局环境变量。除非 `export BASH_ENV=/path/to/my_env_file`，那么脚本会在运行前先执行 `source $BASH_ENV`，现代开发极少使用。

根据上述加载机制，日常配置应遵循以下严格规范：
- 个人配置修改写入 `~/.bashrc`（Zsh 用户为 ~/.zshrc），`~/.bash_profile` 一般会包含一行 `source ~/.bashrc` 来确保登录 Shell 也能加载个人配置。
- 全局配置修改在 `/etc/profile.d/` 目录下新建自定义的 .sh 脚本。严禁直接修改 `/etc/profile` 主文件。

