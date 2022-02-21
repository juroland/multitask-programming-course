#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <string>
#include <iostream>
#include <stdexcept>

#define MAXARGS 10

// All commands have at least a type. Have looked at the type, the code
// typically casts the *cmd to some specific cmd type.
struct Command {
    // Execute cmd, never returns
    virtual void run() = 0;
    virtual void run_as_background() = 0;
};

struct ExecCommand : Command {
    char *argv[MAXARGS] = {0};   // arguments to the command to be exec-ed

    void run() override
    {
        if(argv[0] == 0)
            exit(0);
        execvp(argv[0], argv);
    }

    void run_as_background() override {}
};

// '>' or '<' redirection
struct RedirCommand : Command {
    Command *cmd = nullptr;   // the command to be run (e.g., an execcmd)
    char *file = nullptr;     // the input/output file
    int mode = 0;             // the mode to open the file with
    int fd = 0;               // the file descriptor number to use for the file

    enum struct RedirType {
        Input,  // '<'
        Output  // '>'
    };

    RedirType type;

    void run() override
    {
        int file_fd = open(file, mode);
        dup2(file_fd, fd);
        cmd->run();
    }

    void run_as_background() override {}

    RedirCommand(Command *subcmd, char *file, RedirType type)
    {
        this->type = type;
        cmd = subcmd;
        this->file = file;
        if (type == RedirType::Input) {
            mode = O_RDONLY;
            fd = 0;
        } else {
            mode = O_WRONLY|O_CREAT|O_TRUNC;
            fd = 1;
        }
    }
};

struct PipeCommand : Command {
    Command *left = nullptr;  // left side of pipe
    Command *right = nullptr; // right side of pipe

    PipeCommand(Command *left, Command *right)
    {
        this->left = left;
        this->right = right;
    }

    void run() override
    {
        fprintf(stderr, "pipe not implemented\n");
    }

    void run_as_background() override {}
};

#define MAX_BACKGROUND_GRP 100
int pid_grp[MAX_BACKGROUND_GRP] = {0};

int fork1(void);  // Fork but exits on failure.
Command *parsecmd(char*);

int getcmd(char *buf, int nbuf)
{
    printf("$ ");
    memset(buf, 0, nbuf);
    fgets(buf, nbuf, stdin);
    if(buf[0] == 0) // EOF
        return -1;
    return 0;
}

struct Input {
private:
    std::string line;
    std::string::size_type pos;
    std::string::size_type start;

public:
    bool next_line()
    {
        bool not_eof = static_cast<bool>(std::getline(std::cin, line));
        if (not_eof) {
            start = pos = 0;
        }
        return not_eof;
    }

    /*bool is_prefix(const std::string& prefix)
    {
        return !eof() && line.compare(pos, prefix.size(), prefix) == 0;
    }*/

    bool eof()
    {
        return pos == line.size();
    }

    /*bool accept_prefix(const std::string& prefix)
    {
        if (is_prefix(prefix)) {
            pos += prefix.size();
            return true;
        } else {
            return false;
        }
    }*/

    bool accept(const std::string& symbols)
    {
        if (eof() || symbols.find(line[pos]) == std::string::npos) {
            return false;
        } else {
            ++pos;
            return true;
        }
    }

    const char* c_str()
    {
        return line.c_str() + pos;
    }

    bool accept_back(const char c)
    {
        if (!eof() && line.back() == c) {
            line.pop_back();
            return true;
        } else {
            return false;
        }
    }

    void backup()
    {
        if (pos > start)
            --pos;
        else
            throw std::logic_error{"backup from the beginning of input"};
    }

    void ignore()
    {
        start = pos;
    }

    char last_symbol()
    {
        char c = peek_last_symbol();
    }

    char peek_last_symbol()
    {
        while (!line.empty() && std::isspace(line.back()))
            line.pop_back();

        if (line.size() > pos)
            return '';
        else
            return '';
        static const std::string whitespaces = " \t\r\n\v";
    }

    std::string next_token()
    {
        std::string token = peek_next_token();
        start = pos;
        return token;
    }

    std::string peek_next_token()
    {
        static const std::string symbols_and_whitespaces = "<|> \t\r\n\v";
        static const std::string whitespaces = " \t\r\n\v";

        if (eof())
            return std::string{};

        switch (line[pos]) {
            case '|':
            case '<':
            case '>':
                ++pos;
                break;
            default:
                while(!accept("symbols_and_whitespaces")) ;
        }

        while (accept(whitespaces)) ;

        return std::string{line, start, pos - start};
    }
};

int main()
{
    Input input;
    while (input.next_line()) {
        if (input.peek_next_token() == "cd") {
            input.next_token();
            if(chdir(input.next_token().c_str()) < 0)
                fprintf(stderr, "cannot cd %s\n", input.c_str());
        } else {
            if (input.peek_last() == '&') {
                int pid = fork1();
                if(pid == 0) {
                    setpgid(0,0);
                    parsecmd(input)->run_as_background();
                }
            } else {
                int pid = fork1();
                if(pid == 0) {
                    setpgid(getpid(),getpid());
                    parsecmd(input)->run();
                }
                tcsetpgrp(0,pid);
                waitpid(pid, NULL, 0);
                tcsetpgrp(0,getpid());
            }
        }
    }

    return 0;
}

int fork1(void)
{
    int pid;
    pid = fork();

    if(pid == -1)
        perror("fork");
    return pid;
}

// Parsing

const char whitespace[] = " \t\r\n\v";
const char symbols[] = "<|>";

int gettoken(char **ps, char *es, char **q, char **eq)
{
    char *s;
    int ret;

    s = *ps;
    while(s < es && strchr(whitespace, *s))
        s++;
    if(q)
        *q = s;
    ret = *s;
    switch(*s){
        case 0:
            break;
        case '|':
        case '<':
            s++;
            break;
        case '>':
            s++;
            break;
        default:
            ret = 'a';
            while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
                s++;
            break;
    }
    if(eq)
        *eq = s;

    while(s < es && strchr(whitespace, *s))
        s++;
    *ps = s;
    return ret;
}

// ps : pointer to a string
// es : pointer to the end of the string
// moves *ps to the next character, returns true if this character is in the
// string toks
int peek(char **ps, char *es, char *toks)
{
    char *s;

    s = *ps;
    while(s < es && strchr(whitespace, *s))
        s++;
    *ps = s;
    return *s && strchr(toks, *s);
}

Command *parseline(char**, char*);
Command *parsepipe(char**, char*);
Command *parseexec(char**, char*);

// make a copy of the characters in the input buffer, starting from s through es.
// null-terminate the copy to make it a string.
char *mkcopy(char *s, char *es)
{
    int n = es - s;
    char *c = malloc(n+1);
    assert(c);
    strncpy(c, s, n);
    c[n] = 0;
    return c;
}

Command* parsecmd(Input& input)
{
    char *es;
    struct Command *cmd;

    cmd = parseline(input);
    peek(&s, es, "");
    if(s != es){
        fprintf(stderr, "leftovers: %s\n", s);
        exit(-1);
    }
    return cmd;
}

Command* parseline(Input& input)
{
    return parsepipe(input);
}

Command* parsepipe(Input& input)
{
    Command *cmd = parseexec(ps, es);
    if(peek(ps, es, "|")){
        gettoken(ps, es, 0, 0);
        cmd = new PipeCommand(cmd, parsepipe(ps, es));
    }
    return cmd;
}

Command* parseredirs(Command *cmd, Input& input)
{
    int tok;
    char *q, *eq;

    while(input.accept("<>")){
        tok = gettoken(ps, es, 0, 0);
        if(gettoken(ps, es, &q, &eq) != 'a') {
            fprintf(stderr, "missing file for redirection\n");
            exit(-1);
        }
        switch(tok){
            case '<':
                cmd = new RedirCommand(cmd, mkcopy(q, eq), RedirCommand::RedirType::Input);
                break;
            case '>':
                cmd = new RedirCommand(cmd, mkcopy(q, eq), RedirCommand::RedirType::Output);
                break;
        }
    }
    return cmd;
}

Command* parseexec(Input& input)
{
    char *q, *eq;
    int tok, argc;
    ExecCommand *cmd;
    Command *ret;

    ret = new ExecCommand();
    cmd = (struct ExecCommand*)ret;

    argc = 0;
    ret = parseredirs(ret, ps, es);
    while(!peek(ps, es, "|")){
        if((tok=gettoken(ps, es, &q, &eq)) == 0)
            break;
        if(tok != 'a') {
            fprintf(stderr, "syntax error\n");
            exit(-1);
        }
        cmd->argv[argc] = mkcopy(q, eq);
        argc++;
        if(argc >= MAXARGS) {
            fprintf(stderr, "too many args\n");
            exit(-1);
        }
        ret = parseredirs(ret, ps, es);
    }
    cmd->argv[argc] = 0;
    return ret;
}
