//
// Created by hby on 22-12-3.
//

#ifndef CODE_AVAILABLE_EXPRESSIONS_ANALYSIS_H
#define CODE_AVAILABLE_EXPRESSIONS_ANALYSIS_H

#include <dataflow_analysis.h>

typedef struct {
    IR_OP_TYPE op;
    IR_val rs1, rs2;
} Expr;
extern int Expr_CMP(Expr a, Expr b);
DEF_MAP_CMP(Expr, IR_var, Expr_CMP)

typedef struct {
    bool is_top;
    Set_IR_var set;
} Fact_set_var, *Fact_set_var_ptr;
extern void Fact_set_var_init(Fact_set_var *fact, bool is_top);
extern void Fact_set_var_teardown(Fact_set_var *fact);


DEF_MAP(IR_block_ptr, Fact_set_var_ptr)

typedef Vec_IR_var *Vec_ptr_IR_var;
DEF_MAP(IR_var, Vec_ptr_IR_var)

typedef struct AvailableExpressionsAnalysis AvailableExpressionsAnalysis;
typedef struct AvailableExpressionsAnalysis {
    struct AvailableExpressionsAnalysis_virtualTable {
        void (*teardown) (AvailableExpressionsAnalysis *t);
        bool (*isForward) (AvailableExpressionsAnalysis *t);
        Fact_set_var *(*newBoundaryFact) (AvailableExpressionsAnalysis *t, IR_function *func);
        Fact_set_var *(*newInitialFact) (AvailableExpressionsAnalysis *t);
        void (*setInFact) (AvailableExpressionsAnalysis *t, IR_block *blk, Fact_set_var *fact);
        void (*setOutFact) (AvailableExpressionsAnalysis *t, IR_block *blk, Fact_set_var *fact);
        Fact_set_var *(*getInFact) (AvailableExpressionsAnalysis *t, IR_block *blk);
        Fact_set_var *(*getOutFact) (AvailableExpressionsAnalysis *t, IR_block *blk);
        bool (*meetInto) (AvailableExpressionsAnalysis *t, Fact_set_var *fact, Fact_set_var *target);
        bool (*transferBlock) (AvailableExpressionsAnalysis *t, IR_block *block, Fact_set_var *in_fact, Fact_set_var *out_fact);
    } const *vTable;
    Map_Expr_IR_var mapExpr;
    Map_IR_var_Vec_ptr_IR_var map_e_fill;
    Map_IR_block_ptr_Fact_set_var_ptr mapInFact, mapOutFact;
} AvailableExpressionsAnalysis;
extern void AvailableExpressionsAnalysis_init(AvailableExpressionsAnalysis *t);
extern void AvailableExpressionsAnalysis_merge_common_expr(AvailableExpressionsAnalysis *t,
                                                             IR_function *func);
extern void AvailableExpressionsAnalysis_transferStmt (AvailableExpressionsAnalysis *t,
                                                       IR_stmt *stmt,
                                                       Fact_set_var *fact);
extern bool AvailableExpressionsAnalysis_transferBlock (AvailableExpressionsAnalysis *t,
                                                        IR_block *block,
                                                        Fact_set_var *in_fact,
                                                        Fact_set_var *out_fact);
extern void AvailableExpressionsAnalysis_print_result (AvailableExpressionsAnalysis *t, IR_function *func);
extern void AvailableExpressionsAnalysis_remove_available_expr_def (AvailableExpressionsAnalysis *t, IR_function *func);

#endif //CODE_AVAILABLE_EXPRESSIONS_ANALYSIS_H
