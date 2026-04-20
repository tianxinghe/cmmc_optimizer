#ifndef CODE_INDUCTION_VARIABLE_STRENGTH_REDUCTION_H
#define CODE_INDUCTION_VARIABLE_STRENGTH_REDUCTION_H

#include <IR_optimize.h>
#include <dataflow_analysis.h>

// Reaching Definitions Analysis
typedef struct IVSR_ReachingDefinitions IVSR_ReachingDefinitions;
struct IVSR_ReachingDefinitions {
    struct IVSR_ReachingDefinitions_virtualTable {
        void (*teardown) (IVSR_ReachingDefinitions *t);
        bool (*isForward) (IVSR_ReachingDefinitions *t);
        void *(*newBoundaryFact) (IVSR_ReachingDefinitions *t, IR_function *func);
        void *(*newInitialFact) (IVSR_ReachingDefinitions *t);
        void (*setInFact) (IVSR_ReachingDefinitions *t, IR_block *blk, void *fact);
        void (*setOutFact) (IVSR_ReachingDefinitions *t, IR_block *blk, void *fact);
        void *(*getInFact) (IVSR_ReachingDefinitions *t, IR_block *blk);
        void *(*getOutFact) (IVSR_ReachingDefinitions *t, IR_block *blk);
        bool (*meetInto) (IVSR_ReachingDefinitions *t, void *fact, void *target);
        bool (*transferBlock) (IVSR_ReachingDefinitions *t, IR_block *block, void *in_fact, void *out_fact);
        void (*printResult) (IVSR_ReachingDefinitions *t, IR_function *func);
    } const *vTable;
    void *mapInFact; // Map<IR_block_ptr, RD_Fact> *
    void *mapOutFact; // Map<IR_block_ptr, RD_Fact> *
};

void IVSR_ReachingDefinitions_init(IVSR_ReachingDefinitions *t);

void InductionVariableStrengthReduction_optimize(IR_function *func);

#endif //CODE_INDUCTION_VARIABLE_STRENGTH_REDUCTION_H
