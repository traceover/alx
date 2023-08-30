#if !defined(PARSE_H_)
#define PARSE_H_

typedef enum Token_Type {
	Token_Invalid = 0,
	Token_Integer,
	Token_Word,
	Token_String,
	Token_Dot,
	Token_Colon,
	Token_Open_Paren,
	Token_Close_Paren,
	Count_Token_Kinds,
} Token_Type;

typedef struct Token {
	Token_Type type;
	String_View text;
	// int col, row;
} Token;

bool parse_token(String_View *input, Token *out);

bool isident(char x);
bool isnumber(char x);

#endif // PARSE_H_
