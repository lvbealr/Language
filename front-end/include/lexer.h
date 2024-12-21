#ifndef LEXER_H_
#define LEXER_H_

#include <cstdlib>

#include "bufferExtensions.h"
#include "buffer.h"
#include "AST.h"

enum parseError {
    NO_PARSE_ERRORS           = 0,
    TOKENS_BUFFER_BAD_POINTER = 1,
    BAD_ALLOCATION            = 2,
    STRING_BAD_POINTER        = 3,
    LINE_INDEX_BAD_POINTER    = 4,
    CHAR_INDEX_BAD_POINTER    = 5,
    SYNTAX_ERROR              = 6
};

union tokenValue {
    int    value;
    char  *pointer;
};

#define KEYWORD(...) 1 +
const size_t KEYWORD_NUMBER    =
                               #include "keywords.def"
                               0;
#undef KEYWORD

struct Keyword {
    size_t      keywordID     = {};
    const char *name          = {};
    nameType    type          = {};
    size_t      keywordLength = {};
};

struct Token {
    tokenValue data         = {};
    nameType   type         = {};
    int        line         = {};
    int        tokenIndex   = {};
};

// FUNCTION PROTOTYPES //
parseError initializeTokenBuffer  (Buffer<Keyword> *keywords, Buffer<char *> *buffer, Buffer<Token> *tokens, int tokensCount);

parseError initializeKeywordBuffer(Buffer<Keyword> *keywords, const size_t keywordsCount);
// FUNCTION PROTOTYPES //

#endif // LEXER_H_
