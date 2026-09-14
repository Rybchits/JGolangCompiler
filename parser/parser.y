%require "3.8.2"
%skeleton "lalr1.cc"
%define api.namespace {jgolang}
%define api.parser.class {Parser}
%define api.value.type variant
%define parse.assert

%code requires {
    #include "ast.h"
    #include <string>
    namespace jgolang {
        class Driver;
        class Scanner;
    }
}

%parse-param {jgolang::Driver& driver}
%parse-param {jgolang::Scanner& scanner}
%lex-param {jgolang::Scanner& scanner}

%code provides {
    int yylex(jgolang::Parser::semantic_type* value,
              jgolang::Parser::location_type* location,
              jgolang::Scanner& scanner);
}

%code {
    #include "parser/driver.hpp"
    #include "lexer/scanner.hpp"
}

%locations

%type <IdentifiersList *> IdentifiersList
%type <FunctionList *> InterfaceMembersMoreTwo
%type <TypeList *> TypesWithIdentifiersList
%type <ExpressionList *> ExpressionList Arguments
%type <StatementList *> StatementMoreTwo
%type <SwitchCaseList *> ExprCaseOrDefaultClauseList ExprCaseOrDefaultClauseListOrEmpty
%type <DeclarationList *> Declaration TopLevelDecl TopLevelDeclList TopLevelDeclListOrEmpty
%type <std::list<IdentifiersWithType *> *> FieldDeclMoreTwo Result Parameters NamedArgsList
%type <ElementCompositeLiteralList *> CompositeLiteralBody ElementList
%type <DeclarationList *> VariableDecl VariableSpecMoreTwo ConstDecl ConstSpecMoreTwo TypeDefMoreTwo TypeDecl

%type <BlockStatement *> Block
%type <FunctionSignature *> Signature
%type <SwitchCaseClause *> ExprCaseOrDefaultClause
%type <ElementCompositeLiteral *> KeyedElement
%type <VariableDeclaration *> VariableSpec ConstSpec
%type <IdentifiersWithType *> IdentifiersWithType VariadicNamedArgument
%type <DeclarationAST *> FunctionDecl MethodDecl TypeDef
%type <StatementAST *> Statement SimpleStmt Assignment ReturnStmt IfStmt ForStmt SwitchStmt ShortVarDecl
%type <TypeAST *> Type TypeOnly LiteralType StructType SliceDeclType ArrayDeclType FunctionType VariadicType InterfaceType
%type <ExpressionAST *> Expression Operand BasicLiteral CompositeLiteral FunctionLiteral AccessExpression

%type <ExpressionAST *> HeaderOperand HeaderExpression HeaderAccessExpression HeaderExpressionOptional
%type <ExpressionList *> HeaderExpressionList
%type <StatementAST *> HeaderAssignment HeaderShortVarDecl HeaderSimpleStmt

%token <long long> INT_LIT
%token <double> FLOAT_LIT
%token <int32_t> RUNE_LIT
%token <std::string> STRING_LIT
%token <std::string> IDENTIFIER

%token END   0 	"end of file"

%token BREAK DEFAULT FUNC CASE RETURN VAR TRUE FALSE
       MAP STRUCT ELSE PACKAGE SWITCH INTERFACE NIL
       CONST FALLTHROUGH IF RANGE TYPE CONTINUE FOR

%left OR                            // ||
%left AND                           // &&

%right PLUS_ASSIGNMENT              // +=
%right MINUS_ASSIGNMENT             // -=
%right MUL_ASSIGNMENT               // *=
%right DIV_ASSIGNMENT               // /=
%right MOD_ASSIGNMENT               // %=
%right SHORT_DECL_OP                // :=

%left EQUAL                         // ==
%left NOT_EQUAL                     // !=
%left LESS_OR_EQUAL                 // <=
%left GREATER_OR_EQUAL              // >=
%left '>' '<'
%left '+' '-'
%left '/' '%' '*'
%left '.' ']' '['
%nonassoc '(' ')'
%nonassoc '{' '}'

%right UNARY_PLUS                   // +
%right UNARY_MINUS                  // -
%right POINTER                      // *
%right VARIADIC                     // ...
%right '!'

%left INCREMENT DECREMENT           // ++ --

%start Root

%%
    // The first statement in a Go source file must be package name
    Root: PACKAGE IDENTIFIER SCs TopLevelDeclListOrEmpty                            { driver.setRoot(new PackageAST($2, *$4)); }
    ;

    TopLevelDeclListOrEmpty: TopLevelDeclList                                       { $$ = $1; }
                | /* empty */                                                       { $$ = new DeclarationList(); }
    ;

    TopLevelDeclList: TopLevelDecl SCs                                              { $$ = $1; }
                | TopLevelDeclList TopLevelDecl SCs                                 { $$ = $1; $$->insert($$->end(), $2->begin(), $2->end()); }
    ;

    TopLevelDecl: Declaration                                                       { $$ = $1; }
                | FunctionDecl                                                      { $$ = new DeclarationList({$1}); }
                | MethodDecl                                                        { $$ = new DeclarationList({$1}); }
    ;

    Declaration: ConstDecl                                                          { $$ = $1; }
                | VariableDecl                                                      { $$ = $1; }
                | TypeDecl                                                          { $$ = $1; }
    ;

/* -------------------------------- Types -------------------------------- */

    Type: IDENTIFIER                                                                { $$ = new IdentifierAsType($1); }
                | TypeOnly                                                          { $$ = $1; }
    ;

    TypesWithIdentifiersList: TypeOnly                                              { $$ = new TypeList({$1}); }
                | IdentifiersList ',' TypeOnly                                      { $$ = ListIdentifiersToListTypes(*$1); $$ -> push_back($3); }
                | TypesWithIdentifiersList ',' TypeOnly                             { $$ = $1; $$ -> push_back($3); }
                | TypesWithIdentifiersList ',' IDENTIFIER                           { $$ = $1; $$ -> push_back(new IdentifierAsType($3)); }
    ;

    TypeOnly: '(' Type ')'                                                          { $$ = $2; }
                | LiteralType                                                       { $$ = $1; }
    ;

    LiteralType: SliceDeclType                                                      { $$ = $1; }
                | FunctionType                                                      { $$ = $1; }
                | ArrayDeclType                                                     { $$ = $1; }
                | StructType                                                        { $$ = $1; }
                | InterfaceType                                                     { $$ = $1; }
                | '*' Type %prec POINTER                                            { $$ = $2; $$ -> isPointer = true; }
    ;

    // Struct types
    StructType: STRUCT '{' FieldDeclMoreTwo '}'                                     { $$ = new StructSignature(*$3); }
                | STRUCT '{' FieldDeclMoreTwo SCs '}'                               { $$ = new StructSignature(*$3); }
                | STRUCT '{' IdentifiersWithType '}'                                { $$ = new StructSignature(*(new std::list<IdentifiersWithType *>({$3}))); }
                | STRUCT '{' IdentifiersWithType SCs '}'                            { $$ = new StructSignature(*(new std::list<IdentifiersWithType *>({$3}))); }
                | STRUCT '{' IDENTIFIER '}'                                         {
                                                                                        auto typedIds = new IdentifiersWithType(*(new IdentifiersList({""})), new IdentifierAsType($3));
                                                                                        auto fields = new std::list<IdentifiersWithType *>({typedIds});
                                                                                        $$ = new StructSignature(*fields);
                                                                                    }
                | STRUCT '{' IDENTIFIER SCs '}'                                     {
                                                                                        auto typedIds = new IdentifiersWithType(*(new IdentifiersList({""})), new IdentifierAsType($3));
                                                                                        auto fields = new std::list<IdentifiersWithType *>({typedIds});
                                                                                        $$ = new StructSignature(*fields);
                                                                                    }
    ;

    FieldDeclMoreTwo: IdentifiersWithType SCs IdentifiersWithType                   { $$ = new std::list<IdentifiersWithType *>({$1, $3}); }
                | FieldDeclMoreTwo SCs IdentifiersWithType                          { $$ = $1; $$ -> push_back($3); }
                | FieldDeclMoreTwo SCs IDENTIFIER                                   { $$ = $1; $$ -> push_back(new IdentifiersWithType(*(new IdentifiersList({""})), new IdentifierAsType($3))); }
                | IDENTIFIER SCs IdentifiersWithType                                {
                                                                                        $$ = new std::list<IdentifiersWithType *>();
                                                                                        $$ -> push_back(new IdentifiersWithType(*(new IdentifiersList({""})), new IdentifierAsType($1)));
                                                                                        $$ -> push_back($3);
                                                                                    }
                | IdentifiersWithType SCs IDENTIFIER                                {
                                                                                        $$ = new std::list<IdentifiersWithType *>();
                                                                                        $$ -> push_back($1);
                                                                                        $$ -> push_back(new IdentifiersWithType(*(new IdentifiersList({""})), new IdentifierAsType($3)));
                                                                                    }
                | IDENTIFIER SCs IDENTIFIER                                         {
                                                                                        $$ = new std::list<IdentifiersWithType *>();
                                                                                        $$ -> push_back(new IdentifiersWithType(*(new IdentifiersList({""})), new IdentifierAsType($1)));
                                                                                        $$ -> push_back(new IdentifiersWithType(*(new IdentifiersList({""})), new IdentifierAsType($3)));
                                                                                    }
    ;

    InterfaceType: INTERFACE '{' '}'                                                { $$ = new InterfaceType(*(new FunctionList())); }
                | INTERFACE '{' IDENTIFIER Signature '}'                            { $$ = new InterfaceType(*(new FunctionList({new FunctionDeclaration($3, $4, nullptr)}))); }
                | INTERFACE '{' IDENTIFIER Signature SCs '}'                        { $$ = new InterfaceType(*(new FunctionList({new FunctionDeclaration($3, $4, nullptr)}))); }
                | INTERFACE '{' InterfaceMembersMoreTwo '}'                         { $$ = new InterfaceType(*$3); }
                | INTERFACE '{' InterfaceMembersMoreTwo SCs '}'                     { $$ = new InterfaceType(*$3); }

    InterfaceMembersMoreTwo: IDENTIFIER Signature SCs IDENTIFIER Signature          { $$ = new FunctionList({new FunctionDeclaration($1, $2, nullptr), new FunctionDeclaration($4, $5, nullptr)}); }
                | InterfaceMembersMoreTwo SCs IDENTIFIER Signature                  { $$ = $1; $$ -> push_back(new FunctionDeclaration($3, $4, nullptr)); }

    SliceDeclType: '[' ']' Type { $$ = new ArraySignature($3); }
    ;

    ArrayDeclType: '[' INT_LIT ']' Type { $$ = new ArraySignature($4, $2); }
    ;

    IdentifiersWithType: IdentifiersList Type                                       { $$ = new IdentifiersWithType(*$1, $2); }
    ;

    IdentifiersList: IDENTIFIER                                                     { $$ = new IdentifiersList({$1}); }
                | IdentifiersList ',' IDENTIFIER                                    { $$ = $1; $$ -> push_back($3); }
    ;

// Function type
    FunctionType: FUNC Signature                                                    { $$ = $2; }
    ;

    Signature: Parameters                                                           { $$ = new FunctionSignature(*$1, *(new std::list<IdentifiersWithType *>())); }
                | Parameters Result                                                 { $$ = new FunctionSignature(*$1, *$2); }
    ;

    Result: '(' ')'                                                                 { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | '(' NamedArgsList ')'                                             { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | '(' NamedArgsList ',' ')'                                         { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | '(' TypesWithIdentifiersList ')'                                  { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | '(' IdentifiersList ')'                                           { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | '(' TypesWithIdentifiersList ',' ')'                              { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | '(' IdentifiersList ',' ')'                                       { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | LiteralType                                                       {
                                                                                        $$ = new std::list<IdentifiersWithType *>();
                                                                                        $$ -> push_back(new IdentifiersWithType( *(new IdentifiersList({"_"})), $1));
                                                                                    }
                | IDENTIFIER                                                        {
                                                                                        $$ = new std::list<IdentifiersWithType *>();
                                                                                        $$ -> push_back(new IdentifiersWithType( *(new IdentifiersList({"_"})), new IdentifierAsType($1)));
                                                                                    }
    ;

    Parameters: '(' ')'                                                             { $$ = new std::list<IdentifiersWithType *>(); }
                | '(' NamedArgsList ')'                                             { $$ = $2; }
                | '(' NamedArgsList ',' ')'                                         { $$ = $2; }
                | '(' VariadicNamedArgument ')'                                     { $$ = new std::list<IdentifiersWithType *>({$2}); }
                | '(' NamedArgsList ',' VariadicNamedArgument ')'                   { $$ = $2; $$ -> push_back($4); }
                | '(' TypesWithIdentifiersList ')'                                  { $$ = AttachIdentifiersToListTypes(*$2); }
                | '(' IdentifiersList ')'                                           { $$ = AttachIdentifiersToListTypes( *ListIdentifiersToListTypes(*$2) ); }
                | '(' TypesWithIdentifiersList ',' ')'                              { $$ = AttachIdentifiersToListTypes(*$2); }
                | '(' IdentifiersList ',' ')'                                       { $$ = AttachIdentifiersToListTypes( *ListIdentifiersToListTypes(*$2) ); }
                | '(' TypesWithIdentifiersList ',' VariadicType ')'                 {
                                                                                        $2 -> push_back($4);
                                                                                        $$ = AttachIdentifiersToListTypes(*$2);
                                                                                    }
                | '(' IdentifiersList ',' VariadicType ')'                          {
                                                                                        TypeList* temp = ListIdentifiersToListTypes(*$2);
                                                                                        temp -> push_back($4);
                                                                                        $$ = AttachIdentifiersToListTypes(*temp);
                                                                                    }
                | '(' TypesWithIdentifiersList ',' VariadicType ',' ')'             {
                                                                                        $2 -> push_back($4);
                                                                                        $$ = AttachIdentifiersToListTypes(*$2);
                                                                                    }
                | '(' IdentifiersList ',' VariadicType ',' ')'                      {
                                                                                        TypeList* temp = ListIdentifiersToListTypes(*$2);
                                                                                        temp -> push_back($4);
                                                                                        $$ = AttachIdentifiersToListTypes(*temp);
                                                                                    }
    ;

    NamedArgsList: IdentifiersWithType                                              { $$ = new std::list<IdentifiersWithType *>({$1}); }
                | NamedArgsList ',' IdentifiersWithType                             { $$ = $1; $$ -> push_back($3); }
    ;

    VariadicNamedArgument: IDENTIFIER VariadicType                                  { $$ = new IdentifiersWithType(*(new IdentifiersList({ $1 })), $2); }
    ;

    VariadicType: VARIADIC Type                                                     { $$ = $2; $$ -> isVariadic = true; }
    ;

// Variable Declarations
    VariableDecl: VAR VariableSpec                                                  { $$ = new DeclarationList({$2}); }
                | VAR '(' VariableSpecMoreTwo ')'                                   { $$ = $3; }
                | VAR '(' VariableSpecMoreTwo SCs ')'                               { $$ = $3; }
                | VAR '(' VariableSpec ')'                                          { $$ = new DeclarationList({$3}); }
                | VAR '(' VariableSpec SCs ')'                                      { $$ = new DeclarationList({$3}); }
                | VAR '(' ')'                                                       { $$ = new DeclarationList(); }
    ;

    VariableSpecMoreTwo: VariableSpecMoreTwo SCs VariableSpec                       { $$ = $1; $$ -> push_back($3); }
                | VariableSpec SCs VariableSpec                                     { $$ = new DeclarationList({$1, $3}); }
	;

    VariableSpec: IdentifiersWithType '=' ExpressionList                            { $$ = new VariableDeclaration($1, *$3, false); }
		| IdentifiersWithType                                                       { $$ = new VariableDeclaration($1, *(new ExpressionList()), false); }
		| IdentifiersList '=' ExpressionList                                        { $$ = new VariableDeclaration(new IdentifiersWithType(*$1, nullptr), *$3, false); }
	;

// Constants Declarations
    ConstDecl: CONST ConstSpec                                                      { $$ = new DeclarationList({$2}); }
                | CONST '(' ConstSpecMoreTwo ')'                                    { $$ = $3; }
                | CONST '(' ConstSpecMoreTwo SCs ')'                                { $$ = $3; }
                | CONST '(' ConstSpec ')'                                           { $$ = new DeclarationList({$3}); }
                | CONST '(' ConstSpec SCs ')'                                       { $$ = new DeclarationList({$3}); }
                | CONST '(' ')'                                                     { $$ = new DeclarationList(); }
    ;


    ConstSpecMoreTwo: ConstSpec SCs ConstSpec                                       { $$ = new DeclarationList({$1, $3}); }
                | ConstSpecMoreTwo SCs ConstSpec                                    { $$ = $1; $$ -> push_back($3); }
    ;

    ConstSpec: IdentifiersList IDENTIFIER '=' ExpressionList                        { $$ = new VariableDeclaration(new IdentifiersWithType(*$1, new IdentifierAsType($2)), *$4, true); }
                | IdentifiersList '=' ExpressionList                                { $$ = new VariableDeclaration(new IdentifiersWithType(*$1, nullptr), *$3, true); }
    ;

// Function declarations
    FunctionDecl: FUNC IDENTIFIER Signature Block                                   { $$ = new FunctionDeclaration($2, $3, $4); }
                | FUNC IDENTIFIER Signature                                         { $$ = new FunctionDeclaration($2, $3, nullptr); }
    ;

// Method declaration
    MethodDecl: FUNC '(' IDENTIFIER '*' IDENTIFIER ')' IDENTIFIER Signature Block  {
                                                                                        IdentifierAsType* type = new IdentifierAsType($5);
                                                                                        type->isPointer = true;
                                                                                        $$ = new MethodDeclaration($7, $3, type, $8, $9);
                                                                                    }
                | FUNC '(' IDENTIFIER '*' IDENTIFIER ')' IDENTIFIER Signature      {
                                                                                        IdentifierAsType* type = new IdentifierAsType($5);
                                                                                        type->isPointer = true;
                                                                                        $$ = new MethodDeclaration($7, $3, new IdentifierAsType($5), $8, nullptr);
                                                                                    }
                | FUNC '(' IDENTIFIER IDENTIFIER ')' IDENTIFIER Signature Block    { $$ = new MethodDeclaration($6, $3, new IdentifierAsType($4), $7, $8); }
                | FUNC '(' IDENTIFIER IDENTIFIER ')' IDENTIFIER Signature          { $$ = new MethodDeclaration($6, $3, new IdentifierAsType($4), $7, nullptr); }
    ;

// Type declaration
    TypeDecl: TYPE TypeDef                                                          { $$ = new DeclarationList({$2}); }
                | TYPE '(' TypeDefMoreTwo ')'                                       { $$ = $3; }
                | TYPE '(' TypeDefMoreTwo SCs ')'                                   { $$ = $3; }
                | TYPE '(' TypeDef ')'                                              { $$ = new DeclarationList({$3}); }
                | TYPE '(' TypeDef SCs ')'                                          { $$ = new DeclarationList({$3}); }
                | TYPE '(' ')'                                                      { $$ = new DeclarationList(); }
    ;

    TypeDefMoreTwo: TypeDef SCs TypeDef                                             { $$ = new DeclarationList({$1, $3}); }
                | TypeDefMoreTwo SCs TypeDef                                        { $$ = $1; $$ -> push_back($3); }
    ;

    TypeDef: IDENTIFIER Type                                                        { $$ = new TypeDeclaration($1, $2); }
    ;

/* -------------------------------- Expressions -------------------------------- */

    Operand: FunctionLiteral                                                        { $$ = $1; }
                | CompositeLiteral                                                  { $$ = $1; }
                | IDENTIFIER                                                        { $$ = new IdentifierAsExpression($1); }
                | '(' Expression ')'                                                { $$ = $2; }
                | IDENTIFIER CompositeLiteralBody                                   { $$ = new CompositeLiteral(new IdentifierAsType($1), *$2); }
    ;


    BasicLiteral: INT_LIT                                                           { $$ = new IntegerExpression($1);       }
		| RUNE_LIT                                                          { $$ = new IntegerExpression($1);       }
		| FLOAT_LIT                                                         { $$ = new FloatExpression($1);         }
                | STRING_LIT                                                        { $$ = new StringExpression($1);        }
                | FALSE                                                             { $$ = new BooleanExpression(false);    }
                | TRUE                                                              { $$ = new BooleanExpression(true);     }
                | NIL                                                               { $$ = new NilExpression();             }
    ;

// Composite literals
    CompositeLiteral: SliceDeclType CompositeLiteralBody                            { $$ = new CompositeLiteral($1, *$2);   }
                | ArrayDeclType CompositeLiteralBody                                { $$ = new CompositeLiteral($1, *$2);   }
                | StructType CompositeLiteralBody                                   { throw jgolang::Parser::syntax_error(@$, "Structs are not supported yet");                                         }
    ;

    CompositeLiteralBody: '{' ElementList '}'                                       { $$ = $2; }
                | '{' ElementList ',' '}'                                           { $$ = $2; }
                | '{' '}'                                                           { $$ = new ElementCompositeLiteralList(); }
    ;

    ElementList: KeyedElement                                                       { $$ = new ElementCompositeLiteralList({$1}); }
                | ElementList ',' KeyedElement                                      { $$ = $1; $$ -> push_back($3); }
    ;

    KeyedElement: Expression ':' Expression                                         { $$ = new ElementCompositeLiteral($1, $3); }
                | Expression                                                        { $$ = new ElementCompositeLiteral(nullptr, $1); }
                | Expression ':' CompositeLiteralBody                               { $$ = new ElementCompositeLiteral($1, *$3); }
                | CompositeLiteralBody                                              { $$ = new ElementCompositeLiteral(nullptr, *$1); }
    ;

    FunctionLiteral: FUNC Signature Block                                           { $$ = new FunctionLitExpression($2, $3); }
    ;

    Expression: AccessExpression                                                    { $$ = $1; }
                | BasicLiteral                                                      { $$ = $1; }
                | Expression '+' Expression                                         { $$ = new BinaryExpression(BinaryExpression::Addition, $1, $3);        }
                | Expression '-' Expression                                         { $$ = new BinaryExpression(BinaryExpression::Subtraction, $1, $3);     }
                | Expression '*' Expression                                         { $$ = new BinaryExpression(BinaryExpression::Multiplication, $1, $3);  }
                | Expression '/' Expression                                         { $$ = new BinaryExpression(BinaryExpression::Division, $1, $3);        }
                | Expression '%' Expression                                         { $$ = new BinaryExpression(BinaryExpression::Mod, $1, $3);             }
                | Expression '<' Expression                                         { $$ = new BinaryExpression(BinaryExpression::Less, $1, $3);            }
                | Expression '>' Expression                                         { $$ = new BinaryExpression(BinaryExpression::Greater, $1, $3);         }
                | Expression EQUAL Expression                                       { $$ = new BinaryExpression(BinaryExpression::Equal, $1, $3);           }
                | Expression NOT_EQUAL Expression                                   { $$ = new BinaryExpression(BinaryExpression::NotEqual, $1, $3);        }
                | Expression LESS_OR_EQUAL Expression                               { $$ = new BinaryExpression(BinaryExpression::LessOrEqual, $1, $3);     }
                | Expression GREATER_OR_EQUAL Expression                            { $$ = new BinaryExpression(BinaryExpression::GreatOrEqual, $1, $3);    }
                | Expression AND Expression                                         { $$ = new BinaryExpression(BinaryExpression::And, $1, $3);             }
                | Expression OR Expression                                          { $$ = new BinaryExpression(BinaryExpression::Or, $1, $3);              }
                | '+' Expression %prec UNARY_PLUS                                   { $$ = new UnaryExpression(UnaryExpression::UnaryPlus, $2);             }
                | '-' Expression %prec UNARY_MINUS                                  { $$ = new UnaryExpression(UnaryExpression::UnaryMinus, $2);            }
                | '!' Expression                                                    { $$ = new UnaryExpression(UnaryExpression::UnaryNot, $2);              }
                | Expression VARIADIC                                               { $$ = new UnaryExpression(UnaryExpression::Variadic, $1);              }
    ;

    AccessExpression: Operand                                                       { $$ = $1; }
                | AccessExpression '[' Expression ']'                               { $$ = new AccessExpression(AccessExpression::Indexing, $1, $3); }
                | AccessExpression '.' AccessExpression                             { $$ = new AccessExpression(AccessExpression::FieldSelect, $1, $3); }
                | AccessExpression '[' ':' ']'                                      { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | AccessExpression '[' Expression ':' ']'                           { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | AccessExpression '[' ':' Expression ']'                           { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | AccessExpression '[' Expression ':' Expression ']'                { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | AccessExpression Arguments                                        { $$ = new CallableExpression($1, *$2); }
    ;

    Arguments: '(' ExpressionList ',' ')'                                           { $$ = $2; }
		        | '(' ExpressionList ')'                                            { $$ = $2; }
                | '(' ')'                                                           { $$ = new ExpressionList(); }
	;

    ExpressionList: Expression                                                      { $$ = new ExpressionList({$1}); }
		| ExpressionList ',' Expression                                             { $$ = $1; $$ -> push_back($3); }
    ;

/* -------------------------------- Statements -------------------------------- */

    Assignment: Expression PLUS_ASSIGNMENT Expression                               { $$ = new AssignmentStatement(AssignmentStatement::PlusAssign, $1, $3);     }
                | Expression MINUS_ASSIGNMENT Expression                            { $$ = new AssignmentStatement(AssignmentStatement::MinusAssign, $1, $3);    }
                | Expression MUL_ASSIGNMENT Expression                              { $$ = new AssignmentStatement(AssignmentStatement::MulAssign, $1, $3);      }
                | Expression DIV_ASSIGNMENT Expression                              { $$ = new AssignmentStatement(AssignmentStatement::DivAssign, $1, $3);      }
                | Expression MOD_ASSIGNMENT Expression                              { $$ = new AssignmentStatement(AssignmentStatement::ModAssign, $1, $3);      }
                | ExpressionList '=' ExpressionList                                 { $$ = new AssignmentStatement(AssignmentStatement::SimpleAssign, *$1, *$3); }
    ;

    ShortVarDecl: ExpressionList SHORT_DECL_OP ExpressionList                       {
                                                                                        IdentifiersList* temp = IdentifiersListFromExpressions(*$1);

                                                                                        if (temp == nullptr) {
                                                                                            throw jgolang::Parser::syntax_error(@$, "Lhs of short declaration must contains only identifiers");

                                                                                        } else {
                                                                                            $$ = new ShortVarDeclarationStatement(*temp, *$3);
                                                                                        }
                                                                                    }
    ;

    Statement: Declaration                                                          { $$ = new DeclarationStatement(*$1); }
                | Block                                                             { $$ = $1; }
                | SimpleStmt                                                        { $$ = $1; }
                | ReturnStmt                                                        { $$ = $1; }
                | BREAK                                                             { $$ = new KeywordStatement(KeywordStatement::Break);        }
                | CONTINUE                                                          { $$ = new KeywordStatement(KeywordStatement::Continue);     }
                | FALLTHROUGH                                                       { $$ = new KeywordStatement(KeywordStatement::Fallthrough);  }
                | IfStmt                                                            { $$ = $1; }
                | ForStmt                                                           { $$ = $1; }
                | SwitchStmt                                                        { $$ = $1; }
    ;


    SimpleStmt: Expression                                                          { $$ = new ExpressionStatement($1); }
                | Assignment                                                        { $$ = $1; }
                | ShortVarDecl                                                      { $$ = $1; }
                | Expression INCREMENT                                              { $$ = new ExpressionStatement(new UnaryExpression(UnaryExpression::Increment, $1)); }
                | Expression DECREMENT                                              { $$ = new ExpressionStatement(new UnaryExpression(UnaryExpression::Decrement, $1)); }
    ;

    // Return statements
    ReturnStmt: RETURN ExpressionList                                               { $$ = new ReturnStatement(*$2); }
                | RETURN                                                            { $$ = new ReturnStatement(*(new ExpressionList())); }
    ;

/* -------------------------------- Blocks -------------------------------- */

    Block: '{' StatementMoreTwo '}'                                                 { $$ = new BlockStatement(*$2);     }
                | '{' StatementMoreTwo SCs '}'                                      { $$ = new BlockStatement(*$2);     }
                | '{' Statement '}'                                                 { $$ = new BlockStatement($2);      }
                | '{' Statement SCs '}'                                             { $$ = new BlockStatement($2);      }
                | '{' '}'                                                           { $$ = new BlockStatement();        }
    ;

    StatementMoreTwo: Statement SCs Statement                                       { $$ = new StatementList({$1, $3}); }
                | StatementMoreTwo SCs Statement                                    { $$ = $1; $$ -> push_back($3); }
    ;

    // At the outer level of a control header, an identifier followed by '{'
    // starts the block. Parentheses, arguments and indices use ordinary expressions.
    HeaderOperand: FunctionLiteral                                                  { $$ = $1; }
                | CompositeLiteral                                                  { $$ = $1; }
                | IDENTIFIER                                                        { $$ = new IdentifierAsExpression($1); }
                | '(' Expression ')'                                                { $$ = $2; }
    ;

    HeaderExpression: HeaderAccessExpression                                                    { $$ = $1; }
                | BasicLiteral                                                                  { $$ = $1; }
                | HeaderExpression '+' HeaderExpression                                         { $$ = new BinaryExpression(BinaryExpression::Addition, $1, $3);        }
                | HeaderExpression '-' HeaderExpression                                         { $$ = new BinaryExpression(BinaryExpression::Subtraction, $1, $3);     }
                | HeaderExpression '*' HeaderExpression                                         { $$ = new BinaryExpression(BinaryExpression::Multiplication, $1, $3);  }
                | HeaderExpression '/' HeaderExpression                                         { $$ = new BinaryExpression(BinaryExpression::Division, $1, $3);        }
                | HeaderExpression '%' HeaderExpression                                         { $$ = new BinaryExpression(BinaryExpression::Mod, $1, $3);             }
                | HeaderExpression '<' HeaderExpression                                         { $$ = new BinaryExpression(BinaryExpression::Less, $1, $3);            }
                | HeaderExpression '>' HeaderExpression                                         { $$ = new BinaryExpression(BinaryExpression::Greater, $1, $3);         }
                | HeaderExpression EQUAL HeaderExpression                                       { $$ = new BinaryExpression(BinaryExpression::Equal, $1, $3);           }
                | HeaderExpression NOT_EQUAL HeaderExpression                                   { $$ = new BinaryExpression(BinaryExpression::NotEqual, $1, $3);        }
                | HeaderExpression LESS_OR_EQUAL HeaderExpression                               { $$ = new BinaryExpression(BinaryExpression::LessOrEqual, $1, $3);     }
                | HeaderExpression GREATER_OR_EQUAL HeaderExpression                            { $$ = new BinaryExpression(BinaryExpression::GreatOrEqual, $1, $3);    }
                | HeaderExpression AND HeaderExpression                                         { $$ = new BinaryExpression(BinaryExpression::And, $1, $3);             }
                | HeaderExpression OR HeaderExpression                                          { $$ = new BinaryExpression(BinaryExpression::Or, $1, $3);              }
                | '+' HeaderExpression %prec UNARY_PLUS                                   { $$ = new UnaryExpression(UnaryExpression::UnaryPlus, $2);             }
                | '-' HeaderExpression %prec UNARY_MINUS                                  { $$ = new UnaryExpression(UnaryExpression::UnaryMinus, $2);            }
                | '!' HeaderExpression                                                    { $$ = new UnaryExpression(UnaryExpression::UnaryNot, $2);              }
                | HeaderExpression VARIADIC                                               { $$ = new UnaryExpression(UnaryExpression::Variadic, $1);              }
    ;

    HeaderAccessExpression: HeaderOperand                                                 { $$ = $1; }
                | HeaderAccessExpression '[' Expression ']'                               { $$ = new AccessExpression(AccessExpression::Indexing, $1, $3); }
                | HeaderAccessExpression '.' HeaderAccessExpression                       { $$ = new AccessExpression(AccessExpression::FieldSelect, $1, $3); }
                | HeaderAccessExpression '[' ':' ']'                                      { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | HeaderAccessExpression '[' Expression ':' ']'                           { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | HeaderAccessExpression '[' ':' Expression ']'                           { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | HeaderAccessExpression '[' Expression ':' Expression ']'                { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | HeaderAccessExpression Arguments                                        { $$ = new CallableExpression($1, *$2); }
    ;

    HeaderExpressionOptional: /* empty */                                                 { $$ = nullptr; }
                | HeaderExpression                                                        { $$ = $1; }
    ;

    HeaderExpressionList: HeaderExpression                                                      { $$ = new ExpressionList({$1}); }
		| HeaderExpressionList ',' HeaderExpression                                             { $$ = $1; $$ -> push_back($3); }
    ;

    HeaderAssignment: HeaderExpression PLUS_ASSIGNMENT HeaderExpression                               { $$ = new AssignmentStatement(AssignmentStatement::PlusAssign, $1, $3);     }
                | HeaderExpression MINUS_ASSIGNMENT HeaderExpression                            { $$ = new AssignmentStatement(AssignmentStatement::MinusAssign, $1, $3);    }
                | HeaderExpression MUL_ASSIGNMENT HeaderExpression                              { $$ = new AssignmentStatement(AssignmentStatement::MulAssign, $1, $3);      }
                | HeaderExpression DIV_ASSIGNMENT HeaderExpression                              { $$ = new AssignmentStatement(AssignmentStatement::DivAssign, $1, $3);      }
                | HeaderExpression MOD_ASSIGNMENT HeaderExpression                              { $$ = new AssignmentStatement(AssignmentStatement::ModAssign, $1, $3);      }
                | HeaderExpressionList '=' HeaderExpressionList                                 { $$ = new AssignmentStatement(AssignmentStatement::SimpleAssign, *$1, *$3); }
    ;

    HeaderShortVarDecl: HeaderExpressionList SHORT_DECL_OP HeaderExpressionList     {
                                                                                        IdentifiersList* temp = IdentifiersListFromExpressions(*$1);

                                                                                        if (temp == nullptr) {
                                                                                            throw jgolang::Parser::syntax_error(@$, "Lhs of short declaration must contains only identifiers");

                                                                                        } else {
                                                                                            $$ = new ShortVarDeclarationStatement(*temp, *$3);
                                                                                        }
                                                                                    }
    ;

    HeaderSimpleStmt: HeaderExpression                                                          { $$ = new ExpressionStatement($1); }
                | HeaderAssignment                                                        { $$ = $1; }
                | HeaderShortVarDecl                                                      { $$ = $1; }
                | HeaderExpression INCREMENT                                              { $$ = new ExpressionStatement(new UnaryExpression(UnaryExpression::Increment, $1)); }
                | HeaderExpression DECREMENT                                              { $$ = new ExpressionStatement(new UnaryExpression(UnaryExpression::Decrement, $1)); }
    ;

    IfStmt: IF HeaderSimpleStmt ';' HeaderExpression Block ELSE IfStmt   { $$ = new IfStatement($2, $4, $5, $7); }
        | IF HeaderExpression Block ELSE IfStmt                          { $$ = new IfStatement(nullptr, $2, $3, $5); }
        | IF HeaderSimpleStmt ';' HeaderExpression Block ELSE Block      { $$ = new IfStatement($2, $4, $5, $7); }
        | IF HeaderExpression Block ELSE Block                           { $$ = new IfStatement(nullptr, $2, $3, $5); }
        | IF HeaderSimpleStmt ';' HeaderExpression Block                 { $$ = new IfStatement($2, $4, $5, nullptr); }
        | IF HeaderExpression Block                                      { $$ = new IfStatement(nullptr, $2, $3, nullptr); }
    ;

    ForStmt: FOR HeaderExpression Block                                  { $$ = new WhileStatement($2, $3); }
        | FOR HeaderSimpleStmt ';' HeaderExpressionOptional ';' HeaderSimpleStmt Block
                                                                         { $$ = new ForStatement($2, $4, $6, $7); }
        | FOR HeaderExpressionList '=' RANGE HeaderExpression Block      { $$ = new ForRangeStatement(*$2, $5, $6, false); }
        | FOR HeaderExpressionList SHORT_DECL_OP RANGE HeaderExpression Block
                                                                         { $$ = new ForRangeStatement(*$2, $5, $6, true); }
        | FOR RANGE HeaderExpression Block                               { $$ = new ForRangeStatement(*(new ExpressionList()), $3, $4, false); }
        | FOR Block                                                      { $$ = new WhileStatement(new BooleanExpression(true), $2); }
    ;

    SwitchStmt: SWITCH HeaderSimpleStmt ';' HeaderExpressionOptional '{' ExprCaseOrDefaultClauseListOrEmpty '}'
                                                                         { $$ = new SwitchStatement($2, $4, *$6); }
        | SWITCH HeaderExpressionOptional '{' ExprCaseOrDefaultClauseListOrEmpty '}'
                                                                         { $$ = new SwitchStatement(nullptr, $2, *$4); }
    ;

    ExprCaseOrDefaultClause: CASE Expression ':' StatementMoreTwo  SCs              { $$ = new SwitchCaseClause($2, new BlockStatement(*$4));       }
                | CASE Expression ':' Statement SCs                                 { $$ = new SwitchCaseClause($2, new BlockStatement($4));        }
                | CASE Expression ':'                                               { $$ = new SwitchCaseClause($2, new BlockStatement());          }
                | DEFAULT ':' StatementMoreTwo SCs                                  { $$ = new SwitchCaseClause(nullptr, new BlockStatement(*$3));  }
                | DEFAULT ':' Statement SCs                                         { $$ = new SwitchCaseClause(nullptr, new BlockStatement($3));   }
    ;

    ExprCaseOrDefaultClauseList: ExprCaseOrDefaultClause                            { $$ = new SwitchCaseList({$1}); }
                | ExprCaseOrDefaultClauseList ExprCaseOrDefaultClause               { $$ = $1; $$ -> push_back($2); }
    ;

    ExprCaseOrDefaultClauseListOrEmpty: /* empty */                                 { $$ = new SwitchCaseList(); }
                | ExprCaseOrDefaultClauseList                                       { $$ = $1; }
    ;

    SCs: ';'                                                                        { }
                | SCs ';'                                                           { }
%%
