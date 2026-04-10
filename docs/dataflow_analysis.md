# 数据流分析相关优化

以下内容参考软件分析课程与实验讲义

[Static Program Analysis | Tai-e (pascal-lab.net)](https://tai-e.pascal-lab.net/lectures.html)

可简要阅读实验 1 2 3 获得更多信息

本实验需要完成以下三项优化:
- **全局常量传播优化**
- **全局公共子表达式消除优化**（依赖可用表达式分析与复制传播）
- **全局无用代码消除优化**（依赖活跃变量分析）

因此, 后向求解器、活跃变量分析、常量传播、可用表达式分析、复制传播均需实现

## 数据流分析求解算法框架

框架代码将每个数据流分析抽象成了对应的子类, 并使用统一的求解器进行求解

以活跃变量分析为例, 其主要提供的方法为

```c
typedef struct LiveVariableAnalysis {
    struct LiveVariableAnalysis_virtualTable {
        bool (*isForward) (LiveVariableAnalysis *t); // 前向/后向数据流分析?
        Set_IR_var *(*newBoundaryFact) (LiveVariableAnalysis *t, IR_function *func);
        Set_IR_var *(*newInitialFact) (LiveVariableAnalysis *t);
        bool (*meetInto) (LiveVariableAnalysis *t, Set_IR_var *fact, Set_IR_var *target);
        bool (*transferBlock) (LiveVariableAnalysis *t, IR_block *block, Set_IR_var *in_fact, Set_IR_var *out_fact);
        // 若结果发生改变则返回true
        ...
    } const *vTable;
} LiveVariableAnalysis;
```

若使用迭代求解器求解, 求解器算法如下

![迭代求解器](https://tai-e.pascal-lab.net/pa1/iter-alg.png)

此处 `meetInto()` 的设计可能与你设想的稍有差别：它接受 `fact` 和 `target` 两个参数并把 `fact` 集合并入 `target` 集合。这个函数有什么用呢？考虑一下上图中 `meetInto()` 附近的那行伪代码，它会取出 `B` 的所有后继，然后把它们 `IN facts` 的并集赋给 `OUT[B]`。如果这行代码用 `meetInto()` 来实现，那么我们就可以根据下图所示，用 `meetInto(IN[S], OUT[B])` 把 `B` 的每个后继 `S` 的 `IN fact` 直接并入到 `OUT[B]` 中：

![MeetInto Example](https://tai-e.pascal-lab.net/pa1/meetinto-example.png)

这样设计 `meetInto()` 是出于效率考量。首先，在一个控制流合并节点多次调用 `meetInto()` 时，我们都在改写同一个对象。这样，我们可以避免像伪代码 `OUT[S] = U…` 所描述的那样，每次合并两个集合就会创建出一个新的 `SetFact` 对象保存结果。当然，为了实现上面所说的 meet 策略，你需要在初始化阶段给每条语句的 `OUT[S]` 赋上和 `IN[S]` 一样的初值。

在  `src/IR_optimize/solver.c` 中, 前向分析的迭代求解器与worklist求解器代码已给出实现. 请实现后向分析的 TODO 内容

## 数据流分析具体应用

每个数据流分析都提供了结果输出函数(`printResult`)用来进行debug

### 活跃变量分析

请完成  `src/IR_optimize/live_variable_analysis.c` 中的 TODO 内容

### 常量传播

请完成  `src/IR_optimize/constant_propagation.c` 中的 TODO 内容

### 可用表达式分析

本框架对可用表达式分析采用了以下做法

1. 对相同的表达式使用相同的变量重新赋值, 例如

   ```
   v := v1 + v2
    =>
   expr1 := v1 + v2
   v := expr1
   ```
2. 对所有的expr对应的变量构造 e_kill 与 e_gen, 针对所有expr对应的变量进行数据流分析

注意: 在合并公共表达式前, 框架会先对每条运算语句尝试代数简化(`simple_expr_optimize`), 例如 `x + 0 => x`, `x * 1 => x`, `x - 0 => x`, `x / 1 => x`, 将可简化的运算语句替换为赋值语句

具体优化流程见下文

请完成  `src/IR_optimize/available_expressions_analysis.c` 中的 TODO 内容

### 复制传播

复制传播的数据流分析方程见下

[Data-flow analysis: copy propagation. (uwo.ca)](https://www.csd.uwo.ca/~mmorenom/CS447/Lectures/CodeOptimization.html/node8.html)

请完成  `src/IR_optimize/copy_propagation.c` 中的 TODO 内容

## 优化流水线

在 `src/IR_optimize/IR_optimize.c` 中, 对每个函数依次执行以下优化:

```
1. 常量传播          ConstantPropagation + constant_folding     将常量值传播并折叠到use处
2. 可用表达式分析    AEA merge + solve + remove                 合并、分析、消除公共子表达式
3. 复制传播          CopyPropagation + replace_available_use    将 x := y 的 x 替换为 y
4. 常量传播(第二轮)  同步骤1                                     经过前面优化后可能暴露新的常量
5. 死代码消除(迭代)  LVA + remove_dead_def                      反复分析直到没有新的死代码可删
```

### 全局公共子表达式消除流程示例

```
FUNCTION main :
READ v1
READ v2
v3 := v1 + v2
v4 := v1 + v2
IF v1 < v2 GOTO L1
v5 := v1 - v2
GOTO L2
LABEL L1 :
v6 := v1 - v2
LABEL L2 :
v7 := v1 - v2
WRITE v4
WRITE v7
RETURN #0
```

合并公共子表达式（`AvailableExpressionsAnalysis_merge_common_expr`）

```
FUNCTION main :
READ v1
READ v2
expr1 := v1 + v2
v3 := expr1
expr1 := v1 + v2
v4 := expr1
IF v1 < v2 GOTO L1
expr2 := v1 - v2
v5 := expr2
GOTO L2
LABEL L1 :
expr2 := v1 - v2
v6 := expr2
LABEL L2 :
expr2 := v1 - v2
v7 := expr2
WRITE v4
WRITE v7
RETURN #0
```

消除可用表达式（`AvailableExpressionsAnalysis_remove_available_expr_def`）

```
FUNCTION main :
READ v1
READ v2
expr1 := v1 + v2
v3 := expr1
v4 := expr1
IF v1 < v2 GOTO L1
expr2 := v1 - v2
v5 := expr2
GOTO L2
LABEL L1 :
expr2 := v1 - v2
v6 := expr2
LABEL L2 :
v7 := expr2
WRITE v4
WRITE v7
RETURN #0
```

复制传播（`CopyPropagation_replace_available_use_copy`）

```
FUNCTION main :
READ v1
READ v2
expr1 := v1 + v2
v3 := expr1
v4 := expr1
IF v1 < v2 GOTO L1
expr2 := v1 - v2
v5 := expr2
GOTO L2
LABEL L1 :
expr2 := v1 - v2
v6 := expr2
LABEL L2 :
v7 := expr2
WRITE expr1
WRITE expr2
RETURN #0
```

死代码消除（`LiveVariableAnalysis_remove_dead_def`）

```
FUNCTION main :
READ v1
READ v2
expr1 := v1 + v2
IF v1 < v2 GOTO L1
expr2 := v1 - v2
GOTO L2
LABEL L1 :
expr2 := v1 - v2
LABEL L2 :
WRITE expr1
WRITE expr2
RETURN #0
```

### 死代码消除

死代码消除通过迭代执行活跃变量分析实现: 每轮分析后, 检查每条 `IR_OP_STMT` 和 `IR_ASSIGN_STMT` 的 def 变量是否活跃, 不活跃则标记为死代码并删除. 反复迭代直到没有新的死代码产生

注意: 只有 `IR_OP_STMT` 和 `IR_ASSIGN_STMT` 会被检查和删除, 其他语句类型(load/store/call/read/write/if/goto/return)不参与死代码消除, 因为它们可能有副作用

事实上, 还可以根据常量传播的结果分析IF语句的跳转关系, 标记不可达基本块, 从而进一步完成死代码消除

## * 实验框架改进

目前实验框架的 gen/kill, use/def 关系是以 Stmt 为单位进行转移, 效率较低, 事实上可用将 Block 内所有 Stmt 的 gen/kill, use/def 进行合并以提高 transferBlock 的效率, 从而得到龙书课本上真正的算法实现. 如果你选择实现改进, 你需要建立从 block 到 gen/kill, use/def 的映射, 当然会遇到很多具体细节和难点.
