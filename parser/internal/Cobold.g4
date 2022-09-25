grammar Cobold;
file: importDeclaration* functionDeclaration* EOF;

importDeclaration: 'import' StringConstant ';';
functionDeclaration:
	FUNCTION Identifier '(' argumentList? ')' (
		'->' typeSpecifier
	)? (compoundStatement | externSpecifier ';');
externSpecifier: '#' 'extern' '(' StringConstant ')';
argumentList: Identifier ':' typeSpecifier (',' argumentList)?;

compoundStatement: '{' blockItemList? '}';
blockItemList: blockItem+;
blockItem: statement | declaration;
statement:
	returnStatement
	| assignmentStatement
	| compoundStatement
	| expressionStatement
	| ifStatement
	| iterationStatement;

returnStatement: RETURN expression ';';
expressionStatement: expression ';';

ifStatement:
	IF (expression | '(' expression ')') compoundStatement (
		ELSE IF (expression | '(' expression ')') compoundStatement
	)* (ELSE compoundStatement)?;

iterationStatement: forStatement | whileStatement;
forStatement:
	FOR Identifier (':' typeSpecifier)? IN (
		expression
		| '(' expression ')'
	) compoundStatement;

whileStatement:
	WHILE (expression | '(' expression ')') compoundStatement;

declaration:
	(VAR | LET) Identifier (':' typeSpecifier)? (
		'=' (DASH_INIT | expression)
	)? ';';

primaryExpression:
	BoolConstant
	| CharConstant
	| StringConstant
	| IntegerConstant
	| FloatingConstant
	| Identifier;

postfixExpression: primaryExpression postfixOperations?;
postfixOperations: (
		'[' expression ']'
		| '(' argumentExpressionList? ')'
		| ('.' | '->') Identifier
		| ('++' | '--')
	) postfixOperations?;

argumentExpressionList:
	conditionalExpression (',' conditionalExpression)*;

unaryExpression:
	prefixOperator* (
		postfixExpression
		| unaryOperator castExpression
	);
prefixOperator: '++' | '--';
unaryOperator: '&' | '>>' | '+' | '-' | '~' | '!';

castExpression:
	'(' typeSpecifier ')' castExpression
	| unaryExpression;

multiplicativeExpression:
	castExpression (multiplicativeOperator castExpression)*;
multiplicativeOperator: '*' | '/' | '%';

additiveExpression:
	multiplicativeExpression (
		additiveOperator multiplicativeExpression
	)*;
additiveOperator: '+' | '-';

shiftExpression:
	additiveExpression (shiftOperator additiveExpression)*;
shiftOperator: '<<' | '>>';

relationalExpression:
	shiftExpression (relationalOperator shiftExpression)*;
relationalOperator: '<' | '>' | '<=' | '>=';

equalityExpression:
	relationalExpression (equalityOperator relationalExpression)*;
equalityOperator: '==' | '!=';

andExpression: equalityExpression ( '&' equalityExpression)*;

exclusiveOrExpression: andExpression ('^' andExpression)*;

inclusiveOrExpression:
	exclusiveOrExpression ('|' exclusiveOrExpression)*;

logicalAndExpression:
	inclusiveOrExpression ('&&' inclusiveOrExpression)*;

logicalOrExpression:
	logicalAndExpression ('||' logicalAndExpression)*;

conditionalExpression:
	logicalOrExpression (
		'?' expression ':' conditionalExpression
	)?;

expression:
	conditionalExpression
	| callExpression
	| rangeExpression
	| arrayExpression;
callExpression:
	Identifier '(' (expression (',' expression)*)? ')';
rangeExpression:
	LBRACKET leftExpression? '..' rightExpression? RBRACKET;
leftExpression: expression;
rightExpression: expression;
arrayExpression:
	LBRACKET (expression (',' expression)*)? RBRACKET;

FUNCTION: 'fn';
RETURN: 'return';
IF: 'if';
ELSE: 'else';
FOR: 'for';
WHILE: 'while';
IN: 'in';
VAR: 'var';
LET: 'let';

POINTER: '*';
LBRACKET: '[';
RBRACKET: ']';
DASH_INIT: '--';

// Types
STRING: 'string';
BOOL: 'bool';
CHAR: 'char';
NIL: 'nil';

// Integral Types
IntegralType: I8 | I16 | I32 | I64 | I128 | I256;
I8: 'i8';
I16: 'i16';
I32: 'i32';
I64: 'i64';
I128: 'i128';
I256: 'i256';

// Floating Types
FloatingType: F8 | F16 | F32 | F64 | F128 | F256;
F8: 'f8';
F16: 'f16';
F32: 'f32';
F64: 'f64';
F128: 'f128';
F256: 'f256';

assignmentStatement:
	expression assignmentOperator expression ';';
assignmentOperator:
	'='
	| '*='
	| '/='
	| '%='
	| '+='
	| '-='
	| '<<='
	| '>>='
	| '&='
	| '^='
	| '|=';

typeSpecifier:
	(IntegralType | FloatingType | STRING | CHAR | BOOL | NIL)
	| LBRACKET typeSpecifier RBRACKET
	| typeSpecifier POINTER;

BoolConstant: 'true' | 'false';

Identifier: Nondigit ( Nondigit | Digit)*;
fragment Nondigit: [a-zA-Z_];
fragment Digit: [0-9];

CharConstant: CHAR_LITERAL;
CHAR_LITERAL: ('\'' (~[\r\n'] | SimpleEscapeSequence)* '\'');

StringConstant: STRING_LITERAL;
STRING_LITERAL: ('"' (~[\r\n"] | SimpleEscapeSequence)* '"');
fragment SimpleEscapeSequence: '\\' ['"?abfnrtv\\];

FloatingConstant: Digit* '.' Digit+;

IntegerConstant:
	Sign? (
		DecimalConstant
		| OctalConstant
		| HexadecimalConstant
		| BinaryConstant
	);
fragment Sign: [+-];
fragment BinaryConstant: '0' [bB] [0-1]+;
fragment DecimalConstant: NonzeroDigit Digit*;
fragment OctalConstant: '0' OctalDigit*;
fragment HexadecimalConstant:
	HexadecimalPrefix HexadecimalDigit+;
fragment HexadecimalPrefix: '0' [xX];
fragment NonzeroDigit: [1-9];
fragment OctalDigit: [0-7];
fragment HexadecimalDigit: [0-9a-fA-F];

WS: [ \t\r\n]+ -> skip;

BlockComment: '/*' .*? '*/' -> skip;
LineComment: '//' ~[\r\n]* -> skip;