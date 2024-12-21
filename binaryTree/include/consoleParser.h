#ifndef CONSOLE_PARSER_H_
#define CONSOLE_PARSER_H_

#include <cstdlib>
#include <getopt.h>

#include "customWarning.h"
#include "binaryTree.h"

#define NDEBUG

#define INIT_OPTION(optionsBuffer, flagName, hasArgValue, shortName, flagValue) { \
  customWarning(optionsCount < MAX_OPTIONS_COUNT, (void) 1);                      \
                                                                                  \
  optionsBuffer[optionsCount].name    = flagName;                                 \
  optionsBuffer[optionsCount].has_arg = hasArgValue;                              \
  optionsBuffer[optionsCount].val     = shortName;                                \
  optionsBuffer[optionsCount].flag    = flagValue;                                \
                                                                                  \
  optionsCount++;                                                                 \
}

#ifndef NDEBUG
  #define PRINT_OPTION() printf("Option: [-%c]\t %s\n", gotOption, optarg);
#else
  #define PRINT_OPTION()
#endif // _NDEBUG

static const size_t MAX_OPTIONS_COUNT = 1;

template<typename DT>
void parseConsole(int argc, char *argv[], binaryTree<DT> *tree) {
  size_t optionsCount = 0;
  option options[MAX_OPTIONS_COUNT]   = {};

  INIT_OPTION(options, "dumpFolder", 2, 'd', 0);

  int   gotOption    = 0;
  while ((gotOption = getopt_long(argc, argv, "2", options, NULL)) != -1) {
    PRINT_OPTION();

    switch (gotOption) {
      case 'd': {
        strncpy(tree->infoData->dumpFolderName, optarg, MAX_DUMP_FILE_NAME);
        break;
      }

      default: {
        break;
      }
    }
  }
}

#endif // CONSOLE_PARSER_H_
