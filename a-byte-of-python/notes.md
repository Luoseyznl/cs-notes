# 虚拟环境

虚拟环境的核心理念是隔离。允许每个项目拥有独立的 Python 解释器和独立的第三方库。Ubuntu 24.04 严格执行了 PEP 668 标准（Externally Managed Environment），系统级的 Python 会阻止直接使用 pip install 安装全局第三方库，以保护系统自身的组件（默认自带 Python 3.12+）

```sh
# 1 查看系统级 Python 解释器
python3 -V

# 2 安装用于创建虚拟环境的 venv 模块，以及 pip 包管理器
sudo apt install python3-venv python3-pip

# 3 创建名为 .venv 的虚拟环境（这里的 .venv 是隐藏文件夹名）
python3 -m venv .venv

# 4 激活虚拟环境
source .venv/bin/activate
```

虚拟环境有自己的解释器和包管理器，在 VS Code 里可以选择 `.venv/bin/python` 解释器。

---

# Python 核心基础

## 1 基础语法与控制流

Python 一行一语句，不需要分号 `;`。遇到长代码需要换行时，有以下两种方式：

- 显式换行：使用反斜杠 `\`；
- **隐式换行（强推）**：利用括号 `()`, `[]`, `{}` 换行，更符合直觉且不易因为尾随空格报错。

**字符串**：不可变对象 (Immutable)，单引号 `'` 和双引号 `"` 等价。三引号 `'''` 或 `"""` 支持多行字符串。

> Python 3.6 引入了 f-string 用于更简洁地格式化字符串：`f"{name} is {age} years old"`。

**Python 独有的运算符**：

- `//`：向下取整除法（无论正负数一律向下），即使包含浮点数也是取整后的浮点形式： `9 // 1.8` 结果为 `5.0`。
- `**`：幂运算符，`2 ** 3` 结果为 `8`。
- 链式比较：允许直接写 `1 < x < 10`。（相比 C++ 的 `1 < x && x < 10` 更符合数学直觉）
- 逻辑运算符：直接使用英文 `and` / `or` / `not`，取代 C 系的 `&&` / `||` / `!`。

**Python 独有的控制流特性**：

- 没有 `switch-case`：常规开发中，条件分支一律用 `if...elif...else` 或字典映射 (Dict Mapping) 来替代。
- `for...in` 循环直接遍历序列（如列表、元祖、字符串）。（有点像 C++ 11 的 range-based for）
- `for` 或 `while` 循环后可以接 `else`，当循环正常结束（没有被 break 中断）时才会执行 else 块。

## 2 函数与模块

- **默认参数**注意项：必须置于参数列表末尾。**默认值必须是不可变对象**，不要用 `[]` 或 `{}`，否则会共享同一个对象。
- 参数打包与解包：`*args` 接收任意数量的位置参数（打包为 Tuple）；`**kwargs` 接收任意数量的关键字参数（打包为 Dict）。
- 全局变量作用域：函数内可以读取全局变量，但要修改全局变量必须先声明 `global var_name`。
- 文档字符串 (DocStrings)：函数/类/模块定义下的第一个多行字符串 `'''...'''` 是文档字符串，可通过 `func.__doc__` 或 `help(func)` 读取。

**模块与运行机制**：

- 文件即模块：每个 `.py` 文件都是一个模块。（被其他文件首次导入时会被编译成 .pyc 字节码文件缓存起来，以加速后续加载）
- 模块搜索路径：系统路径 `sys.path` 决定了 Python 解释器去哪些目录寻找模块。
- 内省工具：`dir(obj)` 可以列出对象、模块的所有属性和方法列表。
- 标准执行入口：通过 `if __name__ == '__main__':` 区分当前脚本是“被直接运行”还是“被当作模块导入”。

## 3 常用数据结构

Python 中**一切皆对象**。赋值 `A = B` **仅传递引用**。如果要拷贝需用切片 `A = B[:]` 或 copy 模块。

**可迭代对象 (Iterable)**：凡是能放到 `for...in` 循环里的对象，统称可迭代对象（包含序列、字典、集合、生成器等）。

### 3.1 序列 (Sequences: List, Tuple, String, Range)

共性：支持 `in` 成员测试、正负索引（`-1` 代表末尾）、以及**切片 (Slicing)**。

> 切片语法：`seq[start:stop:step]` (左闭右开)。`shoplist[1:3]`：取索引 1, 2。`shoplist[::-1]`：序列反转。

1. 列表 (List) `[]` **可变 (Mutable)**，常用方法：`.append(item)`, `del list[0]`, `.sort()` (就地排序)。

2. 元组 (Tuple) `()` **不可变 (Immutable)**。常用于函数返回多个值，或保证数据不被意外篡改。

> 单元素元组必须加逗号，即 `(2,)`，否则 `(2)` 会被解析为普通的数学括号运算。

### 3.2 哈希表 (Hash-based: Dict, Set)

共性：基于哈希表，无序（不支持索引），查找速度极快。

1. 字典 (Dictionary) `{}`，Key 是不可变的，通过 `for k, v in dict.items()` 遍历（其实返回的是一个视图对象，并不占内存）。常用方法：`dict[key] = value`, `del dict[key]`, `dict.get(key, default)`。

2. 集合 (Set) `set()` **元素唯一**。常用于去重或进行集合运算（交集 `&`、并集 `|`、差集 `-`）。

### 3.3 推导式与生成器 (Comprehensions & Generators)

**推导式**：用极简的单行代码替代多行的 `for` 循环和 `append` 操作的表达式。

- 列表推导式：`[x**2 for x in range(10) if x % 2 == 0]` 生成一个包含 0, 4, 16, 36 的列表。
- 字典推导式：`{x: x**2 for x in range(5)}` 生成一个字典 `{0: 0, 1: 1, 2: 4, 3: 9, 4: 16}`。
- 生成器表达式：`(x**2 for x in range(10))` 生成一个生成器对象，按需计算每个元素，节省内存。（惰性求值）

**生成器**：**带有 `yield` 关键字的函数会返回一个生成器对象**，每次调用 `next()` 会执行到下一个 `yield` 语句并返回值，直到没有 `yield` 时抛出 `StopIteration` 异常。

```py
# 通过 yield 实现一个生成器（以斐波那契数列为例）
def fibonacci(n):
    a, b = 0, 1
    for _ in range(n):
        yield a          # yield a 后函数“冻结”
        a, b = b, a + b  # 从这里“解冻”并继续执行

# 1 手动唤醒生成器
gen = fibonacci(2)
print(next(gen))  # 输出: 0
print(next(gen))  # 输出: 1
print(next(gen))  # 抛出 StopIteration 异常，因为没有更多的 yield 了

# 2 用 for...in 循环自动处理 next() 和结束异常
for num in fibonacci(5):
    print(num, end=" ")  # 输出: 0 1 1 2 3
```

---

# 面向对象编程

作为一种解释型的动态语言，Python 的面向对象编程（OOP）相对简单自由：

1. 所有的类成员（属性和方法都是）默认都是公开的；
2. 所有方法默认都是“虚方法” (子类可以 overwrite 所有的父类方法，运行时根据对象类型决定所调用的方法)。
3. Python 没有访问控制符，而是通过命名约定来表示成员的访问权限：
   1. 单下划线 `_var` 表示“受保护的”成员（不建议外部访问，但仍可访问）；
   2. 双下划线 `__var` 会触发命名修饰（Name Mangling），使得外部无法直接访问（但仍可通过 `_ClassName__var` 访问）。

Python 严格区分成员是属于“类”还是属于“实例”：

1. 属性分为属于类的**类变量**（所有实例共享）和属于实例的**实例变量**（每个实例独占）。
   1. 类变量：定义在类内、方法之外。所有实例共享同一个类变量的内存。
   2. 实例变量：定义在方法之内。（通常在 `__init__` 构造函数中通过 `self.var_name` 绑定）
2. 方法分为**类方法**（可以由类或实例调用）和**实例方法**（只能由实例调用）。
   1. 类方法：使用 `@classmethod` 装饰器，第一个参数是 `cls`，可以通过类或实例调用。
   2. 实例方法：默认的方法，第一个参数**必须**是 `self`（调用时 Python 会自动传入），只能通过实例调用。

> Python 中还有“魔法方法”（Magic Methods），以双下划线开头和结尾的内置方法：
> - `__init__(self, ...)`：构造函数，在实例化对象时自动调用；
> - `__str__(self)`：定义对象的字符串表示，在 `str(obj)` 和 `print(obj)` 时会调用；
> - `__repr__(self)`：定义对象的**正式**字符串表示，通常用于调试，在交互式环境中直接输入对象时会调用。
> - `__add__(self, other)`：重载加法运算符 `+` 的行为，使得 `obj1 + obj2` 调用 `obj1.__add__(obj2)`。
> - `__len__(self)`：配合内建函数，在 `len(obj)` 时会调用这个方法。

总的来说，Python 的 OOP 有“伪封装”，但支持**继承**和**多态**。子类可以重写所有父类的方法。

> 当子类定义了自己的 `__init__` 时，不会自动调用父类的 `__init__`，需要 `super().__init__(...)` 来初始化父类属性。

此外，装饰器本质是一个高阶函数（接受函数作为参数并返回一个新函数）。常见三大类内置装饰器：

- `@staticmethod`：静态方法，不需要 `self` 或 `cls` 参数。不能访问实例或类的属性。（就是一个普通函数，纯辅助）
- `@classmethod`：类方法，第一个参数是 `cls`。可以访问类属性和方法，但不能访问实例属性。（适合用于工厂方法场景）
- `@property`：属性方法，第一个参数是`self`。将一个方法伪装成属性访问的形式，提供了**只读属性**的能力。

```py
class Student:
    school = "MIT"

    def __init__(self, name, score):
        self.name = name
        self._score = score 

    @classmethod
    def change_school(cls, new_school):
        cls.school = new_school # 1. 类方法用于操作类变量

    @staticmethod
    def is_passing(score):
        return score >= 60      # 2. 静态方法，自娱自乐的辅助函数

    @property
    def score(self):
        return self._score      # 3. 属性方法，使得方法像变量一样被读取: print(stu.score)
        
    @score.setter
    def score(self, value):
        if 0 <= value <= 100:
            self._score = value # 拦截非法赋值
```

---

# 文件 I/O 与异常处理

## 1 文件操作与上下文管理器 (`with`)

Python 提供了内置函数 `open(filename, mode)` 来操作文件，其中：

1. 打开模式 `mode` 可以是：`r` 读取（默认），`w` 覆写，`a` 追加，`b` 二进制模式（如 `wb`, `rb`）；
2. 字符编码：默认是 Unicode（UTF-8），但 Windows 上可能是 GBK，建议 `open(filename, mode, encoding='utf-8')`；

Python 的 `with` 语句（上下文管理器）提供了一种简洁的语法来确保资源正确释放，即使发生异常。（底层原理是通过上下文管理器对象的 `__enter__` 和 `__exit__` 方法来实现）

Python 的文件对象本身也是可迭代的，可以按行按需读取，而不需要一次性加载整个文件到内存（节约内存）。

```py
with open('data.txt', 'r', encoding='utf-8') as f:
    for line in f:
        print(line.strip())  # strip() 去除行末的换行符
```

除了常规的捕获机制，Python 的异常处理多了一个独有的 `else` 分支。完整的结构为：`try...except...else...finally`：

- `try`：放置可能引发异常的代码；
- `except ErrorType as e`：捕获并处理特定类型的异常（可以堆叠）；
- `else`：当 `try` 块没有发生任何异常时执行的代码块（通常用于正常流程的后续处理）；
- `finally`：无论是否发生异常都会执行的代码块（通常用于资源清理，如关闭文件、释放锁等）。
- `raise`：用于主动抛出异常，可以带上自定义的错误信息。

---

# 标准库（进阶）

Python 奉行“自带电池 (Batteries Included)”哲学，绝大多数非业务逻辑的基础功能，标准库里都已经写好了。

## 1 标准库概览

1. 系统与环境交互 (OS & System)

   - `os`：操作系统接口（文件/目录路径拼接、读取环境变量等）。
   - `sys`：Python 解释器交互（如获取命令行参数 `sys.argv`，查看模块搜索路径 `sys.path`）。

2. 数据序列化与文本处理 (Data & Text)

   - `json`：JSON 字符串的编码和解码，前后端数据交互、读写配置文件。
   - `pickle`：Python 原生的对象序列化，将内存对象转化为二进制字节流存储。
   - `re`：正则表达式 (Regular Expression)，用于复杂的字符串匹配、查找和清洗。

3. 高阶数据结构与迭代

   - `collections`：进阶容器库。如 `Counter` (自动计数)、`defaultdict` (带默认值的字典)、`namedtuple` (具名元组)。
   - `itertools`：高效的迭代器工具函数，如 `permutations` (全排列)、`combinations` (组合)。
   - `functools`：函数式编程，如 `@lru_cache` (给函数结果加缓存)、`partial` (冻结函数的部分参数)。

4. 数学运算与时间 (Math & Time)

   - `math`：标准底层数学库（对数、三角函数、常数 $\pi$ 和 $e$）。
   - `random`：伪随机数生成器（生成随机整数、随机抽取、打乱列表顺序等）。
   - `datetime`：处理日期和时间，支持时间加减（`timedelta`）和格式化输出。

## 2 Pythonic 进阶语法

1. 元组解包：`a, b = (1, 2)`，也支持链式解包：`a, *b, c = (1, 2, 3, 4)`。
2. 匿名函数：`lambda 参数: 表达式`，常作为高阶函数（如 sort()）的 key 参数。
3. 断言：`assert condition, "Error message"`，防御性编程。

## 3 手写装饰器

外层函数接收原函数，内层函数接收原参数，内层执行原函数并返回。推荐加上 @wraps(f) 保留原函数的文档和名字。

```py
from functools import wraps
import time

# 定义一个计算耗时的装饰器
def timer_decorator(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        start_time = time.time()
        
        result = func(*args, **kwargs) # 执行原函数
        
        end_time = time.time()
        print(f"[{func.__name__}] 耗时: {end_time - start_time:.4f} 秒")
        return result                  # 返回原函数的结果
    return wrapper

@timer_decorator
def heavy_computation(n):
    return sum(i * i for i in range(n))

heavy_computation(1000000)
```

---

# 最佳实践与避坑指南

1. 字典安全访问：优先使用 `dict.get(key, default)` 来读取。
   因为当 Key 不存在时 `get` 会返回 `None` 或指定的默认值，而直接 `dict[key]` 会抛出 `KeyError` 异常。

2. 比较操作：`==` 是比较的数值，而 `is` 是比较的对象身份（内存地址）。
   在判断单例对象时（如 `None`, `True`, `False`）应该使用 `is`，而在比较数值或字符串等不可变对象时使用 `==`。

3. 当遍历时需要使用索引，可以优雅地解包出“索引”和“元素”：`for index, item in enumerate(['a', 'b', 'c'])`。

4. 字符串拼接：字符串是不可变对象，频繁地 `+` 拼接会产生大量临时对象，建议使用 `"".join(str_list)` 一次性拼接成型。

5. 空容器本身的布尔值就是 `False`，建议判空时可以直接 `if not my_list:`（如果容器为空）。
    注意：如果在业务逻辑中，空容器 `""`/`[]`/`()`/`{}`、数字 `0` 为有效值，就不能通过 `if not my_var:` 来判断是否存在（因为它们的布尔值为 `False`）。此时可以使用 `if my_var is None:` 来判断是否为 `None`。

6. 判断一个变量是否为某种类型时，建议用 `isinstance(obj, dict)`（支持继承），而不是 `type(obj) == dict`。

7. 赋值与深浅拷贝：Python 中 `=` 永远是传递引用（贴标签），不存在值拷贝（有时候不可变对象的赋值可能看起来像值拷贝）。

   1. 浅拷贝 (Shallow Copy)：`a = b.copy()` 或 `a = b[:]`。只拷贝外层容器，容器内部的嵌套对象仍然是**共享引用**。
   2. 深拷贝 (Deep Copy)：`import copy; a = copy.deepcopy(b)`。递归地拷贝所有层级的对象，容器间完全独立。
