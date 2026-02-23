#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum Token_Kind
{
	TOKEN_KEYWORD,
	TOKEN_IDENT,
	TOKEN_OPEN_PAREN,
	TOKEN_CLOSE_PAREN,
	TOKEN_ARROW,
	TOKEN_TYPE,
	TOKEN_OPEN_BRACE,
	TOKEN_CLOSE_BRACE,
	TOKEN_INTEGER,
	TOKEN_STRING,
	TOKEN_EOF,			// ADDED:end of file
	TOKEN_SYMBOL,		// ADDED
} Token_Kind;
// all possible categories of tokens produced by the lexer

typedef enum Keyword
{
	KEYWORD_FN,
	KEYWORD_LET,		// ADDED
	KEYWORD_SET,		// ADDED
	KEYWORD_IF,			// ADDED
	KEYWORD_WHILE,		// ADDED
	KEYWORD_CALL,		// ADDED
	KEYWORD_RETURN,
	KEYWORD_TRUE,		// ADDED
	KEYWORD_FALSE,		// ADDED
} Keyword;

typedef enum Type
{
	TYPE_INT,
	TYPE_BOOL,
	TYPE_STR,			// ADDED
} Type;

typedef struct String
{
	char *data;
	long count;
} String;
// Stores a pointer to characters and their length.

typedef struct Loc
{
	const char *file_name;
	long line;
	long col;
} Loc;
// Tracks the location of a token in the source file.

typedef struct Token
{
	Token_Kind kind;
	String source; // Identifier name is stored here
	Loc loc;
	
	union
	{
		long long_value;
		Keyword keyword;
		Type type;
	};
} Token;

typedef struct Token_Array
{
	Token *items;
	long count;
	long capacity;
} Token_Array;
// Resizable array used to collect tokens.

typedef struct Lexer
{
	String source;
	long index;
	Loc loc;
} Lexer;

void token_array_append(Token_Array *arr, Token token)
{
	// Implemented in the slides
	if (arr->count >= arr->capacity) {
		arr->capacity *= 2;
		if (arr->capacity == 0) arr->capacity = 16;
		arr->items = realloc(arr->items, arr->capacity*sizeof(*arr->items)); // reallocate memory
	}
	arr->items[arr->count++] = token;
}

// Functions to consider:
// void skip_whitespace()
// void skip_comments()
// Token next_token()

// ---- Helper: current char ----
static char lexer_current(Lexer *l)
{
	if (l->index >= l->source.count) return '\0';
	return l->source.data[l->index];
}

// ---- Helper: advance one char ----
static char lexer_advance(Lexer *l)
{
	char c = l->source.data[l->index++];
	if (c == '\n') {
		l->loc.line++;
		l->loc.col = 1;
	} else {
		l->loc.col++;
	}
	return c;
}

static void skip_whitespace(Lexer *l)
{
	while (l->index < l->source.count && isspace((unsigned char)lexer_current(l))) {
		lexer_advance(l);
	}
}

static void skip_comments(Lexer *l)
{
	if (l->index + 1 < l->source.count &&
	    l->source.data[l->index] == '/' &&
	    l->source.data[l->index + 1] == '/') {
		while (l->index < l->source.count && lexer_current(l) != '\n') {
			lexer_advance(l);
		}
	}
}

// ---- Check if word matches a keyword ----
static int match_keyword(const char *word, long len, Keyword *out)
{
	struct { const char *name; Keyword kw; } keywords[] = {
		{"fn",     KEYWORD_FN},
		{"let",    KEYWORD_LET},
		{"set",    KEYWORD_SET},
		{"if",     KEYWORD_IF},
		{"while",  KEYWORD_WHILE},
		{"call",   KEYWORD_CALL},
		{"return", KEYWORD_RETURN},
		{"true",   KEYWORD_TRUE},
		{"false",  KEYWORD_FALSE},
	};
	for (int i = 0; i < (int)(sizeof(keywords)/sizeof(keywords[0])); i++) {
		if ((long)strlen(keywords[i].name) == len &&
		    strncmp(keywords[i].name, word, len) == 0) {
			*out = keywords[i].kw;
			return 1;
		}
	}
	return 0;
}

// ---- Check if word matches a type ----
static int match_type(const char *word, long len, Type *out)
{
	if (len == 3 && strncmp(word, "int",  3) == 0) { *out = TYPE_INT;  return 1; }
	if (len == 4 && strncmp(word, "bool", 4) == 0) { *out = TYPE_BOOL; return 1; }
	if (len == 3 && strncmp(word, "str",  3) == 0) { *out = TYPE_STR;  return 1; }
	return 0;
}

static Token next_token(Lexer *l)
{
	// Skip whitespace and comments in a loop
	while (1) {
		skip_whitespace(l);
		long before = l->index;
		skip_comments(l);
		if (l->index == before) break;
	}

	Token t = {0};
	t.loc = l->loc;
	t.source.data = l->source.data + l->index;

	if (l->index >= l->source.count) {
		t.kind = TOKEN_EOF;
		t.source.count = 0;
		return t;
	}

	char c = lexer_current(l);

	// Integer literal
	if (isdigit((unsigned char)c)) {
		long start = l->index;
		while (l->index < l->source.count && isdigit((unsigned char)lexer_current(l))) {
			lexer_advance(l);
		}
		t.kind = TOKEN_INTEGER;
		t.source.count = l->index - start;
		// parse value
		char buf[64];
		long len = t.source.count < 63 ? t.source.count : 63;
		strncpy(buf, t.source.data, len);
		buf[len] = '\0';
		t.long_value = atol(buf);
		return t;
	}

	// Identifier or keyword or type
	if (isalpha((unsigned char)c) || c == '_') {
		long start = l->index;
		while (l->index < l->source.count &&
		       (isalnum((unsigned char)lexer_current(l)) || lexer_current(l) == '_')) {
			lexer_advance(l);
		}
		t.source.count = l->index - start;

		Keyword kw;
		Type ty;
		if (match_keyword(t.source.data, t.source.count, &kw)) {
			t.kind = TOKEN_KEYWORD;
			t.keyword = kw;
		} else if (match_type(t.source.data, t.source.count, &ty)) {
			t.kind = TOKEN_TYPE;
			t.type = ty;
		} else {
			t.kind = TOKEN_IDENT;
		}
		return t;
	}

	// String literal
	if (c == '"') {
		long start = l->index;
		lexer_advance(l); // consume opening "
		while (l->index < l->source.count && lexer_current(l) != '"') {
			if (lexer_current(l) == '\\') lexer_advance(l); // skip escape
			lexer_advance(l);
		}
		if (l->index < l->source.count) lexer_advance(l); // consume closing "
		t.kind = TOKEN_STRING;
		t.source.count = l->index - start;
		return t;
	}

	// Two-character symbols
	if (l->index + 1 < l->source.count) {
		char c2 = l->source.data[l->index + 1];
		if (c == '-' && c2 == '>') {
			t.kind = TOKEN_ARROW;
			t.source.count = 2;
			lexer_advance(l); lexer_advance(l);
			return t;
		}
		// Other two-char operators: ==, !=, <=, >=, &&, ||
		if ((c == '=' && c2 == '=') || (c == '!' && c2 == '=') ||
		    (c == '<' && c2 == '=') || (c == '>' && c2 == '=') ||
		    (c == '&' && c2 == '&') || (c == '|' && c2 == '|')) {
			t.kind = TOKEN_SYMBOL;
			t.source.count = 2;
			lexer_advance(l); lexer_advance(l);
			return t;
		}
	}

	// Single-character symbols
	lexer_advance(l);
	t.source.count = 1;
	switch (c) {
		case '(': t.kind = TOKEN_OPEN_PAREN;  break;
		case ')': t.kind = TOKEN_CLOSE_PAREN; break;
		case '{': t.kind = TOKEN_OPEN_BRACE;  break;
		case '}': t.kind = TOKEN_CLOSE_BRACE; break;
		default:  t.kind = TOKEN_SYMBOL;      break;
	}
	return t;
}

// TODO: Print Token_Array to check the output of your lexer
// ---- Print a single token ----
static void print_token(Token t)
{
	printf("%s:%ld:%ld ", t.loc.file_name, t.loc.line, t.loc.col);
	switch (t.kind) {
		case TOKEN_KEYWORD: {
			const char *names[] = {"fn","let","set","if","while","call","return","true","false"};
			printf("KEYWORD %s\n", names[t.keyword]);
			break;
		}
		case TOKEN_IDENT:
			printf("IDENTIFIER %.*s\n", (int)t.source.count, t.source.data);
			break;
		case TOKEN_OPEN_PAREN:  printf("(\n"); break;
		case TOKEN_CLOSE_PAREN: printf(")\n"); break;
		case TOKEN_ARROW:       printf("->\n"); break;
		case TOKEN_TYPE: {
			const char *names[] = {"int","bool","str"};
			printf("TYPE %s\n", names[t.type]);
			break;
		}
		case TOKEN_OPEN_BRACE:  printf("{\n"); break;
		case TOKEN_CLOSE_BRACE: printf("}\n"); break;
		case TOKEN_INTEGER:
			printf("INTEGER %ld\n", t.long_value);
			break;
		case TOKEN_STRING:
			printf("STRING %.*s\n", (int)t.source.count, t.source.data);
			break;
		case TOKEN_SYMBOL:
			printf("%.*s\n", (int)t.source.count, t.source.data);
			break;
		case TOKEN_EOF:
			printf("EOF\n");
			break;
	}
}

// ---- Print Token_Array ----
static void print_token_array(Token_Array arr)
{
	for (long i = 0; i < arr.count; i++) {
		print_token(arr.items[i]);
	}
}

Token_Array lex_file(const char *file_name)
{
	Token_Array result = {0};
	
	// TODO: Read the file into memory
	// Start skipping whitespace and comments
	// Find the next token in a loop
	// Push these onto result Token_Array

	// Read the file into memory
	FILE *f = fopen(file_name, "rb");
	if (!f) {
		fprintf(stderr, "Error: cannot open file '%s'\n", file_name);
		return result;
	}
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = malloc(size + 1);
	fread(buf, 1, size, f);
	buf[size] = '\0';
	fclose(f);

	Lexer l = {0};
	l.source.data  = buf;
	l.source.count = size;
	l.index        = 0;
	l.loc.file_name = file_name;
	l.loc.line     = 1;
	l.loc.col      = 1;

	// Lex tokens in a loop
	while (1) {
		Token t = next_token(&l);
		token_array_append(&result, t);
		if (t.kind == TOKEN_EOF) break;
	}

	return result;
}

// ---- Test driver ----
int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <source.jive>\n", argv[0]);
		return 1;
	}
	Token_Array tokens = lex_file(argv[1]);
	print_token_array(tokens);
	free(tokens.items);
	return 0;
}