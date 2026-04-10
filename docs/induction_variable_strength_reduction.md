# 归纳变量强度削减（Induction Variable Strength Reduction）

请实现 `src/IR_optimize/induction_variable_strength_reduction.c` 中的 TODO 内容

该文件包含到达定值分析、支配集合计算、自然循环识别以及归纳变量强度削减优化本身的代码, 所有 TODO 均需完成

注意: 该优化 pass 默认未启用. 实现完成后, 需要在 `src/IR_optimize/IR_optimize.c` 中添加对 `InductionVariableStrengthReduction_optimize(func)` 的调用以启用（需包含头文件 `induction_variable_strength_reduction.h`）

## 前置算法

### 到达定值分析 (Reaching Definitions)

到达定值分析是一种**前向**数据流分析, 用于确定程序中每个点处每个变量的所有可能定义语句

- **数据流值 (Fact)**: `Map<IR_var, Set<IR_stmt*>>`, 即每个变量映射到可能定义它的语句集合
- **Boundary Fact**: `OUT[Entry]` = 空 Map
- **Initial Fact**: 其余块的 `OUT[B]` 初始化为空 Map
- **Meet 操作**: 并集. 对每个变量, 将各前驱的定义语句集合取并集
- **Transfer 函数**: 对于定义了变量 `v` 的语句 `s`, kill 掉 `v` 的旧定义集合, gen 新定义 `{s}`

注: 到达定值分析复用了 `src/IR_optimize/solver.c` 中已实现的前向求解器 (`iterative_solver`)

### 支配集合计算

使用迭代不动点算法计算每个基本块的支配集合 (dominator set):

```
Dom(Entry) = {Entry}
Dom(n) = {n} ∪ ( ∩ Dom(p), for p ∈ pred(n) )
```

初始时 `Dom(Entry) = {Entry}`, 其余块 `Dom(n) = AllBlocks`. 迭代直到所有 `Dom` 集合不再变化

### 自然循环

回边 (back edge): CFG 中从节点 n 到节点 h 的边, 其中 h 支配 n. 回边的存在意味着存在循环

给定回边 n → h, 自然循环由循环头 h 和所有能不经过 h 到达 n 的节点组成. 使用栈/DFS 从 n 沿前驱反向搜索即可构造循环体

## 核心数据结构

```c
// 自然循环结构
typedef struct {
    IR_block *header;                 // 循环头
    Set_IR_block_ptr body;            // 循环体基本块集合
} NaturalLoop;

// 线性函数：a * i + b
typedef struct LinearFunction {
    int coeff;        // 乘数 a
    int const_term;   // 常数项 b
    IR_var base_var;  // 基础归纳变量
} LinearFunction;

// 归纳变量信息
typedef struct InductionVarInfo {
    IR_var var;              // 变量本身
    IR_var base;             // 基础归纳变量（基础变量自身为 IR_VAR_NONE）
    LinearFunction expr;     // 线性表达式
    int step;                // 每次迭代的增量（仅对基础变量有效）
    IR_stmt *update_stmt;    // 更新语句（如 i = i + step）
} InductionVarInfo;
```

## 整体流程

### Step 1：构建语句 → 基本块映射

stmt_blk_map：IR_stmt → IR_block, 用于定位定义所在块、判断循环内/外定义

### Step 2：到达定值分析

```c
ReachingDefinitions rd;
iterative_solver(...);
```

得到每个 block 入口变量的所有可能定义语句集合, 用于判断变量是否来自循环外

### Step 3：支配集合 + 自然循环分析

- calculate_dominators(func) → 得到 dom 关系（支配集合）
- 回边识别：n → h 且 h dominates n
- find_natural_loop() → 构造 NaturalLoop（循环体）

### Step 4：识别基础归纳变量

**is_basic_induction_update(stmt)** 识别以下模式：

```
i = i + c    (IR_OP_ADD, rd == 某个操作数, 另一个为常量)
i = i - c    (IR_OP_SUB, 同上)
```

判定逻辑：若变量在循环头入口处有来自循环外的定义, 且满足上述线性递增模式, 则加入 ind_vars, 其线性表达式为 `1 * i + 0`

### Step 5：识别派生归纳变量

**is_strength_reducible(stmt)** 识别以下模式：

```
x = i * c    (IR_OP_MUL, 一个操作数是归纳变量, 另一个是常量)
x = c * i    (同上, 顺序相反)
```

若 `i` 已在 ind_vars 中（基础或派生）, 则需要基于 `i` 的线性表达式组合计算 `x` 的表达式. 例如若 `i` 的表达式为 `a * j + b`（`j` 为基础归纳变量）, 则 `x = c * i` 的表达式为 `(c * a) * j + (c * b)`. 对于基础归纳变量 `i`（表达式为 `1 * i + 0`）, 则简化为 `c * i + 0`. 将 `x` 作为派生归纳变量加入 ind_vars, 迭代直到没有新的派生变量产生

### Step 6：强度削减替换

对每个可削减的归纳变量乘法语句, 执行以下替换：

**6.1 在 preheader 中初始化**

```
tmp = init_val * coeff
```

其中 `init_val` 是基础归纳变量在循环外的初始常量值（通过 `get_initial_constant` 从到达定值分析结果中提取）, `tmp` 是新生成的变量（通过 `ir_var_generator()` 创建）

**6.2 在循环内追加递推更新**（插入到基础归纳变量的更新语句之后）

```
tmp = tmp + coeff * step
```

**6.3 替换原语句**

```
x = tmp
```

## 关键辅助函数

| 函数 | 说明 |
|------|------|
| ReachingDefinitions_* | 到达定值分析 |
| calculate_dominators | 计算每个块的支配集合 (`Map<block, Set<block>>`) |
| is_dominated(dom, n, d) | 判断 d 是否支配 n |
| find_natural_loop | 从回边构造自然循环体 |
| get_preheader | 获取唯一循环前置块（循环头的唯一循环外前驱） |
| get_initial_constant | 从到达定值分析结果中提取变量的初始常量值（若来自循环外的常量赋值） |

## 安全性约束

| 检查项 | 说明 |
|--------|------|
| 循环外初始化 | 基础归纳变量必须存在来自循环外的初始值 |
| 定义唯一性 | 循环内不能有多个语句定义同一归纳变量 |
| 支配关系 | 使用点必须被定义支配 |
| 前驱唯一 | 循环头只能有一个循环外前驱（唯一 preheader） |

## 典型优化示例

优化前（假设 `i` 在循环外被初始化为 `i = 0`）：

```
LABEL L1:
    i = i + 1
    x = i * 10
    ...
    GOTO L1
```

优化后：

```
    t = 0 * 10          // preheader: tmp = init_val * coeff
    GOTO L1
LABEL L1:
    i = i + 1
    t = t + 10           // 递推: tmp = tmp + coeff * step (10 * 1)
    x = t                // 替换: 原乘法变为赋值
    ...
    GOTO L1
```

## 总体执行流程图

```
开始
│
├─► 构建 stmt → block 映射
├─► ReachingDefinitions 分析
├─► 计算支配集合 dom
├─► 构造自然循环 NaturalLoop
│
├─► 对每个循环：
│     │
│     ├─► Step 4: 识别基础归纳变量
│     │     └─► i = i ± c 模式检测
│     │
│     ├─► Step 5: 迭代识别派生归纳变量
│     │     └─► x = i * c / c * i 模式检测
│     │
│     └─► Step 6: 执行强度削减
│           ├─► preheader 初始化: tmp = init_val * coeff
│           ├─► loop 内递推替换: tmp = tmp + coeff * step
│           └─► 原语句替换: x = tmp
│
└─► 释放资源, 结束
```
