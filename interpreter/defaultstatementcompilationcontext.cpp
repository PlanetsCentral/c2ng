/**
  *  \file interpreter/defaultstatementcompilationcontext.cpp
  *  \brief Class interpreter::DefaultStatementCompilationContext
  */

#include "interpreter/defaultstatementcompilationcontext.hpp"

void
interpreter::DefaultStatementCompilationContext::compileBreak(BytecodeObject& bco) const
{
    defaultCompileBreak(bco);
}

void
interpreter::DefaultStatementCompilationContext::compileContinue(BytecodeObject& bco) const
{
    defaultCompileContinue(bco);
}

void
interpreter::DefaultStatementCompilationContext::compileCleanup(BytecodeObject& bco) const
{
    defaultCompileCleanup(bco);
}
