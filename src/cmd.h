/* cmd.h --- user command parser, handler
 */

#ifndef CMD_H_
#define CMD_H_


char **cmd_tokenize(char *inputline, struct table *table, struct servinfo *info);

#endif
