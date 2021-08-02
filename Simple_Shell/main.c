#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

// Output colors
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void Skip_Space(char* s){
	if(s[strlen(s)-1]==' ' || s[strlen(s)-1]=='\n') // Whitespace at end
        s[strlen(s)-1]='\0';
	if(s[0]==' ' || s[0]=='\n') // Whitespace at first
        memmove(s, s+1, strlen(s));
}
void Take_block(char** param,int *n,char *s,const char *c){ // RETURN: param >> array of strings with size n
	char *token;
	token=strtok(s,c); //breaks string into a series of tokens
	int count=-1; // the end of string at size-1
	while(token){
		param[++count]=malloc(sizeof(token)+1);
		strcpy(param[count],token);
		Skip_Space(param[count]); // remove Whitespace
		token=strtok(NULL,c);
	}
	param[++count]=NULL;
	*n=count;
}
void executeBasic(char** arr){ // single external command
    int x = fork();
	if(x>0){ //parent
		wait(x);
	}
	else{
		execvp(arr[0],arr); //child
		//if exec is not successfull, exit
		perror(ANSI_COLOR_RED   "\t\t\t\tInvalid input"   ANSI_COLOR_RESET "\n"); //print an error
		exit(1);
	}
}
void executeAsync(char** s,int n){ // a series of external command run Asyncronously
	int i,count;
	char *arr[100];
	int x = fork();
	for(i=0;i<n;i++){
		Take_block(arr,&count,s[i]," ");
		if(x==0){
			execvp(arr[0],arr);
			perror("invalid input "); //print an error
			exit(1);//in case exec is not successfull, exit
		}
	}
	for(i=0;i<n;i++){
		wait(x);
	}
}
void check(){ // create file and write in it
       FILE *fp;
        fp = fopen("log.txt", "a");
        wait(-1);
        fprintf(fp, "Child process was terminated\n");
        fclose(fp);
}
int main(char** arr,int argc){
    signal(SIGCHLD,check); //1
    remove("log.txt"); // remove old file
	char s[500];
	char *block[100];
	char *params1[100];
	char *token;
	char cwd[1024];
	int n=0;

	printf(ANSI_COLOR_GREEN  "\t\t\t*----------Start----------*"   ANSI_COLOR_RESET "\n");

	while(1){ //2
		printf(ANSI_COLOR_CYAN   "Enter command(or 'exit' to exit):"   ANSI_COLOR_RESET "\n");
		//print current Directory
		if (getcwd(cwd, sizeof(cwd)) != NULL)
		printf(ANSI_COLOR_GREEN "%s  " ANSI_COLOR_RESET, cwd);
		else 	perror("getcwd failed\n");

		//read user input
		fgets(s, 500, stdin);//check overflow

     if(strchr(s,'&')){ // command with argument
			Take_block(block,&n,s,"&");
			executeAsync(block,n);
		}
		else{ // command without argument | exit | invalid
			Take_block(params1,&n,s," ");
			if(strstr(params1[0],"cd")){ //cd builtin command
				chdir(params1[1]);
			}
			else if(strstr(params1[0],"exit")){//exit builtin command
				exit(0);
			}
			else executeBasic(params1);
		}
	}
	return 0;
}
