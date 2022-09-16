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
	StringConstant
	| IntegerConstant
	| FloatingConstant
	| Identifier;

postfixExpression:
	primaryExpression (
		'[' expression ']'
		| '(' argumentExpressionList? ')'
		| ('.' | '->') Identifier
		| ('++' | '--')
	)*;

argumentExpressionList:
	conditionalExpression (',' conditionalExpression)*;

unaryExpression:
	('++' | '--')* (
		postfixExpression
		| unaryOperator castExpression
	);

unaryOperator: '&' | '>>' | '+' | '-' | '~' | '!';

castExpression:
	'(' typeSpecifier ')' castExpression
	| unaryExpression;

multiplicativeExpression:
	castExpression (('*' | '/' | '%') castExpression)*;

additiveExpression:
	multiplicativeExpression (
		('+' | '-') multiplicativeExpression
	)*;

shiftExpression:
	additiveExpression (('<<' | '>>') additiveExpression)*;

relationalExpression:
	shiftExpression (('<' | '>' | '<=' | '>=') shiftExpression)*;

equalityExpression:
	relationalExpression (('==' | '!=') relationalExpression)*;

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
rangeExpression: LBRACKET expression? '..' expression? RBRACKET;
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
I32: 'i32';
I64: 'i64';
STRING: 'string';
POINTER: '*';
LBRACKET: '[';
RBRACKET: ']';
DASH_INIT: '--';

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
	(I32 | I64 | STRING)
	| LBRACKET typeSpecifier RBRACKET
	| typeSpecifier POINTER;

Identifier: Nondigit ( Nondigit | Digit)*;
fragment Nondigit: [a-zA-Z_];
fragment Digit: [0-9];

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