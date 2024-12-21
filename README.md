# Language

## Lexemes

| KEYWORD             | NUMBER | KEYWORD             | TYPE      |
| :-----------------: | :----: | :-----------------: | :-------: |
| IF                  | 11     |      **если**       | OPERATOR  |
| WHILE               | 12     |      **пока**       | OPERATOR  |
| ASSIGNMENT          | 13     |  **сел_по_статье**  | OPERATOR  |
|                     |        |                     |           |
| SIN                 | 21     |                     | OPERATOR  |
| COS                 | 22     |                     | OPERATOR  |
| FLOOR               | 23     |                     | OPERATOR  |
| ADD                 | 24     |                     | OPERATOR  |
| SUB                 | 25     |                     | OPERATOR  |
| MUL                 | 26     |                     | OPERATOR  |
| DIV                 | 27     |    **рассечь**      | OPERATOR  |
| DIFF                | 28     |     **штрих**       | OPERATOR  |
| SQRT                | 29     |    **сквирт**       | OPERATOR  |
|                     |        |                     |           |
| EQUAL               | 31     |  **корефанится_с**  | OPERATOR  |
| LESS                | 32     |   **сосет_у**       | OPERATOR  |
| GREATER             | 33     |    **петушит**      | OPERATOR  |
| LESS_OR_EQUAL       | 34     |  **почти_петушит**  | OPERATOR  |
| GREATER_OR_EQUAL    | 35     |  **почти_сосет_у**  | OPERATOR  |
| NOT_EQUAL           | 36     | **путает_рамсы_с**  | OPERATOR  |
| AND                 | 37     |      **и**          | OPERATOR  |
| OR                  | 38     |     **или**         | OPERATOR  |
| NOT                 | 39     |      **не**         | OPERATOR  |
|                     |        |                     |           |
| OPERATOR_SEPARATOR  | 41     |       **.**         | SEPARATOR |
| ARGUMENT_SEPARATOR  | 42     |       **,**         | SEPARATOR |
|                     |        |                     |           |
| NUMBER              | 51     |     **фраер**       | NAME_TYPE |
|                     |        |                     |           |
| IN                  | 61     | **зашел_в_хату**    | OPERATOR  |
| OUT                 | 62     |  **откинулся**      | OPERATOR  |
|                     |        |                     |           |
| RETURN              | 71     |  **мусорнулся**     | OPERATOR  |
| BREAK               | 72     |     **шухер**       | OPERATOR  |
| CONTINUE            | 73     |  **мою_парашу**     | OPERATOR  |
| ABORT               | 74     | **посадили_на_перо**| OPERATOR  |
|                     |        |                     |           |
| FUNCTION_DEFENITION | 81     |   **мотает_срок**   | SEPARATOR |
| LEFT_BRACKET        | 82     |      **(**          | SEPARATOR |
| RIGHT_BRACKET       | 83     |      **)**          | SEPARATOR |
| BLOCK_OPEN          | 84     | **пошел_раскумар**  | SEPARATOR |
| BLOCK_CLOSE         | 85     |    **торкнуло**     | SEPARATOR |
| CONDITION_SEPARATOR | 86     |                     | SEPARATOR |
| INITIAL_OPERATOR    | 87     |                     | SEPARATOR |
| FUNCTION_CALL       | 88     |   **блатной**       | SEPARATOR |

## Buffer Tokenization Example

```c
#include <cstdio>

int compare(int a, int b) {
    if (a > b) {
        return a;
    }

    if (b >= a) {
        return b;
    }

    return 0;
}

int main() {
    int x = 242;
    int y = 228;

    int result = compare(x, y);

    printf("%d", result);

    return 0;
}

```

```c
вор_в_законе тест

мотает_срок компаратор ( фраер икс , фраер игрек ) пошел_раскумар
    если ( икс петушит игрек ) ништяк
        пошел_раскумар
            мусорнулся икс .
        торкнуло

    если ( игрек почти_сосет_у икс ) ништяк
        пошел_раскумар
            мусорнулся игрек .
        торкнуло

    мусорнулся 0 .
торкнуло

мотает_срок мэйн () пошел_раскумар
    фраер икс сел_по_статье 242 .
    фраер игрек сел_по_статье 228 .

    фраер результирующий сел_по_статье блатной компаратор ( икс , игрек ) .

    откинулся результирующий .

    мусорнулся 0 .
торкнуло

```

```c
token [0 | SEPARATOR]: вор_в_законе
token [1 | NAME]: тест
token [2 | SEPARATOR]: мотает_срок
token [3 | NAME]: компаратор
token [4 | SEPARATOR]: (
token [5 | NAME_TYPE]: фраер
token [6 | NAME]: икс
token [7 | SEPARATOR]: ,
token [8 | NAME_TYPE]: фраер
token [9 | NAME]: игрек
token [10 | SEPARATOR]: )
token [11 | SEPARATOR]: пошел_раскумар
token [12 | OPERATOR]: если
token [13 | SEPARATOR]: (
token [14 | NAME]: икс
token [15 | OPERATOR]: петушит
token [16 | NAME]: игрек
token [17 | SEPARATOR]: )
token [18 | SEPARATOR]: ништяк
token [19 | SEPARATOR]: пошел_раскумар
token [20 | OPERATOR]: мусорнулся
token [21 | NAME]: икс
token [22 | SEPARATOR]: .
token [23 | SEPARATOR]: торкнуло
token [24 | OPERATOR]: если
token [25 | SEPARATOR]: (
token [26 | NAME]: игрек
token [27 | OPERATOR]: почти_сосет_у
token [28 | NAME]: икс
token [29 | SEPARATOR]: )
token [30 | SEPARATOR]: ништяк
token [31 | SEPARATOR]: пошел_раскумар
token [32 | OPERATOR]: мусорнулся
token [33 | NAME]: игрек
token [34 | SEPARATOR]: .
token [35 | SEPARATOR]: торкнуло
token [36 | OPERATOR]: мусорнулся
token [37 | NUMBER]: 0
token [38 | SEPARATOR]: .
token [39 | SEPARATOR]: торкнуло
token [40 | SEPARATOR]: мотает_срок
token [41 | NAME]: мэйн
token [42 | SEPARATOR]: (
token [43 | SEPARATOR]: )
token [44 | SEPARATOR]: пошел_раскумар
token [45 | NAME_TYPE]: фраер
token [46 | NAME]: икс
token [47 | OPERATOR]: сел_по_статье
token [48 | NUMBER]: 242
token [49 | SEPARATOR]: .
token [50 | NAME_TYPE]: фраер
token [51 | NAME]: игрек
token [52 | OPERATOR]: сел_по_статье
token [53 | NUMBER]: 228
token [54 | SEPARATOR]: .
token [55 | NAME_TYPE]: фраер
token [56 | NAME]: результирующий
token [57 | OPERATOR]: сел_по_статье
token [58 | SEPARATOR]: блатной
token [59 | NAME]: компаратор
token [60 | SEPARATOR]: (
token [61 | NAME]: икс
token [62 | SEPARATOR]: ,
token [63 | NAME]: игрек
token [64 | SEPARATOR]: )
token [65 | SEPARATOR]: .
token [66 | OPERATOR]: откинулся
token [67 | NAME]: результирующий
token [68 | SEPARATOR]: .
token [69 | OPERATOR]: мусорнулся
token [70 | NUMBER]: 0
token [71 | SEPARATOR]: .
token [72 | SEPARATOR]: торкнуло
```

