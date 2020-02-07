/* splitline.c - commmand reading and parsing functions for smsh
 *    
 *    char *next_cmd(char *prompt, FILE *fp) - get next command
 *    char **splitline(char *str);           - parse a string
 */

#include	<ctype.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"splitline.h"
#include	"smsh.h"
#include	"flexstr.h"
#include	"varlib.h"

enum states {NORMAL, READ_DOLLAR, READ_FIRST_VAR_CHAR, READ_BACKSLASH};
int state_normal(int, char, FLEXSTR *, FLEXSTR *);
int state_read_dollar(int, char, FLEXSTR *, FLEXSTR *);
int state_read_first_var_char(int, char, FLEXSTR *, FLEXSTR *);
int state_read_backslash(int, char, FLEXSTR *, FLEXSTR *);
int legit_var_first_char(char);
int legit_var_char(char);
                                
int state_normal(int state, char c, FLEXSTR *command, FLEXSTR *var)
{
        int new_state;
        switch (c) {
                case '\n':
                        new_state = -1;
                        break;
                case '\\':      /* skip backslash */
                        new_state = READ_BACKSLASH;
                        break;
                case '$':
                        fs_init(var, 0);
                        fs_addch(var, '$');
                        new_state = READ_DOLLAR;
                        break;
                default:
                        fs_addch(command, c);
                        new_state = NORMAL;     /* (no state change) */
                        break;
        }
        return new_state;
}

int state_read_dollar(int state, char c, FLEXSTR *command, FLEXSTR *var)
{
        int new_state;
        if (c == '\n') {
                fs_addch(command, '$');
                new_state = -1;
        } else if (legit_var_first_char(c)) {
                fs_addch(var, c);
                new_state = READ_FIRST_VAR_CHAR;
        } else {
                fs_addstr(command, fs_getstr(var));
                new_state = NORMAL;
        }
        return new_state;
}

int state_read_first_var_char(int state, char c, FLEXSTR *command, FLEXSTR *var)
{
        int new_state;
        if ((c == '\n') || !legit_var_char(c)) {
                char *strip_var = fs_getstr(var);               /* Strip initial $ before lookup */
                char *var_lookup = VLlookup(strip_var + 1);
                if (strcmp(var_lookup, "") != 0) {
                        fs_addstr(command, var_lookup);
                } else {
                        fs_addstr(command, fs_getstr(var));
                }
                if (c == '\n') {
                        new_state = -1;
                } else if (c == '$') {
                        fs_init(var, 0);
                        fs_addch(var, '$');
                        new_state = READ_DOLLAR;
                } else if (c == '\\') {
                        new_state = READ_BACKSLASH;
                } else {
                        fs_addch(command, c);
                        new_state = NORMAL;
                }
        } else {
                fs_addch(var, c);
                new_state = READ_FIRST_VAR_CHAR;
        }
        return new_state;
}

int state_read_backslash(int state, char c, FLEXSTR *command, FLEXSTR *var)
{
        int new_state;
        if (c == '\n') {
                new_state = -1;
        } else {
                fs_addch(command, c);
                new_state = NORMAL;
        }
        return new_state;
}

int legit_var_first_char(char c)
{
	return (isalpha(c) || (c == '_'));
}

int legit_var_char(char c)
{
	return (isalpha(c) || isdigit(c) || (c == '_'));
}

char * next_cmd(char *prompt, FILE *fp)
/*
 * purpose: read next command line from fp
 * returns: dynamically allocated string holding command line
 *  errors: NULL at EOF (not really an error)
 *          calls fatal from emalloc()
 *   notes: allocates space in BUFSIZ chunks.  
 */
{
	int	c;				/* input char		*/
	FLEXSTR	s;				/* the command		*/
	FLEXSTR var;				/* current shell variable, if any */
	int	pos = 0;
	int	state = NORMAL;
	fs_init(&s, 0);				/* initialize the str	*/
	
	printf("%s", prompt);				/* prompt user	*/
	while( ( c = getc(fp)) != EOF ) 
	{
                switch (state) {
                        case NORMAL:
                                state = state_normal(state, c, &s, &var);
                                break;
                        case READ_DOLLAR:
                                state = state_read_dollar(state, c, &s, &var);
                                break;
                        case READ_FIRST_VAR_CHAR:
                                state = state_read_first_var_char(state, c, &s, &var);
                                break;
                        case READ_BACKSLASH:
                                state = state_read_backslash(state, c, &s, &var);
                                break;
                }
                if (state  == -1) {
                        break;
                } else {
                        pos++;
                }

        }
        if (c == EOF) {
                if (pos == 0) {
                        return NULL;
                } else {
                        char *strip_var, *var_lookup;
                        switch (state) {
                                case READ_DOLLAR:
                                        fs_addch(&s, '$');
                                        break;
                                case READ_FIRST_VAR_CHAR:
                                        strip_var = fs_getstr(&var);
                                        var_lookup = VLlookup(strip_var + 1);
                                        if (strcmp(var_lookup, "") != 0) {
                                                fs_addstr(&s, var_lookup);
                                        } else {
                                                fs_addstr(&s, fs_getstr(&var));
                                        }
                                        break;
                        }
                }
        }
	fs_addch(&s, '\0');     		/* terminate string	*/
	return fs_getstr(&s);
}

/**
 **	splitline ( parse a line into an array of strings )
 **/
#define	is_delim(x) ((x)==' '||(x)=='\t')

char ** splitline(char *line)
/*
 * purpose: split a line into array of white-space separated tokens
 * returns: a NULL-terminated array of pointers to copies of the tokens
 *          or NULL if line is NULL.
 *          (If no tokens on the line, then the array returned by splitline
 *           contains only the terminating NULL.)
 *  action: traverse the array, locate strings, make copies
 *    note: strtok() could work, but we may want to add quotes later
 */
{
	char	*newstr();
	char	*cp = line;			/* pos in string	*/
	char	*start;
	int	len;
	FLEXLIST strings;

	if ( line == NULL )			/* handle special case	*/
		return NULL;

	fl_init(&strings,0);

	while( *cp != '\0' )
	{
		while ( is_delim(*cp) )		/* skip leading spaces	*/
			cp++;
		if ( *cp == '\0' )		/* end of string? 	*/
			break;			/* yes, get out		*/

		/* mark start, then find end of word */
		start = cp;
		len   = 1;
		while (*++cp != '\0' && !(is_delim(*cp)) )
			len++;
		fl_append(&strings, newstr(start, len));
	}
	fl_append(&strings, NULL);
	return fl_getlist(&strings);
}

/*
 * purpose: constructor for strings
 * returns: a string, never NULL
 */
char *newstr(char *s, int l)
{
	char *rv = emalloc(l+1);

	rv[l] = '\0';
	strncpy(rv, s, l);
	return rv;
}

void 
freelist(char **list)
/*
 * purpose: free the list returned by splitline
 * returns: nothing
 *  action: free all strings in list and then free the list
 */
{
	char	**cp = list;
	while( *cp )
		free(*cp++);
	free(list);
}

void * emalloc(size_t n)
{
	void *rv ;
	if ( (rv = malloc(n)) == NULL )
		fatal("out of memory","",1);
	return rv;
}
void * erealloc(void *p, size_t n)
{
	void *rv;
	if ( (rv = realloc(p,n)) == NULL )
		fatal("realloc() failed","",1);
	return rv;
}

