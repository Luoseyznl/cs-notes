# 2. Linux 终端发展史与底层交互架构（TTY/PTY）

在图形化桌面出现之前，与 Unix 系统交互的唯一方式就行基于 shell 提供的命令行界面（Command Line Interface, CLI）。CLI只接收文本输入输出，通常只需要一个简单的哑终端（串口 + 显示器和键盘）就能操作 Unix 系统。

> 哑终端（Dumb Terminal）是一种由键盘和单色显示器组成的硬件。本身不具备运算能力，仅作为物理接口通过串口线（Serial Cable）连接至远端主机。

