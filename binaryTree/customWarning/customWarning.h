#ifndef CUSTOM_WARNING_H
#define CUSTOM_WARNING_H

#include <cstdio>

#define customWarning(expression, returnMessage) do {                      \
    if (!(expression)) {                                                   \
        printf("Assertion Error: %s, file (%s), function (%s), line %d\n", \
                #expression, __FILE__, __FUNCTION__, __LINE__);            \
        return returnMessage;                                              \
    }                                                                      \
} while (0)

#endif // CUSTOM_WARNING_H_
