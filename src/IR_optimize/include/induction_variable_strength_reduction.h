#ifndef CODE_LOOP_INVARIANT_CODE_MOTION_H
#define CODE_LOOP_INVARIANT_CODE_MOTION_H

#include <IR_optimize.h>
#include <dataflow_analysis.h>

// Reaching Definitions Analysis
typedef struct ReachingDefinitions ReachingDefinitions;
struct ReachingDefinitions {
    struct ReachingDefinitions_virtualTable {
        void (*teardown) (ReachingDefinitions *t);
        bool (*isForward) (ReachingDefinitions *t);
        void *(*newBoundaryFact) (ReachingDefinitions *t, IR_function *func);
        void *(*newInitialFact) (ReachingDefinitions *t);
        void (*setInFact) (ReachingDefinitions *t, IR_block *blk, void *fact);
        void (*setOutFact) (ReachingDefinitions *t, IR_block *blk, void *fact);
        void *(*getInFact) (ReachingDefinitions *t, IR_block *blk);
        void *(*getOutFact) (ReachingDefinitions *t, IR_block *blk);
        bool (*meetInto) (ReachingDefinitions *t, void *fact, void *target);
        bool (*transferBlock) (ReachingDefinitions *t, IR_block *block, void *in_fact, void *out_fact);
        void (*printResult) (ReachingDefinitions *t, IR_function *func);
    } const *vTable;
    void *mapInFact; // Map<IR_block_ptr, RD_Fact> *
    void *mapOutFact; // Map<IR_block_ptr, RD_Fact> *
};

void ReachingDefinitions_init(ReachingDefinitions *t);

void InductionVariableStrengthReduction_optimize(IR_function *func);

#endif //CODE_LOOP_INVARIANT_CODE_MOTION_H
