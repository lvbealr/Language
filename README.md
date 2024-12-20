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
token [0 | SEPARATOR]: вор_в_законе
token [1 | VAR_NAME]: тест
token [2 | SEPARATOR]: мотает_срок
token [3 | VAR_NAME]: компаратор
token [4 | SEPARATOR]: (
token [5 | NAME]: фраер
token [6 | VAR_NAME]: икс
token [7 | SEPARATOR]: ,
token [8 | NAME]: фраер
token [9 | VAR_NAME]: игрек
token [10 | SEPARATOR]: )
token [11 | SEPARATOR]: пошел_раскумар
token [12 | OPERATOR]: если
token [13 | SEPARATOR]: (
token [14 | VAR_NAME]: икс
token [15 | OPERATOR]: петушит
token [16 | VAR_NAME]: игрек
token [17 | SEPARATOR]: )
token [18 | SEPARATOR]: ништяк
token [19 | SEPARATOR]: пошел_раскумар
token [20 | OPERATOR]: мусорнулся
token [21 | VAR_NAME]: икс
token [22 | SEPARATOR]: .
token [23 | SEPARATOR]: торкнуло
token [24 | OPERATOR]: если
token [25 | SEPARATOR]: (
token [26 | VAR_NAME]: игрек
token [27 | OPERATOR]: почти_сосет_у
token [28 | VAR_NAME]: икс
token [29 | SEPARATOR]: )
token [30 | SEPARATOR]: ништяк
token [31 | SEPARATOR]: пошел_раскумар
token [32 | OPERATOR]: мусорнулся
token [33 | VAR_NAME]: игрек
token [34 | SEPARATOR]: .
token [35 | SEPARATOR]: торкнуло
token [36 | OPERATOR]: мусорнулся
token [37 | NUMBER]: 0
token [38 | SEPARATOR]: .
token [39 | SEPARATOR]: торкнуло
token [40 | SEPARATOR]: мотает_срок
token [41 | VAR_NAME]: мэйн
token [42 | SEPARATOR]: (
token [43 | SEPARATOR]: )
token [44 | SEPARATOR]: пошел_раскумар
token [45 | NAME]: фраер
token [46 | VAR_NAME]: икс
token [47 | OPERATOR]: сел_по_статье
token [48 | NUMBER]: 242
token [49 | SEPARATOR]: .
token [50 | NAME]: фраер
token [51 | VAR_NAME]: игрек
token [52 | OPERATOR]: сел_по_статье
token [53 | NUMBER]: 228
token [54 | SEPARATOR]: .
token [55 | NAME]: фраер
token [56 | VAR_NAME]: результирующий
token [57 | OPERATOR]: сел_по_статье
token [58 | SEPARATOR]: блатной
token [59 | VAR_NAME]: компаратор
token [60 | SEPARATOR]: (
token [61 | VAR_NAME]: икс
token [62 | SEPARATOR]: ,
token [63 | VAR_NAME]: игрек
token [64 | SEPARATOR]: )
token [65 | SEPARATOR]: .
token [66 | OPERATOR]: откинулся
token [67 | VAR_NAME]: результирующий
token [68 | SEPARATOR]: .
token [69 | OPERATOR]: мусорнулся
token [70 | NUMBER]: 0
token [71 | SEPARATOR]: .
token [72 | SEPARATOR]: торкнуло
```

