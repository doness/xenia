/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2014 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/cpu/compiler/passes/finalization_pass.h"

#include "xenia/cpu/backend/backend.h"
#include "xenia/cpu/compiler/compiler.h"
#include "xenia/cpu/processor.h"
#include "xenia/profiling.h"

namespace xe {
namespace cpu {
namespace compiler {
namespace passes {

// TODO(benvanik): remove when enums redefined.
using namespace xe::cpu::hir;

using xe::cpu::hir::HIRBuilder;

FinalizationPass::FinalizationPass() : CompilerPass() {}

FinalizationPass::~FinalizationPass() {}

bool FinalizationPass::Run(HIRBuilder* builder) {
  // Process the HIR and prepare it for lowering.
  // After this is done the HIR should be ready for emitting.

  auto arena = builder->arena();

  uint16_t block_ordinal = 0;
  auto block = builder->first_block();
  while (block) {
    block->ordinal = block_ordinal++;

    // Ensure all labels have names.
    auto label = block->label_head;
    while (label) {
      if (!label->name) {
        const size_t label_len = 6 + 4 + 1;
        char* name = (char*)arena->Alloc(label_len);
        snprintf(name, label_len, "_label%d", label->id);
        label->name = name;
      }
      label = label->next;
    }

    // Remove unneeded jumps.
    auto tail = block->instr_tail;
    if (tail && tail->opcode == &OPCODE_BRANCH_info) {
      // Jump. Check target.
      auto target = tail->src1.label;
      if (target->block == block->next) {
        // Jumping to subsequent block. Remove.
        tail->Remove();
      }
    }

    block = block->next;
  }

  return true;
}

}  // namespace passes
}  // namespace compiler
}  // namespace cpu
}  // namespace xe
