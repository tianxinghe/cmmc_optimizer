# 循环不变代码外提（LICM）

请实现 `src/IR_optimize/loop_invariant_code_motion.c` 中的 TODO 内容

该文件包含到达定值分析、支配集合计算、自然循环识别以及 LICM 优化本身的代码, 所有 TODO 均需完成

注意: 该优化 pass 默认未启用. 实现完成后, 需要在 `src/IR_optimize/IR_optimize.c` 中取消 `LoopInvariantCodeMotion_optimize(func)` 的注释以启用

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
typedef struct {
    IR_block *header;               // 循环头
    Set_IR_block_ptr body;          // 循环体（基本块集合）
    Set_ptr_IR_stmt_ptr invariant_stmts; // 识别出的不变语句
} NaturalLoop;
```

## 主要函数

### LoopInvariantCodeMotion_optimize(IR_function *func)

步骤：

1. **构建映射**  
   stmt_blk_map：语句 → 所属基本块

2. **到达定值分析**  
   ReachingDefinitions rd; iterative_solver(...)  
   得到每个块入口处每个变量的所有可能定义语句

3. **计算支配集合**  
   calculate_dominators(func)  
   得到 dom 映射：块 → 支配它的所有块的集合

4. **找出所有自然循环**  
   遍历所有边, 识别回边 n → h（h 支配 n）  
   调用 find_natural_loop 构造 NaturalLoop

5. **对每个循环执行优化**（步骤 A/B/C）

### A. 识别不变语句候选（迭代标记）

```c
while (changed) { ... }
```

条件：
- 语句类型为赋值 (`IR_ASSIGN_STMT`) 或二元运算 (`IR_OP_STMT`)
- 每个操作数满足以下之一：
  - 常量
  - 循环内定义且该定义语句已标记为不变
  - 所有到达定义均来自循环外

### B. 安全性修剪（迭代删除）

```c
while (changed) { ... }
```

依次检查, 不满足则从 invariant_stmts 中删除：

| 检查项 | 实现方式 |
|--------|----------|
| 依赖一致性 | 操作数在循环内定义但未标记不变 → 删除 |
| 支配所有出口 | stmt 所在块必须支配所有循环出口块 |
| 唯一定义 | 循环内不能有其他语句定义相同变量 |
| 支配所有使用 | 循环内所有使用该变量的语句必须被 stmt 支配 |
| 入口活跃性 | 若循环头入口处该变量有来自循环外的定义 → 不外提 |

### C. 移动代码

- **寻找唯一前置头（preheader）**: 循环头的唯一一个不在循环体内的前驱块. 若有多个循环外前驱则放弃移动
- 按程序顺序收集要移动的语句
- 将语句插入 preheader 末尾, 但要插在 preheader 已有的跳转/返回语句之前
- 更新 stmt_blk_map

## 关键辅助函数

| 函数 | 说明 |
|------|------|
| ReachingDefinitions_* | 到达定值分析 |
| calculate_dominators | 计算每个块的支配集合 (`Map<block, Set<block>>`) |
| is_dominated(dom, n, d) | 判断 d 是否支配 n |
| find_natural_loop | 从回边构造自然循环体 |
| Util_Set_Union | 集合并集 |

## 移动代码示例

```
// 移动前                        // 移动后
                                     v5 := v3 + v4    // 不变语句移到此处
    GOTO L1    (preheader)           GOTO L1           (preheader, GOTO 已有)
LABEL L1:                        LABEL L1:
    v5 := v3 + v4   // 不变          v6 := v5 * v1
    v6 := v5 * v1                    ...
    ...
```

## 总体执行流程图

```
开始
  │
  ├─► 构建语句→块映射 stmt_blk_map
  ├─► 执行到达定值分析 (ReachingDefinitions)
  ├─► 计算支配集合 (dom)
  │
  ├─► 查找所有自然循环 (遍历回边)
  │      │
  │      └─► 对每个循环：
  │            │
  │            ├─► 步骤A: 迭代标记不变语句
  │            │      │
  │            │      └─► 检查每条语句的操作数：
  │            │            - 常量 → 通过
  │            │            - 循环内定义且已标记不变 → 通过
  │            │            - 所有到达定义来自循环外 → 通过
  │            │            - 否则 → 不是不变
  │            │
  │            ├─► 步骤B: 安全修剪
  │            │      │
  │            │      ├─► 依赖一致性
  │            │      ├─► 支配所有出口
  │            │      ├─► 唯一定义
  │            │      ├─► 支配所有使用（含块内顺序）
  │            │      └─► 入口活跃性检查
  │            │
  │            └─► 步骤C: 代码移动
  │                   │
  │                   ├─► 寻找唯一前置头 preheader
  │                   ├─► 按程序顺序收集要移动的语句
  │                   ├─► 从原块删除, 插入 preheader 末尾跳转语句之前
  │                   └─► 更新 stmt_blk_map
  │
  └─► 释放资源, 结束
```
