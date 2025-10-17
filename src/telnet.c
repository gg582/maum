#include "telnet.h"

#include <errno.h>

static void telnet_send_command(FILE *out, unsigned char command, unsigned char option)
{
    if (out == NULL) {
        return;
    }
    fputc(TELNET_IAC, out);
    fputc(command, out);
    fputc(option, out);
    fflush(out);
}

static int telnet_is_printable(int ch)
{
    unsigned char value = (unsigned char)ch;
    return value >= 0x20 || value == '\t';
}

static void telnet_echo(FILE *out, const char *data, size_t length)
{
    if (out == NULL || data == NULL || length == 0) {
        return;
    }
    fwrite(data, 1, length, out);
    fflush(out);
}

static void telnet_echo_char(FILE *out, unsigned char ch)
{
    if (out == NULL) {
        return;
    }
    fputc(ch, out);
    fflush(out);
}

static void telnet_handle_negotiation(FILE *out, unsigned char command, unsigned char option)
{
    if (out == NULL) {
        return;
    }

    switch (command) {
    case TELNET_DO:
        switch (option) {
        case TELNET_OPT_SUPPRESS_GO_AHEAD:
        case TELNET_OPT_BINARY:
        case TELNET_OPT_ECHO:
            telnet_send_command(out, TELNET_WILL, option);
            break;
        case TELNET_OPT_LINEMODE:
            telnet_send_command(out, TELNET_WONT, option);
            break;
        default:
            telnet_send_command(out, TELNET_WONT, option);
            break;
        }
        break;
    case TELNET_DONT:
        telnet_send_command(out, TELNET_WONT, option);
        break;
    case TELNET_WILL:
        switch (option) {
        case TELNET_OPT_SUPPRESS_GO_AHEAD:
        case TELNET_OPT_BINARY:
        case TELNET_OPT_NAWS:
            telnet_send_command(out, TELNET_DO, option);
            break;
        case TELNET_OPT_ECHO:
        case TELNET_OPT_LINEMODE:
        case TELNET_OPT_TERMINAL_TYPE:
        default:
            telnet_send_command(out, TELNET_DONT, option);
            break;
        }
        break;
    case TELNET_WONT:
        telnet_send_command(out, TELNET_DONT, option);
        break;
    default:
        break;
    }
}

static int telnet_skip_subnegotiation(FILE *in)
{
    int prev = 0;
    while (1) {
        int ch = fgetc(in);
        if (ch == EOF) {
            return -1;
        }
        if (prev == TELNET_IAC && ch == TELNET_SE) {
            return 0;
        }
        prev = ch;
    }
}

static int telnet_getc(FILE *in, FILE *out)
{
    while (1) {
        int ch = fgetc(in);
        if (ch == EOF) {
            return EOF;
        }
        if (ch != TELNET_IAC) {
            return ch;
        }

        int command = fgetc(in);
        if (command == EOF) {
            return EOF;
        }

        if (command == TELNET_IAC) {
            return TELNET_IAC;
        }

        if (command == TELNET_DO || command == TELNET_DONT || command == TELNET_WILL || command == TELNET_WONT) {
            int option = fgetc(in);
            if (option == EOF) {
                return EOF;
            }
            telnet_handle_negotiation(out, (unsigned char)command, (unsigned char)option);
            continue;
        }

        if (command == TELNET_SB) {
            if (telnet_skip_subnegotiation(in) != 0) {
                return EOF;
            }
            continue;
        }

        // Ignore other control commands such as NOP, DM, BRK, etc.
    }
}

void telnet_send_initial_negotiation(FILE *out)
{
    if (out == NULL) {
        return;
    }

    telnet_send_command(out, TELNET_WILL, TELNET_OPT_SUPPRESS_GO_AHEAD);
    telnet_send_command(out, TELNET_WILL, TELNET_OPT_BINARY);
    telnet_send_command(out, TELNET_WILL, TELNET_OPT_ECHO);
    telnet_send_command(out, TELNET_DONT, TELNET_OPT_LINEMODE);
    telnet_send_command(out, TELNET_DO, TELNET_OPT_SUPPRESS_GO_AHEAD);
    telnet_send_command(out, TELNET_DO, TELNET_OPT_BINARY);
    telnet_send_command(out, TELNET_DO, TELNET_OPT_NAWS);
}

int telnet_read_line(FILE *in, FILE *out, char *buffer, size_t size)
{
    if (in == NULL || buffer == NULL || size == 0) {
        errno = EINVAL;
        return -1;
    }

    size_t index = 0;
    int running = 1;
    while (running) {
        int ch = telnet_getc(in, out);
        if (ch == EOF) {
            if (index == 0) {
                return -1;
            }
            break;
        }

        if (ch == '\b' || ch == 0x7f) {
            if (index > 0) {
                index--;
                telnet_echo(out, "\b \b", 3);
            }
            continue;
        }

        if (ch == '\0') {
            continue;
        }

        if (ch == '\r') {
            int next = telnet_getc(in, out);
            telnet_echo(out, "\r\n", 2);
            if (next == '\n' || next == 0 || next == EOF) {
                running = 0;
            } else {
                if (next != EOF && next != '\r' && next != '\n') {
                    if (index + 1 < size) {
                        buffer[index++] = (char)next;
                        if (telnet_is_printable(next)) {
                            telnet_echo_char(out, (unsigned char)next);
                        }
                    }
                }
                running = 0;
            }
            continue;
        }

        if (ch == '\n') {
            telnet_echo(out, "\r\n", 2);
            break;
        }

        if (index + 1 < size) {
            buffer[index++] = (char)ch;
            if (telnet_is_printable(ch)) {
                telnet_echo_char(out, (unsigned char)ch);
            }
        } else {
            telnet_echo_char(out, '\a');
        }
    }

    buffer[index] = '\0';
    return 0;
}
