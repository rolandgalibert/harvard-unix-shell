#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<signal.h>
#include	<sys/wait.h>
#include        "controlflow.h"
#include	"smsh.h"
#include	"splitline.h"
#include	"varlib.h"
#include	<fcntl.h>
/**
 **	small-shell version 4.2
 **		first really useful version after prompting shell
 **		this one parses the command line into strings
 **		uses fork, exec, wait, and ignores signals
 **		It also uses the flexstr library to handle
 **		a the command line and list of args
 **/

#define	DFL_PROMPT	"> "

int init_smsh(int, char **, FILE **, char **);

int main(int argc, char *argv[])
{
	char	*cmdline, *prompt, **arglist;
	int	result, process(char **);
	void	setup();
	FILE 	*input_file;

	if (init_smsh(argc, argv, &input_file, &prompt) == -1) {
		exit(EXIT_FAILURE);
	}
	setup();
	
	while ( (cmdline = next_cmd(prompt, input_file)) != NULL )
	{
		if ( (arglist = splitline(cmdline)) != NULL  ){
			result = process(arglist);
			freelist(arglist);
		}
		free(cmdline);
	}
	if (if_struct != (struct if_struct *) NULL) {
                fprintf(stderr, "smsh: unexpected EOF\n");
        }
	return 0;
}

int init_smsh(int argc, char *argv[], FILE **input_file, char **prompt)
{
	if (argc == 1) {
		*input_file = stdin;
		*prompt = DFL_PROMPT;
	} else if (argc == 2) {
		if ((*input_file = fopen(argv[1], "r")) == NULL) {
			perror("smsh: function init_smsh: error opening file");
			return -1;
		}
		*prompt = "";
	} else {
		fprintf(stderr, "smsh: incorrect number of arguments\n");
		return -1;
	}
	return 1;
}

void setup()
/*
 * purpose: initialize shell
 * returns: nothing. calls fatal() if trouble
 */
{
	extern char **environ;

	VLenviron2table(environ);
	signal(SIGINT,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
        if_struct = (struct if_struct *) NULL;
}

void fatal(char *s1, char *s2, int n)
{
	fprintf(stderr,"Error: %s,%s\n", s1, s2);
	exit(n);
}
