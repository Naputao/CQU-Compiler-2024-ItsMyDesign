# CQU-Compiler-2024-ItsMyDesign

编译原理2024课程设计实现

通过所有测试用例，性能测试得分0.12，rank 1


使用C++语言开发。基于自顶向下分析法生成抽象语法树(AST)，属性制导翻译生成中间代码(IR)，最后使用类似写直达的寄存器分配规则翻译成RISC-V指令。2024-04开始至2024-05结束通过所有测试。

## 待改进方向：

1.在最后一步翻译成RISC-V指令时，将frame_size固定为520，这里其实是存在BUG的，其实需要根据变量的数量n，和要保存的寄存器数量m，开辟栈帧8(n+m)大小的栈帧空间（64位系统）

2.这个修改会导致实验二不通过：修改IR，使其能够准确的标注While语句开始的地方，然后在此基础上，最后一步翻译成RISC-V指令时将寄存器分配规则改为类似cache写回的分配规则。

这里解释一下，举个例子，当while开始时，假设t0寄存器中装的是变量A，当while结束时，t0寄存器中装的是变量B

那么对于下面这样的翻译

```assembly
while:
	addi t1,t0,4
	...
	la s0,A
	sw t0,0(s0)
	la s0,B
	lw t0,0(s0)
	...
while_end:
```

就会导致第一次开始时t1装的是A+4，然而后续的循环t1装的是B+4，导致程序逻辑错误。所以需要从while开始的地方开始记录，记录一下寄存器与变量对应表，然后将程序修改成这个样子：

```assembly
while:
	...
	addi t1,t0,4
	...
	la s0,A
	sw t0,0(s0)
	la s0,B
	lw t0,0(s0)
	...
	la s0,B
	sw t0,0(s0)
	la s0,A
	lw t0,0(s0)
while_end:
```

为什么只有这样才能完成写回？为什么不修改却可以完成写直达？

因为写回策略要考虑缓存一致性，而写直达本身就符合缓存一致性，如果是写直达策略那么就有下面的翻译规则：
```assembly
while:
	...
	la s0,A
	lw t0,0(s0)
	addi t1,t0,4
	la s0,C
	sw t1,0(s0)
	...
	la s0,B
	lw t0,0(s0)
	...
while_end:
```
就不需要考虑上述问题

3.重构src/front/semantic.cpp和include/front/semantic.cpp，删除各个节点里的v属性，统一改用varient

4.修改数组的实现方式，使其能够在栈帧中存在，而不是以全局变量的方式

5.时间长忘了还有哪些不足，欢迎各位补充

