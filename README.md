# 实验五代码框架

该仓库为南京大学编译原理课程实验五代码框架. 

o 基于数据流分析模式的循环无关的全局优化
- 全局公共子表达式优化
- 全局无用代码消除优化
- 全局常量传播优化

o 循环不变式外提优化

o 归纳变量强度削减优化

注意: 实验框架会随时更新注释和修补bug, 请定期从远程仓库执行pull, 最近一次代码更新时间为 2026 年 4 月 10 日

希望大家能够帮助补充相关注释. 发现bug, 提供建议, 可联系助教.

# c语言实现面向对象编程

使用c语言实现面向对象编程请自行STFW

本实验用到的面向对象编程相关宏定义

```c
// 返回已执行init构造函数的指针, 类似于C++的 new obj 语句
#define NEW(TYPE, ...) ({                             \
            TYPE *TMP = (TYPE*)malloc(sizeof(TYPE));  \
            concat(TYPE, _init) (TMP, ##__VA_ARGS__); \
            TMP;                                      \
        })
// 从虚函数表vTable中调用函数, 指针需要解引用
#define VCALL(obj, func, ...) \
        (((obj).vTable)->func(&(obj), ##__VA_ARGS__))
// 执行teardown析构函数并free, 类似于C++的 delete obj 语句
#define DELETE(obj_ptr) \
        do { VCALL(*obj_ptr, teardown); free(obj_ptr); } while(0)
// 虚函数表中没有teardown, 需要显式调用特定类型的析构函数
#define RDELETE(type, obj_ptr) \
        do { concat(type, _teardown)(obj_ptr); free(obj_ptr); } while(0)
```

# 模板宏: 宏定义实现泛型编程

## Vec: 变长动态数组

### 头文件与模板定义

```c
#include <container/vector.h>
```

定义模板类型

```c
DEF_VECTOR(int) // 全局定义
```

宏展开结果大致为

```c
typedef struct Vec_int {
    struct Vec_int_virtualTable {
        void (*teardown)(Vec_int *vec);
        void (*resize)(Vec_int *vec, vec_size_t new_size);
        void (*push_back)(Vec_int *vec, int item);
        void (*pop_back)(Vec_int *vec);
        vec_size_t (*find)(Vec_int *vec, bool (*check)(int *arr_item, void *arg), void *arg);
        vec_size_t (*lower_bound)(Vec_int *vec, bool (*check)(int *arr_item, void *arg), void *arg);
        void (*insert)(Vec_int *vec, vec_size_t idx, int item);
        void (*delete)(Vec_int *vec, vec_size_t idx);
    } const *vTable;
    vec_size_t len; // arr_len
    int *arr;
    ...
} Vec_int;
void Vec_int_init(Vec_int *vec);
```

### 迭代语句

迭代器 it 为 TYPE* 指针, 解引用 *it 为内容

```c
for_vec(int, it, vec)
```

宏展开为

```c
for(int *it = vec.arr; it != vec.arr + vec.len; it ++)
```

### 使用示例

```c
void func() {
    Vec_int v;
    Vec_int_init(&v);
    // do somthing
    Vec_int_teardown(&v);
    // 或者
    Vec_int *v = NEW(Vec_int);

    // do somthing
    // 例如需要调用虚函数表中的函数, 可使用宏定义
    VCALL(v, pushback, 1);
    // 等价于
    v.vTable->push_back(&v, 1);

    DELETE(v); // v->vTable->teardown(v); free(v);
}
```

## List: 双向链表与队列

### 头文件与模板定义

```c
#include <container/list.h>
```

定义模板类型

```c
DEF_LIST(int) // 全局定义
```

宏展开结果大致为

```c
typedef struct ListNode_int {
    struct ListNode_int *pre, *nxt;
    int val;
} ListNode_int;
struct List_int {
    struct List_int_virtualTable {
        void (*teardown)(List_int *l);
        void (*insert_front)(List_int *l, ListNode_int *x, int val);
        void (*insert_back)(List_int *l, ListNode_int *x, int val);
        void (*push_front)(List_int *l, int val);
        void (*push_back)(List_int *l, int val);
        ListNode_int *(*delete)(List_int *l, ListNode_int *x); // 返回nxt节点(迭代器)
        void (*pop_front)(List_int *l);
        void (*pop_back)(List_int *l);
    } const *vTable;
    struct ListNode_int *head, *tail; // 当 list 为空时, head 与 tail 均为 NULL
};
void List_int_init(List_int *l);
```

### 迭代语句

迭代器 it 为 ListNode_TYPE* 链表节点指针, it->val 为内容

```c
for_list(int, it, list)
```

宏展开为

```c
for(ListNode_int *it = list.head; it; it = it->nxt)
```

## Map: 映射

### 头文件与模板定义

```c
#include <container/treap.h>
```

定义模板类型

```c
DEF_MAP(int, double) // 全局定义
DEF_MAP_CMP(int, double, CMP_FUNC)
    // CMP_FUNC 为自定义比较函数, 返回值为 a == b ? -1 : a < b
```

宏展开为

```c
typedef struct MapNode_int_double {
    ...
    int key;
    double val;
} MapNode_int_double;
struct Map_int_double {
    struct Map_int_double_virtualTable {
        void (*teardown)(Map_int_double *t);
        bool (*insert)(Map_int_double *t, int key, double val); // 若key已存在则不修改值, 并返回false
        bool (*delete)(Map_int_double *t, int key); // 若key不存在则返回false
        bool (*exist)(Map_int_double *t, int key);
        double (*get)(Map_int_double *t, int key); // 需保证 key 已经存在
        void (*set)(Map_int_double *t, int key, double val); // // 若key已存在则修改val值, 否则等价于insert
    } const *vTable;
    ...
};
void Map_int_double_init(Map_int_double *t);
```

### 迭代语句

迭代器 it 为 MapNode_KEY_TYPE_VAL_TYPE* 平衡树节点指针, {it->key, it->val} 为映射内容

```c
for_map(int, double, it, map) ...
```

宏展开为

```c
for(
    MapNode_KEY_TYPE_VAL_TYPE *it = TreapNode_first_iter(map.root);
    it != NULL;
    it = TreapNode_next_iter(it)
)
```

## Set: 集合

### 头文件与模板定义

```c
#include <container/treap.h>
```

定义模板类型

```c
DEF_SET(int) // 全局定义
DEF_SET_CMP(int, CMP_FUNC) // 自定义比较函数
```

宏展开为

```c
typedef struct SetNode_int {
    ...
    int key;
} SetNode_int;
struct Set_int {
    struct Set_int_virtualTable {
        void (*teardown)(Set_int *t);
        bool (*insert)(Set_int *t, int key); // 已存在返回false
        bool (*delete)(Set_int *t, int key); // 不存在返回false
        bool (*exist)(Set_int *t, int key);
    } const *vTable;
    ...
};
void Set_int_init(Set_int *t);
```

### 迭代语句

迭代器 it 为 SetNode_KEY_TYPE* 平衡树节点指针, it->key 为内容

```c
for_set(int, it, set) ...
```

宏展开为

```c
for(
    MapNode_KEY_TYPE_VAL_TYPE *it = TreapNode_first_iter(set.root);
    it != NULL;
    it = TreapNode_next_iter(it)
)
```

# IR数据结构

该实验框架已完成IR输入解析, 划分基本块, 建立控制流图等基本工作, 你只需要了解IR相关数据结构

## IR层次结构

- IR_program
  - IR_function
    - IR_block 基本快
      - IR_stmt 基本语句
        - def -> def变量
        - use_vec -> use变量数组

### IR_stmt 类型

```
IR_op_stmt:     rd := rs1 op rs2
IR_assign_stmt: rd := rs
IR_load_stmt:   rd := *rs_addr
IR_store_stmt:  *rd_addr = rs
IR_call_stmt:   (ARGS argv[argc]{rs1,rs2,...})
                rd := CALL func
IR_if_stmt:     IF rs1 relop rs2 GOTO true_label ELSE false_label
IR_goto_stmt:   GOTO label
IR_return_stmt: return rs
IR_read_stmt:   READ rd
IR_write_stmt:  WRITE rs
```

## IR结构代码

```c
//// ================================== IR var & label ==================================

typedef unsigned IR_var;   // 数字表示 v1, v2...
typedef unsigned IR_label; // 数字表示 L1, L2...
typedef unsigned IR_DEC_size_t;
enum {IR_VAR_NONE = 0};
enum {IR_LABEL_NONE = 0};

DEF_VECTOR(IR_var)
extern IR_var ir_var_generator();     // 获取新变量编号
extern IR_label ir_label_generator(); // 获取新label编号

//// ================================== IR ==================================

//// IR_val

typedef struct IR_val { // 常量或变量
    bool is_const;
    union {
        IR_var var;
        int const_val;
    };
} IR_val;

//// IR_stmt

typedef enum {
    IR_OP_STMT, IR_ASSIGN_STMT, IR_LOAD_STMT, IR_STORE_STMT, IR_IF_STMT, IR_GOTO_STMT, IR_CALL_STMT, IR_RETURN_STMT, IR_READ_STMT, IR_WRITE_STMT
} IR_stmt_type;

// use 数组, 需要手动忽略常量
typedef struct IR_use {
    unsigned use_cnt; // use 个数
    IR_val *use_vec;  // use 数组
}IR_use;

struct IR_stmt_virtualTable {
    void (*teardown) (IR_stmt *stmt);
    void (*print) (IR_stmt *stmt, FILE *out);
    IR_var (*get_def) (IR_stmt *stmt);        // def 最多只有一个, 不存在则返回 IR_VAR_NONE
    IR_use (*get_use_vec) (IR_stmt *stmt);
};

struct IR_stmt {
    struct {
        struct IR_stmt_virtualTable const *vTable;
        IR_stmt_type stmt_type;
        bool dead;
    };
};

//// IR_block

typedef struct {
    IR_label label;
    bool dead;
    List_IR_stmt_ptr stmts;
} IR_block, *IR_block_ptr;

//// IR_function

typedef struct {
    IR_var dec_addr;
    IR_DEC_size_t dec_size;
} IR_Dec;

typedef List_IR_block_ptr *List_ptr_IR_block_ptr;

typedef struct IR_function{
    char *func_name;
    Vec_IR_var params;
    Map_IR_var_IR_Dec map_dec; // dec_var => (addr_var, size)
    List_IR_block_ptr blocks;
    // Control Flow Graph
    IR_block *entry, *exit;
    Map_IR_label_IR_block_ptr map_blk_label; // Label -> Block 指针
    Map_IR_block_ptr_List_ptr_IR_block_ptr blk_pred, blk_succ; // Block 指针 -> List<前驱后继Block> 的指针
} IR_function, *IR_function_ptr;

//// IR_program

typedef struct IR_program {
    Vec_IR_function_ptr functions;
} IR_program;

//// ================================== Stmt ==================================

typedef enum {
    IR_OP_ADD, IR_OP_SUB, IR_OP_MUL, IR_OP_DIV
} IR_OP_TYPE;
typedef struct {
    CLASS_IR_stmt
    IR_OP_TYPE op;
    IR_var rd;
    union {
        IR_val use_vec[2];
        struct { IR_val rs1, rs2; };
    };
} IR_op_stmt;

typedef struct {
    CLASS_IR_stmt
    IR_var rd;
    union {
        IR_val use_vec[1];
        struct { IR_val rs; };
    };
} IR_assign_stmt;

typedef struct {
    CLASS_IR_stmt
    IR_var rd;
    union {
        IR_val use_vec[1];
        struct { IR_val rs_addr; };
    };
} IR_load_stmt;

typedef struct {
    CLASS_IR_stmt
    union {
        IR_val use_vec[2];
        struct { IR_val rd_addr, rs; };
    };
} IR_store_stmt;

typedef enum {
    IR_RELOP_EQ, IR_RELOP_NE, IR_RELOP_GT, IR_RELOP_GE, IR_RELOP_LT, IR_RELOP_LE
} IR_RELOP_TYPE;
typedef struct {
    CLASS_IR_stmt
    IR_RELOP_TYPE relop;
    union {
        IR_val use_vec[2];
        struct { IR_val rs1, rs2; };
    };
    IR_label true_label, false_label;
    IR_block *true_blk, *false_blk;
} IR_if_stmt;

typedef struct {
    CLASS_IR_stmt
    IR_label label;
    IR_block *blk;
} IR_goto_stmt;

typedef struct {
    CLASS_IR_stmt
    union {
        IR_val use_vec[1];
        struct { IR_val rs; };
    };
} IR_return_stmt;

typedef struct {
    CLASS_IR_stmt
    IR_var rd;
    char *func_name;
    unsigned argc;
    IR_val *argv;
} IR_call_stmt;

typedef struct {
    CLASS_IR_stmt
    IR_var rd;
} IR_read_stmt;

typedef struct {
    CLASS_IR_stmt
    union {
        IR_val use_vec[1];
        struct { IR_val rs; };
    };
} IR_write_stmt;
```

# 中间代码优化（三选一）

以下三个实验任务请选择其一完成:

- [数据流分析相关优化](docs/dataflow_analysis.md) — 全局公共子表达式消除、常量传播、死代码消除等
- [循环不变代码外提（LICM）](docs/loop_invariant_code_motion.md) — 识别并外提循环不变语句
- [归纳变量强度削减](docs/induction_variable_strength_reduction.md) — 将循环内乘法替换为加法递推

## 编译与运行

本实验的三个选做任务分别放在以下目录中：

- `src/IR_optimize/optional_task1`
- `src/IR_optimize/optional_task2`
- `src/IR_optimize/optional_task3`

编译时需要通过指定 `TASK` 变量来选择对应任务。

例如，要编译并运行**数据流分析相关优化（任务1）**，请执行：

```bash
make TASK=optional_task1
```

编译成功后，会生成可执行文件：

```bash
./parser
```

随后可以运行测试文件：

```bash
./parser test.cmm
```