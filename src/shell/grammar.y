%token WORD

%start complete_cmd
%%

complete_cmd        : cmd
                    | cmd '&'
                    ;

cmd                 : pipeline
                    | simple_cmd
                    ;

pipeline            : cmd '|' simple_cmd
                    ;

io_file_redirect    : '<' filename
                    | '>' filename
                    ;

filename            : WORD
                    ;

simple_cmd          : cmd_prefix cmd_name cmd_suffix
                    | cmd_prefix cmd_name
                    | cmd_prefix
                    | cmd_name cmd_suffix
                    | cmd_name
                    ;

cmd_name            : WORD
                    ;

cmd_prefix          : io_file_redirect
                    | cmd_prefix io_file_redirect
                    ;

cmd_suffix          :            io_file_redirect
                    | cmd_suffix io_file_redirect
                    |            WORD
                    | cmd_suffix WORD
                    ;
