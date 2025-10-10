
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARGS 64

// 🌙 Catppuccin Mocha palette
#define ROSEWATER "\033[38;2;245;224;220m"
#define MAUVE     "\033[38;2;203;166;247m"
#define BLUE      "\033[38;2;137;180;250m"
#define TEAL      "\033[38;2;148;226;213m"
#define GREEN     "\033[38;2;166;227;161m"
#define YELLOW    "\033[38;2;249;226;175m"
#define RED       "\033[38;2;243;139;168m"
#define TEXT      "\033[38;2;205;214;244m"
#define RESET     "\033[0m"

void show_banner(void) {
    printf(MAUVE "╔══════════════════════════════════════════════\n");
    printf("║ # \n");
    printf("║ # \n");
    printf("║ # " RESET "                  " MAUVE "QyzylShell v0.6 BETA" RESET "                  " MAUVE "\n");
    printf("║ # " RESET "            " TEXT "Catppuccin Mocha Edition ☕" RESET "            " MAUVE "\n");
    printf("║ # " RESET "  " TEAL "GitHub:" RESET " KairaBegudiri  " BLUE "Web:" RESET " qyzyl.xyz   " MAUVE "\n");
    printf("║ # \n");
    printf("║ # \n");
    printf(MAUVE "╚══════════════════════════════════════════════\n" RESET);
    printf("\n");
}


void parse_input(char *input, char **args) {
    for (int i = 0; i < MAX_ARGS; i++) {
        args[i] = strsep(&input, " ");
        if (args[i] == NULL) break;
        if (strlen(args[i]) == 0) i--;
    }
}

int handle_builtin(char **args) {
    if (args[0] == NULL) return 1;

    // exit
    if (strcmp(args[0], "exit") == 0) {
        printf(GREEN "Exiting QyzylShell... 👋\n" RESET);
        exit(0);
    }

    // cd
    else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) chdir(getenv("HOME"));
        else if (chdir(args[1]) != 0) perror(RED "cd error" RESET);
        return 1;
    }

    // clear (persistent banner)
    else if (strcmp(args[0], "clear") == 0) {
        printf("\033[3J\033[H\033[2J");
        fflush(stdout);
        show_banner();
        return 1;
    }

    // help
    else if (strcmp(args[0], "help") == 0) {
        printf(MAUVE "QyzylShell Built-in Commands:\n" RESET);
        printf(TEXT "  cd <dir>      " RESET ": Change directory\n");
        printf(TEXT "  clear         " RESET ": Clear screen (keeps banner)\n");
        printf(TEXT "  export VAR=val" RESET ": Set environment variable\n");
        printf(TEXT "  unset VAR     " RESET ": Remove environment variable\n");
        printf(TEXT "  help          " RESET ": Show this help\n");
        printf(TEXT "  history       " RESET ": Show command history\n");
        printf(TEXT "  version       " RESET ": Display version info\n");
        printf(TEXT "  exit          " RESET ": Exit QyzylShell\n");
        return 1;
    }

    // history
    else if (strcmp(args[0], "history") == 0) {
        HIST_ENTRY **the_list = history_list();
        if (the_list)
            for (int i = 0; the_list[i]; i++)
                printf(YELLOW "%3d  " TEXT "%s\n" RESET,
                       i + history_base, the_list[i]->line);
        return 1;
    }

    // version
    else if (strcmp(args[0], "version") == 0) {
        printf(GREEN "QyzylShell v0.6 — readline, export, and Catppuccin support\n" RESET);
        return 1;
    }

    // export
    else if (strcmp(args[0], "export") == 0) {
        if (args[1] == NULL) {
            extern char **environ;
            for (char **env = environ; *env != NULL; env++) {
                printf("%s\n", *env);
            }
        } else {
            char *name = strtok(args[1], "=");
            char *value = strtok(NULL, "");
            if (value == NULL) {
                fprintf(stderr, RED "Usage: export VAR=value\n" RESET);
            } else if (setenv(name, value, 1) != 0) {
                perror(RED "export failed" RESET);
            }
        }
        return 1;
    }

    // unset
    else if (strcmp(args[0], "unset") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, RED "Usage: unset VAR\n" RESET);
        } else if (unsetenv(args[1]) != 0) {
            perror(RED "unset failed" RESET);
        }
        return 1;
    }

    return 0; // not a built-in
}

void execute_command(char **args) {
    pid_t pid = fork();

    if (pid == 0) {
        if (execvp(args[0], args) == -1)
            fprintf(stderr, RED "Command not found: %s\n" RESET, args[0]);
        exit(EXIT_FAILURE);
    } 
    else if (pid > 0) {
        wait(NULL);
    } 
    else {
        perror(RED "fork error" RESET);
    }
}

int main(void) {
    char *input;
    char *args[MAX_ARGS];
    char cwd[256];
    char hostname[128];
    struct passwd *pw = getpwuid(getuid());
    char *username = pw->pw_name;
    gethostname(hostname, sizeof(hostname));

    using_history();
    show_banner();

    while (1) {
        getcwd(cwd, sizeof(cwd));

        char prompt[512];
        snprintf(prompt, sizeof(prompt),
                 MAUVE "%s" RESET "@" BLUE "%s" RESET ":" TEAL "%s" RESET TEXT "$ " RESET,
                 username, hostname, cwd);

        input = readline(prompt);
        if (!input) break;
        if (strlen(input) == 0) { free(input); continue; }

        add_history(input);
        parse_input(input, args);

        if (!handle_builtin(args))
            execute_command(args);

        free(input);
    }

    return 0;
}
