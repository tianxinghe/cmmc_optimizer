#ifndef CODE_LOOP_INVARIANT_CODE_MOTION_H
#define CODE_LOOP_INVARIANT_CODE_MOTION_H

#include <IR_optimize.h>
#include <dataflow_analysis.h>

// Reaching Definitions Analysis
typedef struct LICM_ReachingDefinitions LICM_ReachingDefinitions;
struct LICM_ReachingDefinitions {
    struct LICM_ReachingDefinitions_virtualTable {
        void (*teardown) (LICM_ReachingDefinitions *t);
        bool (*isForward) (LICM_ReachingDefinitions *t);
        void *(*newBoundaryFact) (LICM_ReachingDefinitions *t, IR_function *func);
        void *(*newInitialFact) (LICM_ReachingDefinitions *t);
        void (*setInFact) (LICM_ReachingDefinitions *t, IR_block *blk, void *fact);
        void (*setOutFact) (LICM_ReachingDefinitions *t, IR_block *blk, void *fact);
        void *(*getInFact) (LICM_ReachingDefinitions *t, IR_block *blk);
        void *(*getOutFact) (LICM_ReachingDefinitions *t, IR_block *blk);
        bool (*meetInto) (LICM_ReachingDefinitions *t, void *fact, void *target);
        bool (*transferBlock) (LICM_ReachingDefinitions *t, IR_block *block, void *in_fact, void *out_fact);
        void (*printResult) (LICM_ReachingDefinitions *t, IR_function *func);
    } const *vTable;
    void *mapInFact; // Map<IR_block_ptr, RD_Fact> *
    void *mapOutFact; // Map<IR_block_ptr, RD_Fact> *
};

void LICM_ReachingDefinitions_init(LICM_ReachingDefinitions *t);

// LICM Optimization
void LoopInvariantCodeMotion_optimize(IR_function *func);

#endif //CODE_LOOP_INVARIANT_CODE_MOTION_H
