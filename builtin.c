/* builtin.c
 * contains the switch and the functions for builtin commands
 */

#include        <errno.h>
#include        <limits.h>
#include	<stdio.h>
#include        <stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include        <sys/stat.h>
#include        <unistd.h>
#include	"smsh.h"
#include	"varlib.h"
#include        "flexstr.h"

int assign(char *);
int okname(char *);
void pwd_cmd(char **);
void cd_cmd(char **);
void exit_cmd(char **);
void read_cmd(char **);

int builtin_command(char **args, int *resultp)
/*
 * purpose: run a builtin command 
 * returns: 1 if args[0] is builtin, 0 if not
 * details: test args[0] against all known builtins.  Call functions
 */
{
	int rv = 0;

	if ( strcmp(args[0],"set") == 0 ){	     /* 'set' command? */
		VLlist();
		*resultp = 0;
		rv = 1;
	}
	else if ( strchr(args[0], '=') != NULL ){   /* assignment cmd */
		*resultp = assign(args[0]);
		if ( *resultp != -1 )		    /* x-y=123 not ok */
			rv = 1;
	}
	else if ( strcmp(args[0], "export") == 0 ){
		if ( args[1] != NULL && okname(args[1]) )
			*resultp = VLexport(args[1]);
		else
			*resultp = 1;
		rv = 1;
	}
        /* else if (strcmp(args[0], "pwd") == 0 ){
                pwd_cmd(args);
                rv = 1;
        } */
        else if ( strcmp(args[0], "cd") == 0 ){
                cd_cmd(args);
                rv = 1;
        }
        else if ( strcmp(args[0], "exit") == 0 ){
                exit_cmd(args);
                rv = 1;
        }
        else if ( strcmp(args[0], "read") == 0 ){
                read_cmd(args);
                rv = 1;
        }
        else if (args[0][0] == '#'){
                rv = 1;
        }
	return rv;
}

void read_cmd(char **args)
{
        char c;
        FLEXSTR line;
        fs_init(&line, 0);
        if (args[1] == NULL) {
                fprintf(stderr, "smsh: read: Read variable must be specified\n");
        } else {
                while ((c = getc(stdin)) != '\n') {
                        fs_addch(&line, c);
                }
                fs_addch(&line, '\0');
                VLstore(args[1], fs_getstr(&line));
        }
}

void exit_cmd(char **args)
{
        char *end_ptr;
        long value;
        if (args[1] != NULL) {
                value = strtol(args[1], &end_ptr, 10);
                if ((errno == ERANGE) || (end_ptr == args[1])) {
                        fprintf(stderr, "smsh: exit: Bad exit value %s specified\n", args[1]);
                        exit(-1);
                } else {
                        exit((int) value);
                }
        } else {
                exit(EXIT_SUCCESS);
        }
}

void pwd_cmd(char **args)
{
        printf("Entering pwd_cmd\n", args[0]);

        fprintf(stdout, "%s\n", VLlookup("PWD"));
}

void cd_cmd(char **args)
{

        if (args[1] == NULL) {
                chdir(VLlookup("HOME"));
        } else if (args[2] != NULL) {
        } else {
                chdir(args[1]);
        }
}
int assign(char *str)
/*
 * purpose: execute name=val AND ensure that name is legal
 * returns: -1 for illegal lval, or result of VLstore 
 * warning: modifies the string, but retores it to normal
 */
{
	char	*cp;
	int	rv ;

	cp = strchr(str,'=');
	*cp = '\0';
	rv = ( okname(str) ? VLstore(str,cp+1) : -1 );
	*cp = '=';
	return rv;
}
int okname(char *str)
/*
 * purpose: determines if a string is a legal variable name
 * returns: 0 for no, 1 for yes
 */
{
	char	*cp;

	for(cp = str; *cp; cp++ ){
		if ( (isdigit((int)*cp) && cp==str) || !(isalnum((int)*cp) || *cp=='_' ))
			return 0;
	}
	return ( cp != str );	/* no empty strings, either */
}

int oknamechar(char c, int pos)
/*
 * purpose: used to determine if a char is ok for a name
 */
{
	return ( (isalpha((int)c) || (c=='_' ) ) || ( pos>0 && isdigit((int)c) ) );
}
