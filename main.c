#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_TOKENS 128
#define MAX_CMD_LEN 1024
#define RL_START "\001"
#define RL_END   "\002"

// Version 1.1
// 
// Github: KairaBegudiri
//
// Qyzyl Project
// https://repo.qyzyl.xyz/qpm/sources/

// ──────────────────────────────
void sigintHandler(int sig) {
    printf("\n(Type 'exit' to quit)\n");
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

void sigwinchHandler(int sig) {
    rl_on_new_line();
    rl_redisplay();
}

// ──────────────────────────────
void shorten_path(const char *input, char *output) {
    if (!input || !*input) { strcpy(output, ""); return; }

    char path[512];
    strncpy(path, input, sizeof(path));
    path[sizeof(path)-1] = '\0';

    char *token = strtok(path, "/");
    char parts[64][128];
    int count = 0;
    while (token && count < 64) {
        strncpy(parts[count++], token, sizeof(parts[0]));
        token = strtok(NULL, "/");
    }

    strcpy(output, "/");
    for (int i = 0; i < count; i++) {
        if (i < count - 1) {
            char abbr[3] = { parts[i][0], '\0' };
            strcat(output, abbr);
            strcat(output, "/");
        } else {
            strcat(output, parts[i]);
        }
    }
}

// ──────────────────────────────
char* get_prompt() {
    static char buf[512];
    char cwd[256], shortcwd[256], host[128];
    getcwd(cwd, sizeof(cwd));
    shorten_path(cwd, shortcwd);

    const char *mode = getenv("PROMPT_MODE");
    const char *display_path = (mode && strcmp(mode, "full") == 0) ? cwd : shortcwd;

    char *user = getenv("USER");
    gethostname(host, sizeof(host));

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char timebuf[16];
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm_info);

    snprintf(buf, sizeof(buf),
             RL_START "\033[1;36m" RL_END "[%s]" RL_START "\033[0m" RL_END
             RL_START "\033[1;32m" RL_END "[%s@%s]" RL_START "\033[0m" RL_END
             RL_START "\033[1;34m" RL_END ":[%s]" RL_START "\033[0m" RL_END " $ ",
             timebuf, user, host, display_path);

    return buf;
}

// ──────────────────────────────
void expand_vars(char *input, char *expanded) {
    char *p = input;
    while (*p) {
        if (*p == '$') {
            p++;
            char varname[128] = {0};
            int i = 0;
            while (*p && (isalnum(*p) || *p == '_')) varname[i++] = *p++;
            varname[i] = '\0';
            char *val = getenv(varname);
            if (val) strcat(expanded, val);
        } else {
            int len = strlen(expanded);
            expanded[len] = *p;
            expanded[len + 1] = '\0';
            p++;
        }
    }
}

// ──────────────────────────────
void expand_tilde(char *input, char *output) {
    const char *home = getenv("HOME");
    for (int i = 0; input[i]; i++) {
        if (input[i] == '~' && (i == 0 || isspace(input[i - 1]))) {
            strcat(output, home);
        } else {
            int len = strlen(output);
            output[len] = input[i];
            output[len + 1] = '\0';
        }
    }
}

// ──────────────────────────────
void parse(char *input, char **args) {
    int i = 0;
    args[i] = strtok(input, " ");
    while (args[i] != NULL && i < MAX_TOKENS - 1)
        args[++i] = strtok(NULL, " ");
    args[i] = NULL;
}

// ──────────────────────────────
int builtin_cd(char **args) {
    if (!args[1]) { fprintf(stderr, "cd: expected argument\n"); return 1; }
    if (chdir(args[1]) != 0) perror("cd");
    return 1;
}
int builtin_pwd() { char cwd[1024]; getcwd(cwd, sizeof(cwd)); printf("%s\n", cwd); return 1; }
int builtin_export(char **args) {
    if (!args[1]) { fprintf(stderr, "export: usage: export VAR=value\n"); return 1; }
    char *eq = strchr(args[1], '=');
    if (!eq) { fprintf(stderr, "export: invalid syntax\n"); return 1; }
    *eq = '\0'; setenv(args[1], eq + 1, 1); return 1;
}
int builtin_unset(char **args) {
    if (!args[1]) { fprintf(stderr, "unset: usage: unset VAR\n"); return 1; }
    unsetenv(args[1]); return 1;
}
int builtin_echo(char **args) {
    for (int i = 1; args[i]; i++) printf("%s ", args[i]);
    printf("\n"); return 1;
}
int execute_builtin(char **args) {
    if (!args[0]) return 1;
    if (!strcmp(args[0], "cd")) return builtin_cd(args);
    if (!strcmp(args[0], "pwd")) return builtin_pwd();
    if (!strcmp(args[0], "export")) return builtin_export(args);
    if (!strcmp(args[0], "unset")) return builtin_unset(args);
    if (!strcmp(args[0], "echo")) return builtin_echo(args);
    if (!strcmp(args[0], "exit")) exit(0);
    return 0;
}

// ──────────────────────────────
int redirect_before_builtin(char **args, int saved_fd[3]) {
    saved_fd[0] = dup(STDIN_FILENO);
    saved_fd[1] = dup(STDOUT_FILENO);
    saved_fd[2] = dup(STDERR_FILENO);
    for (int i = 0; args[i]; i++) {
        if (!strcmp(args[i], ">")) {
            int fd = open(args[i+1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO); close(fd); args[i]=NULL; break;
        } else if (!strcmp(args[i], ">>")) {
            int fd = open(args[i+1], O_WRONLY|O_CREAT|O_APPEND, 0644);
            dup2(fd, STDOUT_FILENO); close(fd); args[i]=NULL; break;
        } else if (!strcmp(args[i], "<")) {
            int fd = open(args[i+1], O_RDONLY);
            if (fd<0) perror("input file");
            dup2(fd, STDIN_FILENO); close(fd); args[i]=NULL; break;
        }
    }
    return 0;
}
void restore_stdio(int saved_fd[3]) {
    dup2(saved_fd[0], STDIN_FILENO);
    dup2(saved_fd[1], STDOUT_FILENO);
    dup2(saved_fd[2], STDERR_FILENO);
    close(saved_fd[0]); close(saved_fd[1]); close(saved_fd[2]);
}

// ──────────────────────────────
int has_pipe(char **args){ for(int i=0;args[i];i++) if(!strcmp(args[i],"|")) return i; return -1;}
void handle_redirection(char **args){redirect_before_builtin(args,(int[3]){0});}
int execute_external(char **args){
    int pipe_pos=has_pipe(args),status=0;
    if(pipe_pos!=-1){
        args[pipe_pos]=NULL; char **cmd1=args,**cmd2=&args[pipe_pos+1];
        int pipefd[2]; pipe(pipefd);
        pid_t p1=fork();
        if(p1==0){dup2(pipefd[1],STDOUT_FILENO);close(pipefd[0]);close(pipefd[1]);handle_redirection(cmd1);execvp(cmd1[0],cmd1);perror("execvp");exit(1);}
        pid_t p2=fork();
        if(p2==0){dup2(pipefd[0],STDIN_FILENO);close(pipefd[1]);close(pipefd[0]);handle_redirection(cmd2);execvp(cmd2[0],cmd2);perror("execvp");exit(1);}
        close(pipefd[0]);close(pipefd[1]);waitpid(p1,&status,0);waitpid(p2,&status,0);
    }else{
        pid_t pid=fork();
        if(pid==0){handle_redirection(args);execvp(args[0],args);perror("execvp");exit(EXIT_FAILURE);}
        else if(pid>0){waitpid(pid,&status,0);} else perror("fork");
    }
    return WEXITSTATUS(status);
}

// ──────────────────────────────
void split_by_delim(char *input,char **cmds,const char *delim){
    int i=0;cmds[i]=strtok(input,delim);
    while(cmds[i]!=NULL&&i<MAX_TOKENS-1)cmds[++i]=strtok(NULL,delim);
    cmds[i]=NULL;
}

// ──────────────────────────────
char** my_completion(const char* text,int start,int end){
    (void)start;(void)end;
    return rl_completion_matches(text, rl_filename_completion_function);
}

// ──────────────────────────────
int main(){
    signal(SIGINT,sigintHandler);
    signal(SIGWINCH,sigwinchHandler);
    rl_attempted_completion_function=my_completion;

    const char *home=getenv("HOME");
    char history_file[1024];
    snprintf(history_file,sizeof(history_file),"%s/.qyzylshell_history",home);
    read_history(history_file);

    char *input;
    char expanded[MAX_CMD_LEN];
    char tilde_expanded[MAX_CMD_LEN];
    char *args[MAX_TOKENS];
    char *chain_cmds[MAX_TOKENS];

    while(1){
        input=readline(get_prompt());
        if(!input)break;
        if(strlen(input)==0){free(input);continue;}
        add_history(input);

        expanded[0]='\0'; expand_vars(input,expanded);
        tilde_expanded[0]='\0'; expand_tilde(expanded,tilde_expanded);

        char *p=tilde_expanded; char op=0;
        while(*p){ if(!strncmp(p,"&&",2)){op='&';break;}
        if(!strncmp(p,"||",2)){op='|';break;}
        if(*p==';'){op=';';break;} p++; }

        int saved_fd[3];
        if(op){
            if(op=='&')split_by_delim(tilde_expanded,chain_cmds,"&&");
            else if(op=='|')split_by_delim(tilde_expanded,chain_cmds,"||");
            else if(op==';')split_by_delim(tilde_expanded,chain_cmds,";");
            int result=0;
            for(int i=0;chain_cmds[i];i++){
                parse(chain_cmds[i],args);
                redirect_before_builtin(args,saved_fd);
                int built=execute_builtin(args);
                restore_stdio(saved_fd);
                if(built)continue;
                result=execute_external(args);
                if(op=='&'&&result!=0)break;
                if(op=='|'&&result==0)break;
            }
        }else{
            parse(tilde_expanded,args);
            redirect_before_builtin(args,saved_fd);
            int built=execute_builtin(args);
            restore_stdio(saved_fd);
            if(!built)execute_external(args);
        }

        write_history(history_file);
        free(input);
    }
    write_history(history_file);
    return 0;
}
