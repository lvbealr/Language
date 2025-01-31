#ifndef LEXER_H_
#define LEXER_H_

#include "core.h"

enum class stringIntersection {
    NO_MATCH   = 0,
    IS_BEGIN   = 1,
    FULL_MATCH = 2
};

enum class symbolGroup {
    ENGLISH    = 1 << 0,
    CYRILLIC   = 1 << 1,
    DIGIT      = 1 << 2,
    UNDERSCORE = 1 << 3,
    BRACKET    = 1 << 4,
    OPERATION  = 1 << 5,
    INVALID    = 1 << 6,
    SPACE      = 1 << 7,
    SEPARATOR  = 1 << 8
};

compilationError lexicalAnalysis(compilationContext *context);

#endif // CORE_H_
