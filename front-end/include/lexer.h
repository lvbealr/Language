#ifndef LEXER_H_
#define LEXER_H_

#include <cstdlib>

#include "bufferExtensions.h"
#include "buffer.h"

enum parseError {
    NO_PARSE_ERRORS           = 0,
    TOKENS_BUFFER_BAD_POINTER = 1,
    BAD_ALLOCATION            = 2,
    STRING_BAD_POINTER        = 3,
    LINE_INDEX_BAD_POINTER    = 4,
    CHAR_INDEX_BAD_POINTER    = 5,
    SYNTAX_ERROR              = 6
};

enum tokenType {
    LATIN     = 0,
    CYRILLIC  = 1,
    OPERATOR  = 2,
    SEPARATOR = 3,
    NUMBER    = 4,
    NAME_TYPE = 5,
    NAME      = 6,
    UNDEFINED = 7
};

union tokenValue {
    double value;
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
    tokenType   type          = {};
    size_t      keywordLength = {};
};

struct Token {
    tokenValue data         = {};
    tokenType  type         = {};
    int        line         = {};
    int        tokenIndex   = {};
};

// FUNCTION PROTOTYPES //
parseError initializeTokenBuffer  (Buffer<Keyword> *keywords, Buffer<char *> *buffer, Buffer<Token> *tokens, int tokensCount);

parseError initializeKeywordBuffer(Buffer<Keyword> *keywords, const size_t keywordsCount);
// FUNCTION PROTOTYPES //

#endif // LEXER_H_
