#include <induction_variable_strength_reduction.h>
#include <container/treap.h>
#include <container/vector.h>
#include <container/list.h>
#include <stdlib.h>

//// ============================ 到达定值分析 (Reaching Definitions) ============================

DEF_SET(IR_stmt_ptr)
typedef Set_IR_stmt_ptr *Set_ptr_IR_stmt_ptr;
DEF_MAP(IR_var, Set_ptr_IR_stmt_ptr)
typedef Map_IR_var_Set_ptr_IR_stmt_ptr *RD_Fact;
DEF_MAP(IR_block_ptr, RD_Fact)
typedef Map_IR_block_ptr_RD_Fact *ptr_Map_IR_block_ptr_RD_Fact;

static void ReachingDefinitions_teardown(ReachingDefinitions *t);
static bool ReachingDefinitions_isForward(ReachingDefinitions *t);
static void *ReachingDefinitions_newBoundaryFact(ReachingDefinitions *t, IR_function *func);
static void *ReachingDefinitions_newInitialFact(ReachingDefinitions *t);
static void ReachingDefinitions_setInFact(ReachingDefinitions *t, IR_block *blk, void *fact);
static void ReachingDefinitions_setOutFact(ReachingDefinitions *t, IR_block *blk, void *fact);
static void *ReachingDefinitions_getInFact(ReachingDefinitions *t, IR_block *blk);
static void *ReachingDefinitions_getOutFact(ReachingDefinitions *t, IR_block *blk);
static bool ReachingDefinitions_meetInto(ReachingDefinitions *t, void *fact, void *target);
static bool ReachingDefinitions_transferBlock(ReachingDefinitions *t, IR_block *block, void *in_fact, void *out_fact);
static void ReachingDefinitions_printResult(ReachingDefinitions *t, IR_function *func);

static void ReachingDefinitions_teardown(ReachingDefinitions *t) {
    Map_IR_block_ptr_RD_Fact *in = (Map_IR_block_ptr_RD_Fact*)t->mapInFact;
    Map_IR_block_ptr_RD_Fact *out = (Map_IR_block_ptr_RD_Fact*)t->mapOutFact;
    for_map(IR_block_ptr, RD_Fact, i, *in) {
        for_map(IR_var, Set_ptr_IR_stmt_ptr, j, *i->val) DELETE(j->val);
        RDELETE(Map_IR_var_Set_ptr_IR_stmt_ptr, i->val);
    }
    for_map(IR_block_ptr, RD_Fact, i, *out) {
        for_map(IR_var, Set_ptr_IR_stmt_ptr, j, *i->val) DELETE(j->val);
        RDELETE(Map_IR_var_Set_ptr_IR_stmt_ptr, i->val);
    }
    RDELETE(Map_IR_block_ptr_RD_Fact, in);
    RDELETE(Map_IR_block_ptr_RD_Fact, out);
}

static bool ReachingDefinitions_isForward(ReachingDefinitions *t) { return true; }

static void *ReachingDefinitions_newBoundaryFact(ReachingDefinitions *t, IR_function *func) {
    return NEW(Map_IR_var_Set_ptr_IR_stmt_ptr);
}
static void *ReachingDefinitions_newInitialFact(ReachingDefinitions *t) {
    return NEW(Map_IR_var_Set_ptr_IR_stmt_ptr);
}
static void ReachingDefinitions_setInFact(ReachingDefinitions *t, IR_block *blk, void *fact) {
    VCALL(*(Map_IR_block_ptr_RD_Fact*)t->mapInFact, set, blk, (RD_Fact)fact);
}
static void ReachingDefinitions_setOutFact(ReachingDefinitions *t, IR_block *blk, void *fact) {
    VCALL(*(Map_IR_block_ptr_RD_Fact*)t->mapOutFact, set, blk, (RD_Fact)fact);
}
static void *ReachingDefinitions_getInFact(ReachingDefinitions *t, IR_block *blk) {
    return VCALL(*(Map_IR_block_ptr_RD_Fact*)t->mapInFact, get, blk);
}
static void *ReachingDefinitions_getOutFact(ReachingDefinitions *t, IR_block *blk) {
    return VCALL(*(Map_IR_block_ptr_RD_Fact*)t->mapOutFact, get, blk);
}

static bool Util_Set_Union(Set_ptr_IR_stmt_ptr dest, Set_ptr_IR_stmt_ptr src) {
    bool updated = false;
    for_set(IR_stmt_ptr, stmt, *src) {
        /* TODO: 如果 dest 中不存在 stmt->key，则插入并设置 updated = true */
    }
    return updated;
}

static bool ReachingDefinitions_meetInto(ReachingDefinitions *t, void *void_fact, void *void_target) {
    RD_Fact fact = (RD_Fact)void_fact;
    RD_Fact target = (RD_Fact)void_target;
    bool updated = false;
    for_map(IR_var, Set_ptr_IR_stmt_ptr, item, *fact) {
        IR_var var = item->key;
        Set_ptr_IR_stmt_ptr src_set = item->val;
        /* TODO: 如果 target 中没有 var，则创建新集合并插入；否则将 src_set 合并到现有集合中，更新 updated */
    }
    return updated;
}

static void ReachingDefinitions_transferStmt(ReachingDefinitions *t, IR_stmt *stmt, RD_Fact fact) {
    IR_var def = VCALL(*stmt, get_def);
    if (def != IR_VAR_NONE) {
        Set_ptr_IR_stmt_ptr def_set;
        /* TODO: 如果 fact 中已有 def，则获取其集合并清空；否则创建新集合并插入 */
        VCALL(*def_set, insert, stmt);
    }
}

static bool ReachingDefinitions_transferBlock(ReachingDefinitions *t, IR_block *block, void *in_fact_void, void *out_fact_void) {
    RD_Fact in_fact = (RD_Fact)in_fact_void;
    RD_Fact out_fact = (RD_Fact)out_fact_void;
    RD_Fact curr_fact = (RD_Fact)ReachingDefinitions_newInitialFact(t);
    ReachingDefinitions_meetInto(t, in_fact, curr_fact);
    for_list(IR_stmt_ptr, i, block->stmts) ReachingDefinitions_transferStmt(t, i->val, curr_fact);
    bool changed = false;
    /* TODO: 比较 curr_fact 和 out_fact，如果不同则更新 out_fact 并设置 changed = true。
       注意需要处理 out_fact 中缺失的变量。 */
    
    /* 清理 curr_fact */
    for_map(IR_var, Set_ptr_IR_stmt_ptr, j, *curr_fact) DELETE(j->val);
    RDELETE(Map_IR_var_Set_ptr_IR_stmt_ptr, curr_fact);
    return changed;
}

static void ReachingDefinitions_printResult(ReachingDefinitions *t, IR_function *func) { }

void ReachingDefinitions_init(ReachingDefinitions *t) {
    const static struct ReachingDefinitions_virtualTable vTable = {
        .teardown = ReachingDefinitions_teardown,
        .isForward = ReachingDefinitions_isForward,
        .newBoundaryFact = ReachingDefinitions_newBoundaryFact,
        .newInitialFact = ReachingDefinitions_newInitialFact,
        .setInFact = ReachingDefinitions_setInFact,
        .setOutFact = ReachingDefinitions_setOutFact,
        .getInFact = ReachingDefinitions_getInFact,
        .getOutFact = ReachingDefinitions_getOutFact,
        .meetInto = ReachingDefinitions_meetInto,
        .transferBlock = ReachingDefinitions_transferBlock,
        .printResult = ReachingDefinitions_printResult
    };
    t->vTable = &vTable;
    t->mapInFact = NEW(Map_IR_block_ptr_RD_Fact);
    t->mapOutFact = NEW(Map_IR_block_ptr_RD_Fact);
}

//// ============================ 支配树 (Dominator Tree) ============================
DEF_SET(IR_block_ptr)
typedef Set_IR_block_ptr *Set_ptr_IR_block_ptr;
DEF_MAP(IR_block_ptr, Set_ptr_IR_block_ptr)

static Map_IR_block_ptr_Set_ptr_IR_block_ptr calculate_dominators(IR_function *func) {
    Map_IR_block_ptr_Set_ptr_IR_block_ptr dom;
    Map_IR_block_ptr_Set_ptr_IR_block_ptr_init(&dom);
    Set_IR_block_ptr all_blocks;
    Set_IR_block_ptr_init(&all_blocks);
    for_list(IR_block_ptr, b, func->blocks) VCALL(all_blocks, insert, b->val);
    for_list(IR_block_ptr, b, func->blocks) {
        Set_ptr_IR_block_ptr s = NEW(Set_IR_block_ptr);
        if (b->val == func->entry) VCALL(*s, insert, b->val);
        else {
            /* TODO: 将 all_blocks 的所有元素插入 s */
        }
        VCALL(dom, insert, b->val, s);
    }
    bool changed = true;
    while (changed) {
        changed = false;
        for_list(IR_block_ptr, b, func->blocks) {
            if (b->val == func->entry) continue;
            IR_block *blk = b->val;
            Set_IR_block_ptr new_dom;
            Set_IR_block_ptr_init(&new_dom);
            VCALL(new_dom, insert, blk);
            Set_ptr_IR_block_ptr temp_intersect = NEW(Set_IR_block_ptr);
            for_set(IR_block_ptr, item, all_blocks) VCALL(*temp_intersect, insert, item->key);
            List_ptr_IR_block_ptr preds = VCALL(func->blk_pred, get, blk);
            if (preds) {
                for_list(IR_block_ptr, p, *preds) {
                    Set_ptr_IR_block_ptr p_dom = VCALL(dom, get, p->val);
                    Set_IR_block_ptr next_intersect;
                    Set_IR_block_ptr_init(&next_intersect);
                    /* TODO: 计算 temp_intersect 和 p_dom 的交集，存入 next_intersect，然后替换 temp_intersect */
                    Set_IR_block_ptr_teardown(&next_intersect);
                }
            }
            /* TODO: 将 temp_intersect 的所有元素加入 new_dom */
            DELETE(temp_intersect);
            Set_ptr_IR_block_ptr old_dom = VCALL(dom, get, blk);
            /* TODO: 比较 new_dom 和 old_dom，若不同则更新 old_dom 并设置 changed = true */
            Set_IR_block_ptr_teardown(&new_dom);
        }
    }
    Set_IR_block_ptr_teardown(&all_blocks);
    return dom;
}

static bool is_dominated(Map_IR_block_ptr_Set_ptr_IR_block_ptr dom, IR_block *n, IR_block *d) {
    Set_ptr_IR_block_ptr s = VCALL(dom, get, n);
    /* TODO: 返回 d 是否在 s 中 */
}

//// ============================ 自然循环 (Natural Loop) ============================
typedef struct {
    IR_block *header;
    Set_IR_block_ptr body;
} NaturalLoop;

DEF_VECTOR(NaturalLoop)
DEF_VECTOR(IR_block_ptr)

static void find_natural_loop(IR_function *func, Map_IR_block_ptr_Set_ptr_IR_block_ptr dom,
                              IR_block *n, IR_block *h, NaturalLoop *loop) {
    loop->header = h;
    Set_IR_block_ptr_init(&loop->body);
    VCALL(loop->body, insert, h);
    Vec_IR_block_ptr stack;
    Vec_IR_block_ptr_init(&stack);
    if (n != h) {
        VCALL(loop->body, insert, n);
        VCALL(stack, push_back, n);
    }
    while (stack.len > 0) {
        IR_block *m = stack.arr[stack.len - 1];
        VCALL(stack, pop_back);
        List_ptr_IR_block_ptr preds = VCALL(func->blk_pred, get, m);
        if (preds) {
            for_list(IR_block_ptr, p, *preds) {
                /* TODO: 如果 p->val 不在 loop->body 中，则插入并压入栈 */
            }
        }
    }
    Vec_IR_block_ptr_teardown(&stack);
}

//// ============================ 归纳变量强度削减 ============================

// 线性函数表示 a*i + b
typedef struct LinearFunction {
    int coeff;      // 乘数 a
    int const_term; // 常数项 b
    IR_var base_var; // 基础归纳变量
} LinearFunction;

// 归纳变量信息
typedef struct InductionVarInfo {
    IR_var var;                 // 变量本身
    IR_var base;                // 基础归纳变量（基础变量自身为 IR_VAR_NONE）
    LinearFunction expr;        // 表达式
    int step;                   // 每次迭代的增量（仅对基础变量有效）
    IR_stmt *update_stmt;       // 更新语句（如 i = i + step）
} InductionVarInfo;

DEF_MAP(IR_var, InductionVarInfo)
DEF_MAP(IR_stmt_ptr, IR_block_ptr)
DEF_VECTOR(IR_stmt_ptr)

// 判断是否为基础归纳变量的更新语句 (i = i + c 或 i = i - c)
static bool is_basic_induction_update(IR_stmt *stmt, IR_var *var, int *step) {
    if (stmt->stmt_type != IR_OP_STMT) return false;
    IR_op_stmt *op = (IR_op_stmt*)stmt;
    if (op->op != IR_OP_ADD && op->op != IR_OP_SUB) return false;
    /* TODO: 检查形如 rd = rs1 op rs2，其中 rd == rs1 或 rd == rs2，另一个是常量。
       如果是 i = i + c，则 *var = i, *step = c；如果是 i = i - c，则 *step = -c。
       返回 true，否则 false。 */
    return false;
}

// 判断语句是否可以通过强度削减优化（如 rd = i * c 或 rd = c * i）
static bool is_strength_reducible(IR_stmt *stmt, IR_var *ind_var, int *coeff, IR_var *def_var) {
    if (stmt->stmt_type != IR_OP_STMT) return false;
    IR_op_stmt *op = (IR_op_stmt*)stmt;
    if (op->op != IR_OP_MUL) return false;
    /* TODO: 检查形如 rd = rs1 * rs2，其中一个操作数是归纳变量，另一个是常量。
       设置 ind_var, coeff, def_var，返回 true。否则返回 false。 */
    return false;
}

static IR_block *get_preheader(IR_function *func, NaturalLoop *loop) {
    IR_block *preheader = NULL;
    List_ptr_IR_block_ptr preds = VCALL(func->blk_pred, get, loop->header);
    if (preds) {
        for_list(IR_block_ptr, p, *preds) {
            if (!VCALL(loop->body, exist, p->val)) {
                if (preheader == NULL) preheader = p->val;
                else { preheader = NULL; break; }
            }
        }
    }
    return preheader;
}

// 从到达定值事实中提取变量的初始常量值（若来自循环外的常量定义）
static bool get_initial_constant(RD_Fact header_in, IR_var var, int *init_val) {
    /* TODO: 如果 header_in 中存在 var，且其所有定义语句都是常量赋值（如 v = 5），
       且这些定义语句都来自循环外，则提取该常量值存入 init_val，返回 true。否则返回 false。 */
    return false;
}

void InductionVariableStrengthReduction_optimize(IR_function *func) {
    // 1. 构建语句 -> 基本块映射
    Map_IR_stmt_ptr_IR_block_ptr stmt_blk_map;
    Map_IR_stmt_ptr_IR_block_ptr_init(&stmt_blk_map);
    for_list(IR_block_ptr, b, func->blocks) {
        for_list(IR_stmt_ptr, s, b->val->stmts) {
            VCALL(stmt_blk_map, insert, s->val, b->val);
        }
    }

    // 2. 到达定值分析
    ReachingDefinitions rd;
    ReachingDefinitions_init(&rd);
    iterative_solver((DataflowAnalysis*)&rd, func);

    // 3. 支配树计算
    Map_IR_block_ptr_Set_ptr_IR_block_ptr dom = calculate_dominators(func);

    // 4. 查找所有自然循环
    Vec_NaturalLoop loops;
    Vec_NaturalLoop_init(&loops);
    for_list(IR_block_ptr, b, func->blocks) {
        List_ptr_IR_block_ptr succs = VCALL(func->blk_succ, get, b->val);
        if (succs) {
            for_list(IR_block_ptr, s, *succs) {
                if (is_dominated(dom, b->val, s->val)) {
                    NaturalLoop loop;
                    find_natural_loop(func, dom, b->val, s->val, &loop);
                    VCALL(loops, push_back, loop);
                }
            }
        }
    }

    // 5. 对每个循环进行优化
    for (int i = 0; i < loops.len; ++i) {
        NaturalLoop *loop = &loops.arr[i];
        IR_block *preheader = get_preheader(func, loop);
        if (!preheader) continue;  // 没有唯一前置头，放弃优化

        // 步骤 A. 识别基础归纳变量
        Map_IR_var_InductionVarInfo ind_vars;
        Map_IR_var_InductionVarInfo_init(&ind_vars);
        for_set(IR_block_ptr, blk_node, loop->body) {
            IR_block *blk = blk_node->key;
            for_list(IR_stmt_ptr, s_node, blk->stmts) {
                IR_stmt *stmt = s_node->val;
                IR_var var;
                int step;
                if (is_basic_induction_update(stmt, &var, &step)) {
                    RD_Fact header_in = VCALL(*(Map_IR_block_ptr_RD_Fact*)rd.mapInFact, get, loop->header);
                    /* TODO: 检查 var 在循环头入口处是否有来自循环外的定义（且是常量或至少有一个初始值），
                       若有则将其记录为基础归纳变量，插入 ind_vars。 */
                    InductionVarInfo info = {
                        .var = var,
                        .base = IR_VAR_NONE,
                        .step = step,
                        .update_stmt = stmt,
                        .expr = { .coeff = 1, .const_term = 0, .base_var = var }
                    };
                    VCALL(ind_vars, insert, var, info);
                }
            }
        }

        // 步骤 B. 迭代检测派生归纳变量（如 j = 2*i + 1）
        bool changed = true;
        while (changed) {
            changed = false;
            for_set(IR_block_ptr, blk_node, loop->body) {
                IR_block *blk = blk_node->key;
                for_list(IR_stmt_ptr, s_node, blk->stmts) {
                    IR_stmt *stmt = s_node->val;
                    IR_var ind_var;
                    int coeff;
                    IR_var def_var;
                    if (is_strength_reducible(stmt, &ind_var, &coeff, &def_var)) {
                        /* TODO: 如果 ind_var 已经在 ind_vars 中（基础或派生），
                           则计算 def_var 的表达式为 coeff * (ind_var的表达式)，
                           然后将其作为派生归纳变量插入 ind_vars，并设置 changed = true。 */
                    }
                }
            }
        }

        // 步骤 C. 执行强度削减替换
        for_set(IR_block_ptr, blk_node, loop->body) {
            IR_block *blk = blk_node->key;
            for_list(IR_stmt_ptr, s_node, blk->stmts) {
                IR_stmt *stmt = s_node->val;
                IR_var ind_var;
                int coeff;
                IR_var def_var;
                if (is_strength_reducible(stmt, &ind_var, &coeff, &def_var)) {
                    /* TODO: 如果 ind_var 是归纳变量（在 ind_vars 中），则执行强度削减：
                       1. 获取 ind_var 的初始值 init_val（通过 get_initial_constant 从循环头入口的到达定值获得）
                       2. 在 preheader 中生成初始化语句：new_var = init_val * coeff
                       3. 在 ind_var 的更新语句后插入增量更新：new_var = new_var + coeff * step
                       4. 将原 stmt 替换为 def_var = new_var
                       注意使用 ir_var_generator() 创建新变量，插入语句时保持正确的顺序。 */
                }
            }
        }

        // 清理当前循环的归纳变量映射
        Map_IR_var_InductionVarInfo_teardown(&ind_vars);
        Set_IR_block_ptr_teardown(&loop->body);
    }

    // 清理全局数据结构
    Vec_NaturalLoop_teardown(&loops);
    for_map(IR_block_ptr, Set_ptr_IR_block_ptr, i, dom) DELETE(i->val);
    Map_IR_block_ptr_Set_ptr_IR_block_ptr_teardown(&dom);
    rd.vTable->teardown(&rd);
    Map_IR_stmt_ptr_IR_block_ptr_teardown(&stmt_blk_map);
}