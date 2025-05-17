#include <cctype>
#include <climits>
#include <clocale>

#include "lexer.h"
#include "buffer.h"
#include "core.h"
#include "nameTable.h"
#include "AST.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define currentSymbol context->fileContent[*currentIndex]

#define ADD_TOKEN(NODE) do {                                                                    \
    node<astNode> *newToken = NODE;                                                             \
    if (writeDataToBuffer(context->tokens, &newToken, 1) != bufferError::NO_BUFFER_ERROR) {     \
        return compilationError::TOKEN_BUFFER_ERROR;                                            \
    }                                                                                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

static compilationError tokenizeNumber   (compilationContext *context, size_t *currentIndex);
static compilationError tokenizeWord     (compilationContext *context, size_t *currentIndex);
static size_t           getNextWordLength(compilationContext *context, size_t  currentIndex);

static compilationError tokenizeNewIdentifier     (compilationContext *context, size_t *currentIndex, size_t length);
static compilationError tokenizeExistingIdentifier(compilationContext *context, size_t *currentIndex, size_t length, size_t nameIndex);

static stringIntersection checkWordIntersection(const char *word,            const char *pattern, size_t wordLength);
static stringIntersection findMaxIntersection  (compilationContext *context, const char *word,    size_t wordLength, size_t *foundNameIndex);
static symbolGroup        getSymbolGroup       (compilationContext *context, size_t      symbolIndex);

static symbolGroup        getPermittedSymbols(symbolGroup group);
static size_t             getMaxWordLength   (symbolGroup group);

static bool   isCyrillic      (const char *multiByteSymbol);
static size_t getMaxWordLength(char byte);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

compilationError lexicalAnalysis(compilationContext *context) {
    customWarning(context != NULL, compilationError::CONTEXT_ERROR);

    size_t currentIndex = 0;

    setlocale(LC_ALL, "en_US.utf8");

    while (currentIndex < context->fileSize) {
        if (context->fileContent[currentIndex] == '\n') {
            context->currentLine++;
        }

        if (isspace(context->fileContent[currentIndex])) {
            currentIndex++;
            continue;
        }

        if (isdigit(context->fileContent[currentIndex])) {
            tokenizeNumber(context, &currentIndex);
        }

        else {
            tokenizeWord(context, &currentIndex);
        }
    }

    ADD_TOKEN(_TERMINATOR_()); // ?

    return compilationError::NO_ERRORS;
}

static compilationError tokenizeNumber(compilationContext *context, size_t *currentIndex) {
    customWarning(context      != NULL, compilationError::CONTEXT_ERROR);
    customWarning(currentIndex != NULL, compilationError::CONTEXT_ERROR);

    int number       = NAN;
    int numberLength = 0;

    if (sscanf(&currentSymbol, "%d%n", &number, &numberLength) > 0) {
        node<astNode> *constToken = _CONST_(number);
        constToken->data.line = context->currentLine;

        ADD_TOKEN(constToken);

        (*currentIndex) += (size_t)numberLength;
        
        return compilationError::NO_ERRORS;
    }

    return compilationError::TOKEN_BUFFER_ERROR;
}

static compilationError tokenizeWord(compilationContext *context, size_t *currentIndex) {
    customWarning(context      != NULL, compilationError::CONTEXT_ERROR);
    customWarning(currentIndex != NULL, compilationError::CONTEXT_ERROR);

    size_t initialWordLength = 0;
    size_t wordLength        = 0;

    while (true) {
        size_t nextWordLength = getNextWordLength(context, *currentIndex + wordLength);

        if (initialWordLength == 0) {
            initialWordLength = nextWordLength;
        }

        if (nextWordLength == 0) {
            if (initialWordLength == 0) {
                return compilationError::IDENTIFIER_EXPECTED;
            }

            return tokenizeNewIdentifier(context, currentIndex, initialWordLength);
        }

        wordLength += nextWordLength;

        size_t foundNameIndex = 0;

        switch (findMaxIntersection(context, &currentSymbol, wordLength, &foundNameIndex)) {
            case stringIntersection::NO_MATCH: {
                return tokenizeNewIdentifier(context, currentIndex, initialWordLength);
            }

            case stringIntersection::FULL_MATCH: {
                return tokenizeExistingIdentifier(context, currentIndex, wordLength, foundNameIndex);
            }

            case stringIntersection::IS_BEGIN: {
                break;
            }
        }
    }
}

static compilationError tokenizeNewIdentifier(compilationContext *context, size_t *currentIndex, size_t length) {
    customWarning(context      != NULL, compilationError::CONTEXT_ERROR);
    customWarning(currentIndex != NULL, compilationError::CONTEXT_ERROR);

    char *newIdentifier = (char *)calloc(length + 1, sizeof(char));
    memcpy(newIdentifier, &currentSymbol, length);

    if (addIdentifier(context->nameTable, newIdentifier) != bufferError::NO_BUFFER_ERROR) {
        return compilationError::CONTEXT_ERROR;
    }

    node<astNode> *identifierToken = _NAME_(context->nameTable->currentIndex - 1);
    identifierToken->data.line = context->currentLine;

    ADD_TOKEN(identifierToken);

    (*currentIndex) += length;

    return compilationError::NO_ERRORS;
}

static compilationError tokenizeExistingIdentifier(compilationContext *context, size_t *currentIndex, size_t length, size_t nameIndex) {
    customWarning(context      != NULL, compilationError::CONTEXT_ERROR);
    customWarning(currentIndex != NULL, compilationError::CONTEXT_ERROR);

    node<astNode> *identifierToken = _NAME_(nameIndex);
    identifierToken->data.line = context->currentLine;

    ADD_TOKEN(identifierToken);

    (*currentIndex) += length;

    return compilationError::NO_ERRORS;
}

static size_t getNextWordLength(compilationContext *context, size_t currentIndex) {
    customWarning(context != NULL, -1); // TODO

    if (currentIndex > context->fileSize || context->fileContent[currentIndex] == '\n') {
        return 0;
    }

    symbolGroup currentGroup   = getSymbolGroup     (context, currentIndex);
    symbolGroup permittedGroup = getPermittedSymbols(currentGroup);

    size_t maxLength = getMaxWordLength(currentGroup);
    size_t length    = 0;

    while (context->fileContent[currentIndex + length] != '\n' &&
           ((int)currentGroup & (int)permittedGroup) && length < maxLength) {
                if (currentGroup == symbolGroup::CYRILLIC) {
                    length++;
                }

                length++;

                if (currentIndex + length < context->fileSize) {
                    currentGroup = getSymbolGroup(context, currentIndex + length);
                }

                else {
                    break;
                }
           }

    return length;
}

static stringIntersection findMaxIntersection(compilationContext *context, const char *word, size_t wordLength, size_t *foundNameIndex) {
    // customWarning(context,        compilationError::CONTEXT_ERROR); // TODO
    // customWarning(word,           compilationError::CONTEXT_ERROR); // TODO
    // customWarning(foundNameIndex, compilationError::CONTEXT_ERROR); // TODO

    stringIntersection maxIntersection = stringIntersection::NO_MATCH;

    for (size_t nameIndex = 0; nameIndex < context->nameTable->currentIndex; nameIndex++) {
        stringIntersection currentIntersection = checkWordIntersection(word, context->nameTable->data[nameIndex].name, wordLength);

        if (currentIntersection == stringIntersection::FULL_MATCH) {
            *foundNameIndex = nameIndex;

            return stringIntersection::FULL_MATCH;
        }

        if (maxIntersection == stringIntersection::NO_MATCH && currentIntersection == stringIntersection::IS_BEGIN) {
            maxIntersection = stringIntersection::IS_BEGIN;
        }
    }

    return maxIntersection;
}

static stringIntersection checkWordIntersection(const char *word, const char *pattern, size_t wordLength) {
    customWarning(word,    stringIntersection::NO_MATCH); // TODO
    customWarning(pattern, stringIntersection::NO_MATCH); // TODO

    for (size_t symbol = 0; symbol < wordLength; symbol++) {
        if (pattern[symbol] == '\0') {
            return stringIntersection::NO_MATCH;
        }

        if (pattern[symbol] != word[symbol]) {
            return stringIntersection::NO_MATCH;
        }
    }

    if (pattern[wordLength] == '\0') {
        return stringIntersection::FULL_MATCH;
    }

    return stringIntersection::IS_BEGIN;
}

static size_t getMaxWordLength(symbolGroup group) {
    switch (group) {
        case symbolGroup::ENGLISH:
        case symbolGroup::CYRILLIC:
        case symbolGroup::DIGIT:
        case symbolGroup::UNDERSCORE:
        case symbolGroup::SPACE:
            {
                return INT_MAX;
            }
        
        case symbolGroup::BRACKET:
        case symbolGroup::SEPARATOR:
            {
                return 1;
            }

        case symbolGroup::OPERATION:
            {
                return 2;
            }
        
        case symbolGroup::INVALID:
        default:
            {
                return 0;
            }
    }
}

static symbolGroup getPermittedSymbols(symbolGroup group) {
    switch (group) {
        case symbolGroup::ENGLISH:
        case symbolGroup::CYRILLIC:
        case symbolGroup::DIGIT:
        case symbolGroup::UNDERSCORE:
            {
                return (symbolGroup) ((int)symbolGroup::ENGLISH | (int)symbolGroup::CYRILLIC |
                                      (int)symbolGroup::DIGIT   | (int)symbolGroup::UNDERSCORE);
            }

        default:
            {
                return group;
            }
    }
}

static symbolGroup getSymbolGroup(compilationContext *context, size_t symbolIndex) {
    // customWarning(context, symbolGroup::INVALID); // TODO

    char symbol = context->fileContent[symbolIndex];

    if (isalpha(symbol)) {
        return symbolGroup::ENGLISH;
    }
    
    else if (isdigit(symbol)) {
        return symbolGroup::DIGIT;
    }

    else if (symbol == '{' || symbol == '}' ||
             symbol == '[' || symbol == ']' ||
             symbol == '(' || symbol == ')') {
                return symbolGroup::BRACKET;
             }
            
    else if (symbol == '+' || symbol == '-' ||
             symbol == '*' || symbol == '/' ||
             symbol == '=' || symbol == '!' ||
             symbol == '<' || symbol == '>') {
                return symbolGroup::OPERATION;
             }
    
    else if (isspace(symbol)) {
        return symbolGroup::SPACE;
    }

    else if (symbol == '_') {
        return symbolGroup::UNDERSCORE;
    }

    else if (symbol == '.' || symbol == ',') {
        return symbolGroup::SEPARATOR;
    }

    else if (isCyrillic(&context->fileContent[symbolIndex])) {
        return symbolGroup::CYRILLIC;
    }

    else {
        return symbolGroup::INVALID;
    }
}

static bool isCyrillicSecondByte(char byte) {
    return (byte >= 0x10 && byte <= 0x4f) || byte == 0x51 || byte == 0x01; // unicode table
}

static bool isCyrillic(const char *multiByteSymbol) {
    customWarning(multiByteSymbol != NULL, false);

    wchar_t symbol = 0;
    mbtowc(&symbol, multiByteSymbol, 2);

    char *symbolBytes = (char *)(&symbol);

    if (symbolBytes[1] == 0x04 && isCyrillicSecondByte(symbolBytes[0])) {
        return true;
    }

    return false;
}