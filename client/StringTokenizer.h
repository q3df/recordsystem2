#ifndef CLIENT_STRINGTOKENIZER_H_
#define CLIENT_STRINGTOKENIZER_H_

#include <list>
#include <string>

using namespace std;

#define	MAX_STRING_CHARS	1024
#define	MAX_STRING_TOKENS	1024
#define	MAX_TOKEN_CHARS		1024
#define	BIG_INFO_STRING		8192


class StringTokenizer {
private:
	unsigned int   cmd_argc;
	char  *cmd_argv[MAX_STRING_TOKENS];   // points into cmd_tokenized
	char  cmd_tokenized[BIG_INFO_STRING+MAX_STRING_TOKENS]; // will have 0 bytes inserted	
	char  cmd_cmd[BIG_INFO_STRING]; // the original command we received (no token processing)

public:
	inline StringTokenizer(const char *src, bool ignoreQuotes) {
		this->SetNewSourceString(src, ignoreQuotes);
	}

	inline ~StringTokenizer() {
	}

	inline void SetNewSourceString(const char *src, bool ignoreQuotes) {
		const char *text;
		char *textOut;

		// clear previous args
		cmd_argc = 0;

		if ( !src ) {
			return;
		}

		strncpy( cmd_cmd, src, sizeof(cmd_cmd) );

		text = cmd_cmd;
		textOut = cmd_tokenized;

		while ( 1 ) {
			if ( cmd_argc == MAX_STRING_TOKENS ) {
				return;			// this is usually something malicious
			}

			while ( 1 ) {
				// skip whitespace
				while ( *text && *text <= ' ' ) {
					text++;
				}
				if ( !*text ) {
					return;			// all tokens parsed
				}

				// skip // comments
				if ( text[0] == '/' && text[1] == '/' ) {
					return;			// all tokens parsed
				}

				// skip /* */ comments
				if ( text[0] == '/' && text[1] =='*' ) {
					while ( *text && ( text[0] != '*' || text[1] != '/' ) ) {
						text++;
					}
					if ( !*text ) {
						return;		// all tokens parsed
					}
					text += 2;
				} else {
					break;			// we are ready to parse a token
				}
			}

			// handle quoted strings
			// NOTE TTimo this doesn't handle \" escaping
			if ( !ignoreQuotes && *text == '"' ) {
				cmd_argv[cmd_argc] = textOut;
				cmd_argc++;
				text++;
				while ( *text && *text != '"' ) {
					*textOut++ = *text++;
				}
				*textOut++ = 0;
				if ( !*text ) {
					return;		// all tokens parsed
				}
				text++;
				continue;
			}

			// regular token
			cmd_argv[cmd_argc] = textOut;
			cmd_argc++;

			// skip until whitespace, quote, or command
			while ( *text > ' ' ) {
				if ( !ignoreQuotes && text[0] == '"' ) {
					break;
				}

				if ( text[0] == '/' && text[1] == '/' ) {
					break;
				}

				// skip /* */ comments
				if ( text[0] == '/' && text[1] =='*' ) {
					break;
				}

				*textOut++ = *text++;
			}

			*textOut++ = 0;

			if ( !*text ) {
				return;		// all tokens parsed
			}
		}
	}

	inline int Argc() {
		return cmd_argc;
	}

	const char *Argv(int index) {
		if ( (unsigned)index >= cmd_argc ) {
			return "";
		}

		return cmd_argv[index];
	}
};

#endif // CLIENT_STRINGTOKENIZER_H_

