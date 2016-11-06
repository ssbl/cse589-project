#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "server.h"
#include "cmd.h"

char **
cmd_tokenize(char *inputline, struct table *table, struct servinfo *servinfo)
{
    assert(inputline);
    assert(table);
    assert(servinfo);

    int tok;
    static char *tokens[4];     /* return value */
    char *str, *token, *delim, *saveptr;

    for (tok = 0, str = inputline, delim = " \n"; ; tok++, str = NULL) {
        token = strtok_r(str, delim, &saveptr);
        if (!token)
            break;
        tokens[tok] = token;
    }

    if (!tok)
        return NULL;
    if (tok == 5) {
        fprintf(stderr, "too many arguments in command\n");
        return NULL;
    }

    return tokens;
}

/* int
 * main(void)
 * {
 *     struct table *table = table_init(2, 1);
 *     struct servinfo *servinfo = servinfo_init(1, 2, 2);
 * 
 *     char line[16] = "display";
 *     cmd_handler(line, table, servinfo);
 * 
 *     table_free(table);
 *     free(servinfo);
 * 
 *     return 0;
 * } */
