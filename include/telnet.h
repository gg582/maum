#ifndef TELNET_H
#define TELNET_H

#include <stdio.h>
#include <stddef.h>

#define TELNET_IAC 255
#define TELNET_DONT 254
#define TELNET_DO 253
#define TELNET_WONT 252
#define TELNET_WILL 251
#define TELNET_SB 250
#define TELNET_SE 240
#define TELNET_NOP 241
#define TELNET_DM 242
#define TELNET_BREAK 243
#define TELNET_IP 244
#define TELNET_AO 245
#define TELNET_AYT 246
#define TELNET_EC 247
#define TELNET_EL 248
#define TELNET_GA 249

#define TELNET_OPT_BINARY 0
#define TELNET_OPT_ECHO 1
#define TELNET_OPT_SUPPRESS_GO_AHEAD 3
#define TELNET_OPT_STATUS 5
#define TELNET_OPT_TIMING_MARK 6
#define TELNET_OPT_TERMINAL_TYPE 24
#define TELNET_OPT_NAWS 31
#define TELNET_OPT_TERMINAL_SPEED 32
#define TELNET_OPT_LINEMODE 34

void telnet_send_initial_negotiation(FILE *out);
int telnet_read_line(FILE *in, FILE *out, char *buffer, size_t size);

#endif // TELNET_H
