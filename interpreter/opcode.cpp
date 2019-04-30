/**
  *  \file interpreter/opcode.cpp
  *  \brief Class interpreter::Opcode
  */

#include "interpreter/opcode.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/staticassert.hpp"

/* An Opcode object should be 4 bytes.
   Nothing in the code relies on that (I hope).
   Anyway, this is a canary to tell us if a compiler starts to think otherwise.
   If this breaks, (a) remove this check to get it working again quickly,
   and (b) start investigating why it broke. */
static_assert(sizeof(interpreter::Opcode), 4);

namespace {
    // Names for enum Scope
    static const char*const SCOPES[]     = { "var", "loc", "top", "glob", "gvar", "lit", "int", "bool" };

    // Format placeholders for enum Scope
    static const char*const SCOPE_ARGS[] = { "%n",  "%L",  "%T",  "%G",   "%n",   "%l",  "%d",  "%d" };

    // Names for miXXXX
    static const char*const IMS[]        = { "call",  "load",  "store", "pop",          // Refuse nothing --> call anything
                                             "proc",  "pload", "pstore", "ppop",        // RefuseFunctions --> procedures
                                             "fcall", "func",  "fstore", "fpop" };      // RefuseProcedures --> functions

    // Names for enum Stack
    static const char*const STACK_OPS[]  = { "dup", "drop", "swap" };

    // Names for enum Special
    static const char*const SPECIALS[] = {
        "uncatch",
        "return\t%u",
        "with",
        "endwith",
        "firstindex",
        "nextindex",
        "endindex",
        "evals\t%u",
        "evalx",
        "defsub\t%n",
        "defshipp\t%n",
        "defplanetp\t%n",
        "load",
        "print",
        "addhook",
        "runhook",
        "throw",
        "terminate",
        "suspend",
        "newarray\t%u",
        "makelist\t%u",
        "newhash",
        "instance",
        "resizearray\t%u",
        "bind",
        "first",
        "next",
    };

    const char* formatEnum(uint8_t minor, afl::base::Memory<const char*const> values)
    {
        if (const char*const* p = values.at(minor)) {
            return *p;
        } else {
            return "?";
        }
    }

    const char* formatScope(uint8_t minor)
    {
        return formatEnum(minor, SCOPES);
    }

    const char* formatIM(uint8_t minor)
    {
        return formatEnum(minor, IMS);
    }
}

// Get template for disassembling this opcode.
String_t
interpreter::Opcode::getDisassemblyTemplate() const
{
    String_t tpl;
    switch (major) {
     case maPush:
        tpl += "push";
        tpl += formatScope(minor);
        tpl += "\t";
        tpl += formatEnum(minor, SCOPE_ARGS);
        break;
     case maBinary:
        tpl += "b";
        tpl += getBinaryName(minor);
        break;
     case maUnary:
        tpl += "u";
        tpl += getUnaryName(minor);
        break;
     case maTernary:
        tpl += "t";
        tpl += getTernaryName(minor);
        break;
     case maJump:
        {
            uint8_t flags = uint8_t(minor & ~jSymbolic);
            if (flags == 0) {
                tpl += "label";
            } else if (flags < jCatch) {
                tpl += "j";
                if ((flags & jAlways) != jAlways) {
                    if (flags & jIfTrue)
                        tpl += "t";
                    if (flags & jIfFalse)
                        tpl += "f";
                    if (flags & jIfEmpty)
                        tpl += "e";
                    if ((flags & jAlways) == 0)
                        tpl += "never"; /* might occur as jneverp */
                }
                if (flags & jPopAlways)
                    tpl += "p";
            } else if (flags == jCatch) {
                tpl += "catch";
            } else if (flags == jDecZero) {
                tpl += "jdz";
            } else {
                tpl += "junknown";
            }

            if (minor & jSymbolic)
                tpl += "\tsym%u";
            else
                tpl += "\t#%u";
        }
        break;
     case maIndirect:
        tpl += formatIM(minor);
        tpl += "ind\t%u";
        break;
     case maStack:
        tpl += formatEnum(minor, STACK_OPS);
        tpl += "\t%u";
        break;
     case maPop:
        tpl += "pop";
        tpl += formatScope(minor);
        tpl += "\t";
        tpl += formatEnum(minor, SCOPE_ARGS);
        break;
     case maStore:
        tpl += "store";
        tpl += formatScope(minor);
        tpl += "\t";
        tpl += formatEnum(minor, SCOPE_ARGS);
        break;
     case maMemref:
        tpl += formatIM(minor);
        tpl += "mem\t%n";
        break;
     case maDim:
        tpl += "dim";
        tpl += formatScope(minor);
        tpl += "\t%n";
        break;
     case maSpecial:
        tpl += "s";
        tpl += formatEnum(minor, SPECIALS);
        break;

     case maFusedUnary:
        tpl += "push";
        tpl += formatScope(minor);
        tpl += "(u)\t";
        tpl += formatEnum(minor, SCOPE_ARGS);
        break;

     case maFusedBinary:
        tpl += "push";
        tpl += formatScope(minor);
        tpl += "(b)\t";
        tpl += formatEnum(minor, SCOPE_ARGS);
        break;

     case maFusedComparison:
        tpl += "b";
        tpl += getBinaryName(minor);
        tpl += "(j)";
        break;

     case maFusedComparison2:
        tpl += "push";
        tpl += formatScope(minor);
        tpl += "(b,j)\t";
        tpl += formatEnum(minor, SCOPE_ARGS);
        break;

     case maInplaceUnary:
        tpl += "push";
        tpl += formatScope(minor);
        tpl += "(xu)\t";
        tpl += formatEnum(minor, SCOPE_ARGS);
        break;

     default:
        tpl += "unknown?\t%u";
        break;
    }
    return tpl;
}

// Get external "major" value.
uint8_t
interpreter::Opcode::getExternalMajor() const
{
    switch (major) {
     case maPush:
     case maBinary:
     case maUnary:
     case maTernary:
     case maJump:
     case maIndirect:
     case maStack:
     case maPop:
     case maStore:
     case maMemref:
     case maDim:
     case maSpecial:
        return major;

     case maFusedUnary:
     case maFusedBinary:
     case maFusedComparison2:
        return maPush;

     case maFusedComparison:
        return maBinary;

     case maInplaceUnary:
        return maPush;
    }
    return major;
}
