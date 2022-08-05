/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

grammar SIEVEIR ;

// LEXER RULES
// ===========

NUMERIC_LITERAL : BINARY_LITERAL | OCTAL_LITERAL
                | DECIMAL_LITERAL | HEXADECIMAL_LITERAL ;

BINARY_LITERAL      : ( '0b' | '0B' ) ( '0' | '1' )+ ;
OCTAL_LITERAL       : '0o' [0-7]+ ;
DECIMAL_LITERAL     : '0' | ( [1-9][0-9]* ) ;
HEXADECIMAL_LITERAL : ( '0x' | '0X' ) [0-9a-fA-F]+ ;

OP_ADD : '+' ;
OP_SUB : '-' ;
OP_MUL : '*' ;
OP_DIV : '/' ;

RES_RELATION : 'relation' ;
RES_INSTANCE : 'instance' ;
RES_SHORT_WITNESS : 'short_witness' ;

XOR : '@xor' ;
AND : '@and' ;
NOT : '@not' ;

ADD : '@add' ;
MUL : '@mul' ;
ADDC : '@addc' ;
MULC : '@mulc' ;

INSTANCE : '@instance' ;
SHORT_WITNESS : '@short_witness' ;

FUNCTION : '@function' ;
FOR : '@for' ;
SWITCH : '@switch' ;

BOOLEAN : 'boolean' ;
ARITHMETIC : 'arithmetic' ;

LINE_COMMENT : '//' ~'\n'* '\n' -> skip ;
MULTI_COMMENT : '/*' .*? '*/' -> skip ;
WHITESPACE : [\t\r\n ] -> skip ;

WIRE_NUM : '$' NUMERIC_LITERAL ;
LABEL : [a-zA-Z_] [a-zA-Z0-9_]* ( ( '::' | '.' ) [a-zA-Z_] [a-zA-Z0-9_]* )* ;

// COMMON PARSER RULES
// ===================

fieldLiteral : '<' NUMERIC_LITERAL '>' ;

// HEADER PARSER RULES
// ===================

header : versionDecl fieldDecl ;

versionDecl : 'version' NUMERIC_LITERAL
                    '.' NUMERIC_LITERAL
                    '.' NUMERIC_LITERAL ';' ;

fieldDecl : 'field'
                'characteristic' NUMERIC_LITERAL
                'degree' NUMERIC_LITERAL ';' ;

// RESOURCE PARSER RULE
// ====================

resourceDecl : RES_RELATION | RES_INSTANCE | RES_SHORT_WITNESS ;

// IR PARAMETERS
// =============

arithmeticGateNames : ADD | ADDC | MUL | MULC ;
booleanGateNames : AND | NOT | XOR ;
gateSet : 'gate_set' ':'
            ( BOOLEAN
            | ARITHMETIC
            | arithmeticGateNames ( ',' arithmeticGateNames )*
            | booleanGateNames ( ',' booleanGateNames )* ) ';' ;

featureName : FUNCTION | FOR | SWITCH ;
featureToggles : 'features' ':'
                   ( featureName ( ',' featureName )*
                   | 'simple' ) ';' ;

// RELATION
// ========

relationBody : '@begin'
                 ( functionDeclare )*
                 directiveList
               '@end' ;

directiveList : ( directive )+ ;

directive : binaryGate
          | binaryConstGate
          | unaryGate
          | input
          | copy
          | assign
          | assertZero
          | deleteSingle
          | deleteRange
          | functionInvoke
          | anonFunction
          | forLoop
          | switchStatement ;

// SIMPLE GATES
// ============

binaryGate : WIRE_NUM '<-' binaryGateType '(' WIRE_NUM ',' WIRE_NUM ')' ';' ;
binaryConstGate : WIRE_NUM '<-' binaryConstGateType
                    '(' WIRE_NUM ',' fieldLiteral ')' ';' ;
unaryGate : WIRE_NUM '<-' unaryGateType '(' WIRE_NUM ')' ';' ;

binaryGateType : AND | XOR | ADD | MUL ;
binaryConstGateType : ADDC | MULC ;
unaryGateType : NOT ;

input : WIRE_NUM '<-' ( INSTANCE | SHORT_WITNESS ) ';' ;
copy : WIRE_NUM '<-' WIRE_NUM ';' ;
assign : WIRE_NUM '<-' fieldLiteral ';' ;
assertZero : '@assert_zero' '(' WIRE_NUM ')' ';' ;

deleteSingle : '@delete' '(' WIRE_NUM ')' ';' ;
deleteRange : '@delete' '(' WIRE_NUM ',' WIRE_NUM ')' ';' ;

// FUNCTION GATES
// ==============

functionDeclare : FUNCTION '(' LABEL ','
                    '@out' ':' NUMERIC_LITERAL ','
                    '@in' ':' NUMERIC_LITERAL ','
                    INSTANCE ':' NUMERIC_LITERAL ','
                    SHORT_WITNESS ':' NUMERIC_LITERAL
                  ')'
                    directiveList
                  '@end' ;

functionInvoke : ( outputs=wireList '<-' )?
                     '@call' '(' LABEL ( ',' inputs=wireList )? ')' ';' ;

wireList : wireListElement ( ',' wireListElement )* ;
wireListElement : wireListSingle | wireListRange ;
wireListSingle : WIRE_NUM ;
wireListRange : WIRE_NUM '...' WIRE_NUM ;

anonFunction : ( outputs=wireList '<-' )?
                 '@anon_call' '(' ( inputs=wireList ',' )?
                 INSTANCE ':' NUMERIC_LITERAL ','
                 SHORT_WITNESS ':' NUMERIC_LITERAL
               ')'
                 directiveList
               '@end' ;

// FOR LOOPS
// =========

forLoop : ( wireList '<-' )? FOR LABEL '@first' NUMERIC_LITERAL
                               '@last' NUMERIC_LITERAL
                               (iterExprFunctionInvoke | iterExprAnonFunction )
                             '@end' ;

// iterator expressions are slightly ambiguous with wire-nums. oops.
iterExpr : LABEL
         | NUMERIC_LITERAL
         | '(' iterExpr OP_ADD iterExpr ')'
         | '(' iterExpr OP_SUB iterExpr ')'
         | '(' iterExpr OP_MUL iterExpr ')'
         | '(' iterExpr OP_DIV NUMERIC_LITERAL ')' ;

iterExprWireNum : WIRE_NUM | '$' iterExpr ;

iterExprWireList : iterExprWireListElement ( ',' iterExprWireListElement )* ;
iterExprWireListElement : iterExprWireListSingle | iterExprWireListRange ;
iterExprWireListSingle : iterExprWireNum ;
iterExprWireListRange : iterExprWireNum  '...' iterExprWireNum ;

iterExprFunctionInvoke : ( outputs=iterExprWireList '<-' )? '@call' '(' LABEL
                           ( ',' inputs=iterExprWireList )? ')' ';' ;

iterExprAnonFunction : ( outputs=iterExprWireList '<-' )? '@anon_call' '('
                         ( inputs=iterExprWireList ',' )?
                         INSTANCE ':' NUMERIC_LITERAL ','
                         SHORT_WITNESS ':' NUMERIC_LITERAL
                       ')'
                         directiveList
                       '@end' ;

// SWITCH STATEMENTS
// =================

switchStatement : ( wireList '<-' )? SWITCH '(' WIRE_NUM ')'
                    ( '@case' fieldLiteral ':' caseFunction )+
                  '@end' ;

caseFunction : caseFunctionInvoke | caseAnonFunction ;

caseFunctionInvoke : '@call' '(' LABEL ( ',' wireList )? ')' ';' ;

caseAnonFunction : '@anon_call' '('
                     ( wireList ',' )?
                     INSTANCE ':' NUMERIC_LITERAL ','
                     SHORT_WITNESS ':' NUMERIC_LITERAL
                   ')'
                     directiveList
                   '@end' ;

// INSTANCE AND SHORT WITNESS
// ==========================

inputStream : '@begin' ( fieldLiteral ';' )* '@end' ;
