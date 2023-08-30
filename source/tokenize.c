bool parse_token(String_View *input, Token *token)
{
	*input = sv_trim_left(*input);
	
	if (input->count <= 0) return false;
	
	// Parse a sequence of digits as an integer
	if (isdigit(*input->data)) {
		token->type = Token_Integer;
		token->text = sv_chop_left_while(input, isnumber);
		return true;
	}
	
	// Parse a single-character token
	switch (*input->data) {
	case '.':
		token->type = Token_Dot;
		break;
	case ':':
		token->type = Token_Colon;
		break;
	case '(':
		token->type = Token_Open_Paren;
		break;
	case ')':
		token->type = Token_Close_Paren;
		break;
	case '\"':
		PANIC("strings are not implemented");
	default:
		// Parse a sequence of alpha-numeric characters as a word
		token->type = Token_Word;
		token->text = sv_chop_left_while(input, isident);	
		return true;
	}
	
	token->text = sv_chop_left(input, 1);
	return true;
}

inline bool isident(char x)
{
	return !isspace(x) && x != ':' && x != '(' && x != ')' && x != '.';
}

inline bool isnumber(char x)
{
	return x == '_' || isdigit(x);
}
