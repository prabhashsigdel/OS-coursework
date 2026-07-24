#ifndef COMMON_H
#define COMMON_H

#define PORT 5050
#define BUF_SIZE 1024
#define MAX_CLIENTS 20

/* Simple line-based text protocol. Client sends one command per line,
   server replies with one line starting OK or ERR.
   Commands:
     LOGIN <user> <pass>   -> OK welcome / ERR bad login
     ECHO <text>           -> OK <text>
     TIME                  -> OK <server time>
     ADD <a> <b>           -> OK <sum>
     QUIT                  -> OK bye (server closes)
   Every command except LOGIN/QUIT needs a logged-in session. */

#endif
