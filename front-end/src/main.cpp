#include <cstdio>
#include <cctype>

#include "lexer.h"

int main() {
    Buffer<char>   code   = {};
    Buffer<char *> lines  = {};
    Buffer<Token>  tokens = {};

    readFile(&code, "test.txt");
    getLinePointerFromFile(&lines, &code);

    Buffer<Keyword> keywords = {};
    initializeKeywordBuffer(&keywords, KEYWORD_NUMBER);

    initializeTokenBuffer(&keywords, &lines, &tokens, code.size);

    printf("TOKENS SIZE: %d\n", tokens.size);

    customPrint(red, bold, bgDefault, "=====================================\n");
    customPrint(red, bold, bgDefault, "=====================================\n");

    char s[50] = {};

    static const char *array[] = {"LATIN", "CYRILLIC", "OPERATOR", "SEPARATOR", "NUMBER", "NAME_TYPE", "NAME", "UNDEFINED"};

    for (size_t index = 0; index < tokens.size; index++) {
        if (tokens.data[index].type == NUMBER) {
            customPrint(green, bold, bgDefault, "token [%d | %s]: %lg\n", index, array[tokens.data[index].type], tokens.data[index].data.value);
        }

        else if (ispunct(*tokens.data[index].data.pointer)) {
            sscanf(tokens.data[index].data.pointer, "%1s", s);
            customPrint(blue, bold, bgDefault, "token [%d | %s]: %s\n", index, array[tokens.data[index].type], s);
        }

        else {
            sscanf(tokens.data[index].data.pointer, "%s", s);
            customPrint(red, bold, bgDefault, "token [%d | %s]: %s\n", index, array[tokens.data[index].type], s);
        }
    }
}
