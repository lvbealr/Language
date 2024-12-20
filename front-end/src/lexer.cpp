#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <cmath>
#include <cinttypes>
#include <unistd.h>

#include "lexer.h"
#include "customWarning.h"
#include "colorPrint.h"
#include "bufferExtensions.h"
#include "buffer.h"

#define LINE_              buffer->data[lineIndex]
#define SYMBOL_(charIndex) string[*(charIndex)]

#define CREATE_TOKEN_(value, type)                                  \
    createToken(tokens, value, string, lineIndex, charIndex, type);

#define SCAN_DOUBLE_()                                              \
    sscanf(&(SYMBOL_(charIndex)), "%lg%n", &number, &numberLength); \
    CREATE_TOKEN_(tokenValue {.value = number}, NUMBER);            \
    (*charIndex) += numberLength;                                   \

#define SKIP_LATIN_()                                               \
    while (isalpha(SYMBOL_(charIndex))) {                           \
        ++(*charIndex);                                             \
    }                                                             

#define SKIP_CYRILLIC_() {                                          \
    while (isCyrillic(&(SYMBOL_(charIndex)), CYRILLIC)) {           \
        (*charIndex) += 2;                                          \
    }                                                               \
}

#define FREE_(ptr) \
    free(ptr);     \
    ptr = NULL;    

#define FIND_KEYWORD_(str)                                                                    \
    for (size_t index = 0; index < keywords->size; index++) {                                 \
        if (!strncmp(keywords->data[index].name, str, keywords->data[index].keywordLength)) { \
            if (isCyrillic(&(SYMBOL_(charIndex)), CYRILLIC)) {                                \
                (*charIndex) += 2;                                                            \
            }                                                                                 \
                                                                                              \
            else {                                                                            \
                (*charIndex) += 1;                                                            \
            }                                                                                 \
                                                                                              \
            FREE_(word);                                                                      \
            return keywords->data[index].type;                                                \
        }                                                                                     \
    }

static const size_t MAX_WORD_LENGTH = 100;

static parseError scanLexeme       (Buffer<Keyword> *keywords, char          *string,    Buffer<Token> *tokens,    int *lineIndex, int *charIndex);
static parseError createToken      (Buffer<Token>   *tokens,   tokenValue     value,     char          *string,    int *lineIndex, int *charIndex, tokenType type);
static tokenType  getTokenType     (Buffer<Keyword> *keywords, char          *string,    int           *charIndex, int *startIndex);
static void       skipSpaces       (char            *string,   int           *pointer);
static int        isCyrillic       (char            *string,   tokenType      type);
static parseError writeSyntaxError (char            *string,   int           *lineIndex, int           *charIndex);

parseError initializeTokenBuffer(Buffer<Keyword> *keywords, Buffer<char *> *buffer, Buffer<Token> *tokens, int tokensCount) {
    customWarning(buffer   != NULL, (parseError) POINTER_IS_NULL);
    customWarning(tokens   != NULL,              TOKENS_BUFFER_BAD_POINTER);
    customWarning(keywords != NULL, (parseError) POINTER_IS_NULL);

    tokens->data = (Token *)calloc((size_t) tokensCount, sizeof(Token));
    customWarning(tokens->data != NULL, BAD_ALLOCATION);

    int charIndex = 0;

    for (int lineIndex = 0; lineIndex < buffer->size; lineIndex++) {
        charIndex = 0;
        while (LINE_[charIndex] != '\n' && LINE_[charIndex] != EOF) {
            scanLexeme(keywords, LINE_, tokens, &lineIndex, &charIndex);
        } 
    }

    return NO_PARSE_ERRORS;
}

parseError initializeKeywordBuffer(Buffer<Keyword> *keywords, const size_t keywordsCount) {
    customWarning(keywords != NULL, (parseError) POINTER_IS_NULL);

    keywords->data = (Keyword *)calloc(keywordsCount, sizeof(Keyword));
    customWarning(keywords->data != NULL, BAD_ALLOCATION);

    size_t keywordIndex = 0;

    #define KEYWORD(NAME, NUMBER, LEXEME, TYPE)                                                                             \
        keywords->data[keywordIndex++] = {.keywordID = NUMBER, .name = LEXEME, .type = TYPE, .keywordLength = strlen(LEXEME)};

    #include "keywords.def"

    #undef KEYWORD

    keywords->size = keywordsCount;

    return NO_PARSE_ERRORS;
}

static parseError scanLexeme(Buffer<Keyword> *keywords, char *string, Buffer<Token> *tokens, int *lineIndex, int *charIndex) {
    customWarning(keywords  != NULL, (parseError) POINTER_IS_NULL);
    customWarning(string    != NULL,              STRING_BAD_POINTER);
    customWarning(tokens    != NULL,              TOKENS_BUFFER_BAD_POINTER);
    customWarning(lineIndex != NULL,              LINE_INDEX_BAD_POINTER);
    customWarning(charIndex != NULL,              CHAR_INDEX_BAD_POINTER);

    skipSpaces(string, charIndex);
    
    double    number       = 0;
    int       numberLength = 0;

    int startIndex = *charIndex;

    tokenType type = getTokenType(keywords, string, charIndex, &startIndex);

    switch (type) {
        #define WORD_(type)                                                               \
            case type:                                                                    \
                {                                                                         \
                    CREATE_TOKEN_(tokenValue {.pointer = &(SYMBOL_(&startIndex))}, type); \
                    break;                                                                \
                }

        WORD_(LATIN);
        WORD_(CYRILLIC);
        WORD_(OPERATOR);
        WORD_(SEPARATOR);
        WORD_(NAME_TYPE);
        WORD_(NAME);
        
        #undef WORD_

		case NUMBER:
			{
				SCAN_DOUBLE_();
				break;
			}
		
		case UNDEFINED:
		default:
			{
				writeSyntaxError(string, lineIndex, &startIndex);
				exit(-1);
			}

	}

    return NO_PARSE_ERRORS;
}

static void skipSpaces(char *string, int *pointer) {
    customWarning(string  != NULL, (void) POINTER_IS_NULL);
    customWarning(pointer != NULL, (void) POINTER_IS_NULL);

    while (string[*pointer] == ' ') {
        ++(*pointer);
    }
}

static tokenType getTokenType(Buffer<Keyword> *keywords, char *string, int *charIndex, int *startIndex) {

    if (!keywords || !string || !charIndex) {
        return UNDEFINED;
    }
    
    char *word = (char *)calloc(MAX_WORD_LENGTH + 1, sizeof(char));

    if (!word) {
        return UNDEFINED;
    }

    skipSpaces(string, charIndex);

    if (ispunct(SYMBOL_(charIndex))) {
        (*startIndex) = (*charIndex);
        FIND_KEYWORD_(&(SYMBOL_(charIndex)));

        return UNDEFINED;
    }

    if (isdigit(SYMBOL_(charIndex))) {
        return NUMBER;
    }

    size_t letterNumber = 0;
    
    (*startIndex) = (*charIndex);

    while ((isalpha(SYMBOL_(charIndex)) || ispunct(SYMBOL_(charIndex)))) { // TODO check going beyond boundaries
        word[letterNumber]     = SYMBOL_(charIndex);
        word[letterNumber + 1] = '\0';

        FIND_KEYWORD_(word);

        ++(*charIndex);
        ++letterNumber;
        
        if ((isalpha(SYMBOL_(charIndex)) && ispunct(SYMBOL_(charIndex - 1))) || (isalpha(SYMBOL_(charIndex - 1)) && ispunct(SYMBOL_(charIndex)))) {
                break;
            }
    }

    letterNumber = 2;

    while ((isCyrillic(&(SYMBOL_(charIndex)), CYRILLIC) || ispunct(SYMBOL_(charIndex)))) { // TODO check going beyond boundaries
        memcpy(word, &(SYMBOL_(startIndex)), letterNumber + 1);
        word[letterNumber + 1] = '\0';
        
        FIND_KEYWORD_(word);

        if (isCyrillic(&SYMBOL_(charIndex), CYRILLIC)) {
            (*charIndex) += 2;
            letterNumber += 2;
        }

        if (ispunct(SYMBOL_(charIndex))) {
            (*charIndex) += 1;
            letterNumber += 1;
        }

        if (((isalpha(SYMBOL_(charIndex))     || isCyrillic(&(SYMBOL_(charIndex)),     CYRILLIC)) && ispunct(SYMBOL_(charIndex - 1))) ||
            ((isalpha(SYMBOL_(charIndex - 1)) || isCyrillic(&(SYMBOL_(charIndex - 1)), CYRILLIC)) && ispunct(SYMBOL_(charIndex)))) {
                break;
            } // TODO
    }

    return NAME; // ?
}


static int isCyrillic(char *string, tokenType type) {
	if ((*string & 0xe0) != 0xc0) {
		return 0;
  	}

  	if ((string[1] & 0xc0) != 0x80) {
    	return 0;
  	}

  	uint32_t uc = ((string[0] & 0x1f) << 6) | (string[1] & 0x3f); // 5 bits `s[0]` and 6 bits `s[1]`

  	if (uc < 0x400 || uc > 0x4ff) {
  	  return 0; // not cyrillic
  	}

  	if (type) {
		if ((uc < 0x410 || uc > 0x44f) && // А - Я; а - я
  	      !(uc == 0x401 || // Ё
  	        uc == 0x451)) {
  	    		return 0;
			} // ё
	}

  	return uc;
}

static parseError writeSyntaxError(char *string, int *lineIndex, int *charIndex) {
	customWarning(string    != NULL, (parseError) POINTER_IS_NULL);
	customWarning(lineIndex != NULL, (parseError) POINTER_IS_NULL);
	customWarning(charIndex != NULL, (parseError) POINTER_IS_NULL);

	customPrint(red, bold, bgDefault, "Syntax Error in line [%d], col [%d]!\n", *lineIndex, *charIndex); // TODO

	return SYNTAX_ERROR;
}

static parseError createToken(Buffer<Token> *tokens, tokenValue value, char *string, int *lineIndex, int *charIndex, tokenType type) {
    customWarning(tokens    != NULL,              TOKENS_BUFFER_BAD_POINTER);
	customWarning(string    != NULL, (parseError) POINTER_IS_NULL);
	customWarning(lineIndex != NULL, (parseError) POINTER_IS_NULL);
	customWarning(charIndex != NULL, (parseError) POINTER_IS_NULL);

    tokens->data[tokens->size].type = type;

    if (type == NUMBER) {
        tokens->data[tokens->size].data.value = value.value;
    }


    else {
        tokens->data[tokens->size].data.pointer = value.pointer;
    }

    tokens->data[tokens->size].tokenIndex = *charIndex;
    tokens->data[tokens->size].line       = *lineIndex;
    
    ++(tokens->size);

    return NO_PARSE_ERRORS;
}
