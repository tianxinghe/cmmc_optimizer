#include <loop_invariant_code_motion.h>
#include <container/treap.h>
#include <container/vector.h>
#include <container/list.h>
#include <stdlib.h>

//// ============================ Reaching Definitions Analysis (到达定值分析) ============================

DEF_SET(IR_stmt_ptr)
typedef Set_IR_stmt_ptr *Set_ptr_IR_stmt_ptr;
DEF_MAP(IR_var, Set_ptr_IR_stmt_ptr)
typedef Map_IR_var_Set_ptr_IR_stmt_ptr *RD_Fact;
DEF_MAP(IR_block_ptr, RD_Fact)
typedef Map_IR_block_ptr_RD_Fact *ptr_Map_IR_block_ptr_RD_Fact;

static void LICM_ReachingDefinitions_teardown(LICM_ReachingDefinitions *t);
static bool LICM_ReachingDefinitions_isForward(LICM_ReachingDefinitions *t);
static void *LICM_ReachingDefinitions_newBoundaryFact(LICM_ReachingDefinitions *t, IR_function *func);
static void *LICM_ReachingDefinitions_newInitialFact(LICM_ReachingDefinitions *t);
static void LICM_ReachingDefinitions_setInFact(LICM_ReachingDefinitions *t, IR_block *blk, void *fact);
static void LICM_ReachingDefinitions_setOutFact(LICM_ReachingDefinitions *t, IR_block *blk, void *fact);
static void *LICM_ReachingDefinitions_getInFact(LICM_ReachingDefinitions *t, IR_block *blk);
static void *LICM_ReachingDefinitions_getOutFact(LICM_ReachingDefinitions *t, IR_block *blk);
static bool LICM_ReachingDefinitions_meetInto(LICM_ReachingDefinitions *t, void *fact, void *target);
static bool LICM_ReachingDefinitions_transferBlock(LICM_ReachingDefinitions *t, IR_block *block, void *in_fact, void *out_fact);
static void LICM_ReachingDefinitions_printResult(LICM_ReachingDefinitions *t, IR_function *func);

static void LICM_ReachingDefinitions_teardown(LICM_ReachingDefinitions *t) {
    Map_IR_block_ptr_RD_Fact *in = (Map_IR_block_ptr_RD_Fact*)t->mapInFact;
    Map_IR_block_ptr_RD_Fact *out = (Map_IR_block_ptr_RD_Fact*)t->mapOutFact;

    for_map(IR_block_ptr, RD_Fact, i, *in) {
        for_map(IR_var, Set_ptr_IR_stmt_ptr, j, *i->val) {
            DELETE(j->val);
        }
        RDELETE(Map_IR_var_Set_ptr_IR_stmt_ptr, i->val);
    }
    for_map(IR_block_ptr, RD_Fact, i, *out) {
        for_map(IR_var, Set_ptr_IR_stmt_ptr, j, *i->val) {
            DELETE(j->val);
        }
        RDELETE(Map_IR_var_Set_ptr_IR_stmt_ptr, i->val);
    }
    RDELETE(Map_IR_block_ptr_RD_Fact, in);
    RDELETE(Map_IR_block_ptr_RD_Fact, out);
}

static bool LICM_ReachingDefinitions_isForward(LICM_ReachingDefinitions *t) { return true; }

static void *LICM_ReachingDefinitions_newBoundaryFact(LICM_ReachingDefinitions *t, IR_function *func) {
    return NEW(Map_IR_var_Set_ptr_IR_stmt_ptr);
}

static void *LICM_ReachingDefinitions_newInitialFact(LICM_ReachingDefinitions *t) {
    return NEW(Map_IR_var_Set_ptr_IR_stmt_ptr);
}

static void LICM_ReachingDefinitions_setInFact(LICM_ReachingDefinitions *t, IR_block *blk, void *fact) {
    VCALL(*(Map_IR_block_ptr_RD_Fact*)t->mapInFact, set, blk, (RD_Fact)fact);
}
static void LICM_ReachingDefinitions_setOutFact(LICM_ReachingDefinitions *t, IR_block *blk, void *fact) {
    VCALL(*(Map_IR_block_ptr_RD_Fact*)t->mapOutFact, set, blk, (RD_Fact)fact);
}
static void *LICM_ReachingDefinitions_getInFact(LICM_ReachingDefinitions *t, IR_block *blk) {
    return VCALL(*(Map_IR_block_ptr_RD_Fact*)t->mapInFact, get, blk);
}
static void *LICM_ReachingDefinitions_getOutFact(LICM_ReachingDefinitions *t, IR_block *blk) {
    return VCALL(*(Map_IR_block_ptr_RD_Fact*)t->mapOutFact, get, blk);
}

static bool Util_Set_Union(Set_ptr_IR_stmt_ptr dest, Set_ptr_IR_stmt_ptr src) {
    bool updated = false;
    for_set(IR_stmt_ptr, stmt, *src) {
        /* TODO: 如果 dest 中不存在 stmt->key，则插入并设置 updated = true */
    }
    return updated;
}

static bool LICM_ReachingDefinitions_meetInto(LICM_ReachingDefinitions *t, void *void_fact, void *void_target) {
    RD_Fact fact = (RD_Fact)void_fact;
    RD_Fact target = (RD_Fact)void_target;
    bool updated = false;
    for_map(IR_var, Set_ptr_IR_stmt_ptr, item, *fact) {
        IR_var var = item->key;
        Set_ptr_IR_stmt_ptr src_set = item->val;
        
        /* TODO: 如果 target 中没有 var，则创建新集合并插入；否则将 src_set 合并到现有集合。
           注意更新 updated 标志。 */
    }
    return updated;
}

static void LICM_ReachingDefinitions_transferStmt(LICM_ReachingDefinitions *t, IR_stmt *stmt, RD_Fact fact) {
    IR_var def = VCALL(*stmt, get_def);
    if (def != IR_VAR_NONE) {
        Set_ptr_IR_stmt_ptr def_set;
        /* TODO: 如果 fact 中已有 def，则清空该集合；否则创建新集合并插入。 */
        /* 然后将 stmt 插入到 def_set 中。 */
    }
}

static bool LICM_ReachingDefinitions_transferBlock(LICM_ReachingDefinitions *t, IR_block *block, void *in_fact_void, void *out_fact_void) {
    RD_Fact in_fact = (RD_Fact)in_fact_void;
    RD_Fact out_fact = (RD_Fact)out_fact_void;
    RD_Fact curr_fact = (RD_Fact)LICM_ReachingDefinitions_newInitialFact(t);
    LICM_ReachingDefinitions_meetInto(t, in_fact, curr_fact);
    
    for_list(IR_stmt_ptr, i, block->stmts) {
        LICM_ReachingDefinitions_transferStmt(t, i->val, curr_fact);
    }
    
    bool changed = false;
    /* TODO: 比较 curr_fact 和 out_fact，如果不同则更新 out_fact 并设置 changed = true。
       注意需要处理 out_fact 中缺失的变量。 */
    
    /* 清理 curr_fact */
    for_map(IR_var, Set_ptr_IR_stmt_ptr, j, *curr_fact) DELETE(j->val);
    RDELETE(Map_IR_var_Set_ptr_IR_stmt_ptr, curr_fact);
    return changed;
}

static void LICM_ReachingDefinitions_printResult(LICM_ReachingDefinitions *t, IR_function *func) { }

void LICM_ReachingDefinitions_init(LICM_ReachingDefinitions *t) {
    const static struct LICM_ReachingDefinitions_virtualTable vTable = {
        .teardown = LICM_ReachingDefinitions_teardown,
        .isForward = LICM_ReachingDefinitions_isForward,
        .newBoundaryFact = LICM_ReachingDefinitions_newBoundaryFact,
        .newInitialFact = LICM_ReachingDefinitions_newInitialFact,
        .setInFact = LICM_ReachingDefinitions_setInFact,
        .setOutFact = LICM_ReachingDefinitions_setOutFact,
        .getInFact = LICM_ReachingDefinitions_getInFact,
        .getOutFact = LICM_ReachingDefinitions_getOutFact,
        .meetInto = LICM_ReachingDefinitions_meetInto,
        .transferBlock = LICM_ReachingDefinitions_transferBlock,
        .printResult = LICM_ReachingDefinitions_printResult
    };
    t->vTable = &vTable;
    t->mapInFact = NEW(Map_IR_block_ptr_RD_Fact);
    t->mapOutFact = NEW(Map_IR_block_ptr_RD_Fact);
}

//// ============================ LICM ============================

DEF_SET(IR_block_ptr)
typedef Set_IR_block_ptr *Set_ptr_IR_block_ptr;
DEF_MAP(IR_block_ptr, Set_ptr_IR_block_ptr)

static Map_IR_block_ptr_Set_ptr_IR_block_ptr calculate_dominators(IR_function *func) {
    Map_IR_block_ptr_Set_ptr_IR_block_ptr dom;
    Map_IR_block_ptr_Set_ptr_IR_block_ptr_init(&dom);
    Set_IR_block_ptr all_blocks;
    Set_IR_block_ptr_init(&all_blocks);
    for_list(IR_block_ptr, b, func->blocks) VCALL(all_blocks, insert, b->val);
    
    // 初始化 Dom 集合
    for_list(IR_block_ptr, b, func->blocks) {
        Set_ptr_IR_block_ptr s = NEW(Set_IR_block_ptr);
        if (b->val == func->entry) {
            VCALL(*s, insert, b->val);
        } else {
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
                    /* TODO: 计算 temp_intersect 和 p_dom 的交集，存入 next_intersect */
                    /* 然后将 temp_intersect 替换为 next_intersect */
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

typedef struct {
    IR_block *header;
    Set_IR_block_ptr body;
    Set_ptr_IR_stmt_ptr invariant_stmts;
} NaturalLoop;

DEF_VECTOR(NaturalLoop)
DEF_VECTOR(IR_block_ptr)

static void find_natural_loop(IR_function *func, Map_IR_block_ptr_Set_ptr_IR_block_ptr dom, IR_block *n, IR_block *h, NaturalLoop *loop) {
    loop->header = h;
    Set_IR_block_ptr_init(&loop->body);
    loop->invariant_stmts = NEW(Set_IR_stmt_ptr);
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

DEF_MAP(IR_stmt_ptr, IR_block_ptr)
DEF_VECTOR(IR_stmt_ptr)

void LoopInvariantCodeMotion_optimize(IR_function *func) {
    // 1. 构建语句 -> 块映射
    Map_IR_stmt_ptr_IR_block_ptr stmt_blk_map;
    Map_IR_stmt_ptr_IR_block_ptr_init(&stmt_blk_map);
    for_list(IR_block_ptr, b, func->blocks) {
        for_list(IR_stmt_ptr, s, b->val->stmts) {
            VCALL(stmt_blk_map, insert, s->val, b->val);
        }
    }

    // 2. 到达定值分析
    LICM_ReachingDefinitions rd;
    LICM_ReachingDefinitions_init(&rd);
    iterative_solver((DataflowAnalysis*)&rd, func);
    
    // 3. 支配树
    Map_IR_block_ptr_Set_ptr_IR_block_ptr dom = calculate_dominators(func);
    
    // 4. 查找循环
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
    
    // 5. 优化过程
    for (int i = 0; i < loops.len; ++i) {
        NaturalLoop *loop = &loops.arr[i];

        // ---------- 步骤 A. 识别循环不变语句候选 ----------
        bool changed = true;
        while (changed) {
            changed = false;
            for_set(IR_block_ptr, blk_node, loop->body) {
                IR_block *blk = blk_node->key;
                for_list(IR_stmt_ptr, s_node, blk->stmts) {
                    IR_stmt *stmt = s_node->val;
                    if (VCALL(*loop->invariant_stmts, exist, stmt)) continue;
                    if (stmt->stmt_type != IR_ASSIGN_STMT && stmt->stmt_type != IR_OP_STMT) continue;
                    
                    bool is_inv = true;
                    IR_use use = VCALL(*stmt, get_use_vec);
                    for (int u = 0; u < use.use_cnt; ++u) {
                        if (use.use_vec[u].is_const) continue;
                        IR_var v = use.use_vec[u].var;
                        
                        // 检查同一基本块内的前向定义
                        bool local_def_found = false;
                        IR_stmt *closest_local_def = NULL;
                        for (ListNode_IR_stmt_ptr *it = blk->stmts.head; it && it->val != stmt; it = it->nxt) {
                            if (VCALL(*it->val, get_def) == v) {
                                local_def_found = true;
                                closest_local_def = it->val;
                            }
                        }
                        if (local_def_found) {
                            /* TODO: 如果 closest_local_def 不在 invariant_stmts 中，则 is_inv = false */
                            continue;
                        }
                        
                        // 否则通过到达定值检查定义是否在循环外或已标记不变
                        RD_Fact in_fact = VCALL(*(Map_IR_block_ptr_RD_Fact*)rd.mapInFact, get, blk);
                        /* TODO: 如果 in_fact 中没有 v 的定义，则继续；否则遍历所有定义，若定义在循环体内且不在 invariant_stmts 中，则 is_inv = false */
                    }
                    if (is_inv) {
                        VCALL(*loop->invariant_stmts, insert, stmt);
                        changed = true;
                    }
                }
            }
        }

        // ---------- 步骤 B. 安全性修剪 ----------
        changed = true;
        while (changed) {
            changed = false;
            Vec_IR_stmt_ptr to_remove;
            Vec_IR_stmt_ptr_init(&to_remove);
            for_set(IR_stmt_ptr, s_node, *loop->invariant_stmts) {
                IR_stmt *stmt = s_node->key;
                IR_block *stmt_blk = VCALL(stmt_blk_map, get, stmt);
                bool keep = true;
                
                // 1) 依赖一致性（重新检查）
                IR_use use = VCALL(*stmt, get_use_vec);
                for (int u = 0; u < use.use_cnt; ++u) {
                    if (use.use_vec[u].is_const) continue;
                    IR_var v = use.use_vec[u].var;
                    /* TODO: 类似步骤 A 重新验证，如果依赖的定义不再是不变的，则 keep = false */
                }
                if (!keep) { VCALL(to_remove, push_back, stmt); continue; }
                
                // 2) 支配所有出口
                bool dom_exits = true;
                for_set(IR_block_ptr, b_node, loop->body) {
                    IR_block *b = b_node->key;
                    List_ptr_IR_block_ptr succs = VCALL(func->blk_succ, get, b);
                    if (succs) {
                        for_list(IR_block_ptr, s, *succs) {
                            if (!VCALL(loop->body, exist, s->val)) {
                                /* TODO: 如果 b 不被 stmt_blk 支配，则 dom_exits = false */
                            }
                        }
                    }
                    if (!dom_exits) break;
                }
                if (!dom_exits) { VCALL(to_remove, push_back, stmt); continue; }
                
                // 3) 唯一定义
                IR_var def = VCALL(*stmt, get_def);
                if (def != IR_VAR_NONE) {
                    bool unique = true;
                    for_set(IR_block_ptr, b_node, loop->body) {
                        for_list(IR_stmt_ptr, other_node, b_node->key->stmts) {
                            IR_stmt *other = other_node->val;
                            /* TODO: 如果存在 other != stmt 且定义了相同变量，则 unique = false */
                        }
                        if (!unique) break;
                    }
                    if (!unique) { VCALL(to_remove, push_back, stmt); continue; }
                }
                
                // 4) 支配所有使用（含块内顺序）
                bool dom_uses = true;
                if (def != IR_VAR_NONE) {
                    for_set(IR_block_ptr, b_node, loop->body) {
                        for_list(IR_stmt_ptr, other_node, b_node->key->stmts) {
                            IR_stmt *user = other_node->val;
                            IR_use u = VCALL(*user, get_use_vec);
                            bool uses = false;
                            for (int k = 0; k < u.use_cnt; k++) if (u.use_vec[k].var == def) uses = true;
                            if (uses) {
                                /* TODO: 如果 b_node->key 不被 stmt_blk 支配，或者同块内 user 出现在 stmt 之前，则 dom_uses = false */
                            }
                        }
                        if (!dom_uses) break;
                    }
                }
                if (!dom_uses) { VCALL(to_remove, push_back, stmt); continue; }
                
                // 5) 入口活跃性检查（Live-In）
                if (def != IR_VAR_NONE) {
                    bool live_in = false;
                    RD_Fact header_in = VCALL(*(Map_IR_block_ptr_RD_Fact*)rd.mapInFact, get, loop->header);
                    /* TODO: 如果 header_in 中存在 def 且定义来自循环外，则 live_in = true */
                    if (live_in) { VCALL(to_remove, push_back, stmt); continue; }
                }
            }
            if (to_remove.len > 0) {
                changed = true;
                for (int k = 0; k < to_remove.len; ++k)
                    VCALL(*loop->invariant_stmts, delete, to_remove.arr[k]);
            }
            Vec_IR_stmt_ptr_teardown(&to_remove);
        }
        
        // ---------- 步骤 C. 移动代码 ----------
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
        if (preheader) {
            Vec_IR_stmt_ptr to_move;
            Vec_IR_stmt_ptr_init(&to_move);
            // 按程序顺序收集要移动的语句
            for_list(IR_block_ptr, b_node, func->blocks) {
                if (VCALL(loop->body, exist, b_node->val)) {
                    for_list(IR_stmt_ptr, s_node, b_node->val->stmts) {
                        if (VCALL(*loop->invariant_stmts, exist, s_node->val)) {
                            VCALL(to_move, push_back, s_node->val);
                        }
                    }
                }
            }
            // 移动
            for (int k = 0; k < to_move.len; ++k) {
                IR_stmt *stmt = to_move.arr[k];
                IR_block *owner = VCALL(stmt_blk_map, get, stmt);
                /* TODO: 从 owner 的语句列表中删除 stmt */
                /* TODO: 将 stmt 插入到 preheader 的末尾（但要避开末尾的分支/返回语句） */
                VCALL(stmt_blk_map, set, stmt, preheader);
            }
            Vec_IR_stmt_ptr_teardown(&to_move);
        }
        
        DELETE(loop->invariant_stmts);
        Set_IR_block_ptr_teardown(&loop->body);
    }
    
    Vec_NaturalLoop_teardown(&loops);
    for_map(IR_block_ptr, Set_ptr_IR_block_ptr, i, dom) DELETE(i->val);
    Map_IR_block_ptr_Set_ptr_IR_block_ptr_teardown(&dom);
    rd.vTable->teardown(&rd);
    Map_IR_stmt_ptr_IR_block_ptr_teardown(&stmt_blk_map);
}