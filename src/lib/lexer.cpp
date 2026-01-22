#include "lexer.h"

/* Constructor */
Lexer::Lexer(const std::string &input) : input_(input) {}

/* Destructor */
Lexer::~Lexer(void) {}

/* peek description */
char Lexer::peek(void) const {}

/* get description */
char Lexer::get(void) {}

/* skip withe space description */
void Lexer::skip_white_space(void) {}

/* read number description */
Token Lexer::read_number(void) {}

/* read identifier description */
Token Lexer::read_identifier(void) {}