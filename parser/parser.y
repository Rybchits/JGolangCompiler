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

%type <IdentifiersList> IdentifiersList
%type <FunctionList> InterfaceMembersMoreTwo
%type <TypeList> TypesWithIdentifiersList
%type <ExpressionList> ExpressionList Arguments
%type <StatementList> StatementMoreTwo
%type <SwitchCaseList> ExprCaseOrDefaultClauseList ExprCaseOrDefaultClauseListOrEmpty
%type <DeclarationList> Declaration TopLevelDecl TopLevelDeclList TopLevelDeclListOrEmpty
%type <IdentifiersWithTypeList> FieldDeclMoreTwo Result Parameters NamedArgsList
%type <ElementCompositeLiteralList> CompositeLiteralBody ElementList
%type <DeclarationList> VariableDecl VariableSpecMoreTwo ConstDecl ConstSpecMoreTwo TypeDefMoreTwo TypeDecl

%type <BlockStatementPtr> Block
%type <FunctionSignaturePtr> Signature
%type <SwitchCaseClausePtr> ExprCaseOrDefaultClause
%type <ElementCompositeLiteralPtr> KeyedElement
%type <VariableDeclarationPtr> VariableSpec ConstSpec
%type <IdentifiersWithTypePtr> IdentifiersWithType VariadicNamedArgument
%type <DeclarationASTPtr> FunctionDecl MethodDecl TypeDef
%type <StatementASTPtr> Statement SimpleStmt Assignment ReturnStmt IfStmt ForStmt SwitchStmt ShortVarDecl
%type <TypeASTPtr> Type TypeOnly LiteralType StructType SliceDeclType ArrayDeclType FunctionType VariadicType InterfaceType
%type <ExpressionASTPtr> Expression Operand BasicLiteral CompositeLiteral FunctionLiteral AccessExpression

%type <ExpressionASTPtr> HeaderOperand HeaderExpression HeaderAccessExpression HeaderExpressionOptional
%type <ExpressionList> HeaderExpressionList
%type <StatementASTPtr> HeaderAssignment HeaderShortVarDecl HeaderSimpleStmt

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
    Root: PACKAGE IDENTIFIER SCs TopLevelDeclListOrEmpty                            { driver.setRoot(std::make_unique<PackageAST>(std::move($2), std::move($4))); }
    ;

    TopLevelDeclListOrEmpty: TopLevelDeclList                                       { $$ = std::move($1); }
                | /* empty */                                                       { $$ = MakeList<DeclarationList>(); }
    ;

    TopLevelDeclList: TopLevelDecl SCs                                              { $$ = std::move($1); }
                | TopLevelDeclList TopLevelDecl SCs                                 { $$ = std::move($1); $$.splice($$.end(), $2); }
    ;

    TopLevelDecl: Declaration                                                       { $$ = std::move($1); }
                | FunctionDecl                                                      { $$ = MakeList<DeclarationList>(std::move($1)); }
                | MethodDecl                                                        { $$ = MakeList<DeclarationList>(std::move($1)); }
    ;

    Declaration: ConstDecl                                                          { $$ = std::move($1); }
                | VariableDecl                                                      { $$ = std::move($1); }
                | TypeDecl                                                          { $$ = std::move($1); }
    ;

/* -------------------------------- Types -------------------------------- */

    Type: IDENTIFIER                                                                { $$ = std::make_unique<IdentifierAsType>(std::move($1)); }
                | TypeOnly                                                          { $$ = std::move($1); }
    ;

    TypesWithIdentifiersList: TypeOnly                                              { $$ = MakeList<TypeList>(std::move($1)); }
                | IdentifiersList ',' TypeOnly                                      { $$ = ListIdentifiersToListTypes($1); $$.push_back(std::move($3)); }
                | TypesWithIdentifiersList ',' TypeOnly                             { $$ = std::move($1); $$.push_back(std::move($3)); }
                | TypesWithIdentifiersList ',' IDENTIFIER                           { $$ = std::move($1); $$.push_back(std::make_unique<IdentifierAsType>(std::move($3))); }
    ;

    TypeOnly: '(' Type ')'                                                          { $$ = std::move($2); }
                | LiteralType                                                       { $$ = std::move($1); }
    ;

    LiteralType: SliceDeclType                                                      { $$ = std::move($1); }
                | FunctionType                                                      { $$ = std::move($1); }
                | ArrayDeclType                                                     { $$ = std::move($1); }
                | StructType                                                        { $$ = std::move($1); }
                | InterfaceType                                                     { $$ = std::move($1); }
                | '*' Type %prec POINTER                                            { $$ = std::move($2); $$ -> isPointer = true; }
    ;

    // Struct types
    StructType: STRUCT '{' FieldDeclMoreTwo '}'                                     { $$ = std::make_unique<StructSignature>(std::move($3)); }
                | STRUCT '{' FieldDeclMoreTwo SCs '}'                               { $$ = std::make_unique<StructSignature>(std::move($3)); }
                | STRUCT '{' IdentifiersWithType '}'                                { $$ = std::make_unique<StructSignature>(MakeList<IdentifiersWithTypeList>(std::move($3))); }
                | STRUCT '{' IdentifiersWithType SCs '}'                            { $$ = std::make_unique<StructSignature>(MakeList<IdentifiersWithTypeList>(std::move($3))); }
                | STRUCT '{' IDENTIFIER '}'                                         {
                                                                                        auto typedIds = std::make_unique<IdentifiersWithType>(MakeList<IdentifiersList>(""), std::make_unique<IdentifierAsType>(std::move($3)));
                                                                                        auto fields = MakeList<IdentifiersWithTypeList>(std::move(typedIds));
                                                                                        $$ = std::make_unique<StructSignature>(std::move(fields));
                                                                                    }
                | STRUCT '{' IDENTIFIER SCs '}'                                     {
                                                                                        auto typedIds = std::make_unique<IdentifiersWithType>(MakeList<IdentifiersList>(""), std::make_unique<IdentifierAsType>(std::move($3)));
                                                                                        auto fields = MakeList<IdentifiersWithTypeList>(std::move(typedIds));
                                                                                        $$ = std::make_unique<StructSignature>(std::move(fields));
                                                                                    }
    ;

    FieldDeclMoreTwo: IdentifiersWithType SCs IdentifiersWithType                   { $$ = MakeList<IdentifiersWithTypeList>(std::move($1), std::move($3)); }
                | FieldDeclMoreTwo SCs IdentifiersWithType                          { $$ = std::move($1); $$.push_back(std::move($3)); }
                | FieldDeclMoreTwo SCs IDENTIFIER                                   { $$ = std::move($1); $$.push_back(std::make_unique<IdentifiersWithType>(MakeList<IdentifiersList>(""), std::make_unique<IdentifierAsType>(std::move($3)))); }
                | IDENTIFIER SCs IdentifiersWithType                                {
                                                                                        $$ = MakeList<IdentifiersWithTypeList>();
                                                                                        $$.push_back(std::make_unique<IdentifiersWithType>(MakeList<IdentifiersList>(""), std::make_unique<IdentifierAsType>(std::move($1))));
                                                                                        $$.push_back(std::move($3));
                                                                                    }
                | IdentifiersWithType SCs IDENTIFIER                                {
                                                                                        $$ = MakeList<IdentifiersWithTypeList>();
                                                                                        $$.push_back(std::move($1));
                                                                                        $$.push_back(std::make_unique<IdentifiersWithType>(MakeList<IdentifiersList>(""), std::make_unique<IdentifierAsType>(std::move($3))));
                                                                                    }
                | IDENTIFIER SCs IDENTIFIER                                         {
                                                                                        $$ = MakeList<IdentifiersWithTypeList>();
                                                                                        $$.push_back(std::make_unique<IdentifiersWithType>(MakeList<IdentifiersList>(""), std::make_unique<IdentifierAsType>(std::move($1))));
                                                                                        $$.push_back(std::make_unique<IdentifiersWithType>(MakeList<IdentifiersList>(""), std::make_unique<IdentifierAsType>(std::move($3))));
                                                                                    }
    ;

    InterfaceType: INTERFACE '{' '}'                                                { $$ = std::make_unique<InterfaceType>(MakeList<FunctionList>()); }
                | INTERFACE '{' IDENTIFIER Signature '}'                            { $$ = std::make_unique<InterfaceType>(MakeList<FunctionList>(std::make_unique<FunctionDeclaration>(std::move($3), std::move($4), nullptr))); }
                | INTERFACE '{' IDENTIFIER Signature SCs '}'                        { $$ = std::make_unique<InterfaceType>(MakeList<FunctionList>(std::make_unique<FunctionDeclaration>(std::move($3), std::move($4), nullptr))); }
                | INTERFACE '{' InterfaceMembersMoreTwo '}'                         { $$ = std::make_unique<InterfaceType>(std::move($3)); }
                | INTERFACE '{' InterfaceMembersMoreTwo SCs '}'                     { $$ = std::make_unique<InterfaceType>(std::move($3)); }

    InterfaceMembersMoreTwo: IDENTIFIER Signature SCs IDENTIFIER Signature          { $$ = MakeList<FunctionList>(std::make_unique<FunctionDeclaration>(std::move($1), std::move($2), nullptr), std::make_unique<FunctionDeclaration>(std::move($4), std::move($5), nullptr)); }
                | InterfaceMembersMoreTwo SCs IDENTIFIER Signature                  { $$ = std::move($1); $$.push_back(std::make_unique<FunctionDeclaration>(std::move($3), std::move($4), nullptr)); }

    SliceDeclType: '[' ']' Type { $$ = std::make_unique<ArraySignature>(std::move($3)); }
    ;

    ArrayDeclType: '[' INT_LIT ']' Type { $$ = std::make_unique<ArraySignature>(std::move($4), std::move($2)); }
    ;

    IdentifiersWithType: IdentifiersList Type                                       { $$ = std::make_unique<IdentifiersWithType>(std::move($1), std::move($2)); }
    ;

    IdentifiersList: IDENTIFIER                                                     { $$ = MakeList<IdentifiersList>(std::move($1)); }
                | IdentifiersList ',' IDENTIFIER                                    { $$ = std::move($1); $$.push_back(std::move($3)); }
    ;

// Function type
    FunctionType: FUNC Signature                                                    { $$ = std::move($2); }
    ;

    Signature: Parameters                                                           { $$ = std::make_unique<FunctionSignature>(std::move($1), MakeList<IdentifiersWithTypeList>()); }
                | Parameters Result                                                 { $$ = std::make_unique<FunctionSignature>(std::move($1), std::move($2)); }
    ;

    Result: '(' ')'                                                                 { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | '(' NamedArgsList ')'                                             { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | '(' NamedArgsList ',' ')'                                         { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | '(' TypesWithIdentifiersList ')'                                  { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | '(' IdentifiersList ')'                                           { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | '(' TypesWithIdentifiersList ',' ')'                              { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | '(' IdentifiersList ',' ')'                                       { throw jgolang::Parser::syntax_error(@$, "Many returns are not supported yet"); }
                | LiteralType                                                       {
                                                                                        $$ = MakeList<IdentifiersWithTypeList>();
                                                                                        $$.push_back(std::make_unique<IdentifiersWithType>( MakeList<IdentifiersList>("_"), std::move($1)));
                                                                                    }
                | IDENTIFIER                                                        {
                                                                                        $$ = MakeList<IdentifiersWithTypeList>();
                                                                                        $$.push_back(std::make_unique<IdentifiersWithType>( MakeList<IdentifiersList>("_"), std::make_unique<IdentifierAsType>(std::move($1))));
                                                                                    }
    ;

    Parameters: '(' ')'                                                             { $$ = MakeList<IdentifiersWithTypeList>(); }
                | '(' NamedArgsList ')'                                             { $$ = std::move($2); }
                | '(' NamedArgsList ',' ')'                                         { $$ = std::move($2); }
                | '(' VariadicNamedArgument ')'                                     { $$ = MakeList<IdentifiersWithTypeList>(std::move($2)); }
                | '(' NamedArgsList ',' VariadicNamedArgument ')'                   { $$ = std::move($2); $$.push_back(std::move($4)); }
                | '(' TypesWithIdentifiersList ')'                                  { $$ = AttachIdentifiersToListTypes(std::move($2)); }
                | '(' IdentifiersList ')'                                           { $$ = AttachIdentifiersToListTypes( ListIdentifiersToListTypes($2) ); }
                | '(' TypesWithIdentifiersList ',' ')'                              { $$ = AttachIdentifiersToListTypes(std::move($2)); }
                | '(' IdentifiersList ',' ')'                                       { $$ = AttachIdentifiersToListTypes( ListIdentifiersToListTypes($2) ); }
                | '(' TypesWithIdentifiersList ',' VariadicType ')'                 {
                                                                                        $2.push_back(std::move($4));
                                                                                        $$ = AttachIdentifiersToListTypes(std::move($2));
                                                                                    }
                | '(' IdentifiersList ',' VariadicType ')'                          {
                                                                                        TypeList temp = ListIdentifiersToListTypes($2);
                                                                                        temp.push_back(std::move($4));
                                                                                        $$ = AttachIdentifiersToListTypes(std::move(temp));
                                                                                    }
                | '(' TypesWithIdentifiersList ',' VariadicType ',' ')'             {
                                                                                        $2.push_back(std::move($4));
                                                                                        $$ = AttachIdentifiersToListTypes(std::move($2));
                                                                                    }
                | '(' IdentifiersList ',' VariadicType ',' ')'                      {
                                                                                        TypeList temp = ListIdentifiersToListTypes($2);
                                                                                        temp.push_back(std::move($4));
                                                                                        $$ = AttachIdentifiersToListTypes(std::move(temp));
                                                                                    }
    ;

    NamedArgsList: IdentifiersWithType                                              { $$ = MakeList<IdentifiersWithTypeList>(std::move($1)); }
                | NamedArgsList ',' IdentifiersWithType                             { $$ = std::move($1); $$.push_back(std::move($3)); }
    ;

    VariadicNamedArgument: IDENTIFIER VariadicType                                  { $$ = std::make_unique<IdentifiersWithType>(MakeList<IdentifiersList>( std::move($1) ), std::move($2)); }
    ;

    VariadicType: VARIADIC Type                                                     { $$ = std::move($2); $$ -> isVariadic = true; }
    ;

// Variable Declarations
    VariableDecl: VAR VariableSpec                                                  { $$ = MakeList<DeclarationList>(std::move($2)); }
                | VAR '(' VariableSpecMoreTwo ')'                                   { $$ = std::move($3); }
                | VAR '(' VariableSpecMoreTwo SCs ')'                               { $$ = std::move($3); }
                | VAR '(' VariableSpec ')'                                          { $$ = MakeList<DeclarationList>(std::move($3)); }
                | VAR '(' VariableSpec SCs ')'                                      { $$ = MakeList<DeclarationList>(std::move($3)); }
                | VAR '(' ')'                                                       { $$ = MakeList<DeclarationList>(); }
    ;

    VariableSpecMoreTwo: VariableSpecMoreTwo SCs VariableSpec                       { $$ = std::move($1); $$.push_back(std::move($3)); }
                | VariableSpec SCs VariableSpec                                     { $$ = MakeList<DeclarationList>(std::move($1), std::move($3)); }
	;

    VariableSpec: IdentifiersWithType '=' ExpressionList                            { $$ = std::make_unique<VariableDeclaration>(std::move($1), std::move($3), false); }
		| IdentifiersWithType                                                       { $$ = std::make_unique<VariableDeclaration>(std::move($1), MakeList<ExpressionList>(), false); }
		| IdentifiersList '=' ExpressionList                                        { $$ = std::make_unique<VariableDeclaration>(std::make_unique<IdentifiersWithType>(std::move($1), nullptr), std::move($3), false); }
	;

// Constants Declarations
    ConstDecl: CONST ConstSpec                                                      { $$ = MakeList<DeclarationList>(std::move($2)); }
                | CONST '(' ConstSpecMoreTwo ')'                                    { $$ = std::move($3); }
                | CONST '(' ConstSpecMoreTwo SCs ')'                                { $$ = std::move($3); }
                | CONST '(' ConstSpec ')'                                           { $$ = MakeList<DeclarationList>(std::move($3)); }
                | CONST '(' ConstSpec SCs ')'                                       { $$ = MakeList<DeclarationList>(std::move($3)); }
                | CONST '(' ')'                                                     { $$ = MakeList<DeclarationList>(); }
    ;


    ConstSpecMoreTwo: ConstSpec SCs ConstSpec                                       { $$ = MakeList<DeclarationList>(std::move($1), std::move($3)); }
                | ConstSpecMoreTwo SCs ConstSpec                                    { $$ = std::move($1); $$.push_back(std::move($3)); }
    ;

    ConstSpec: IdentifiersList IDENTIFIER '=' ExpressionList                        { $$ = std::make_unique<VariableDeclaration>(std::make_unique<IdentifiersWithType>(std::move($1), std::make_unique<IdentifierAsType>(std::move($2))), std::move($4), true); }
                | IdentifiersList '=' ExpressionList                                { $$ = std::make_unique<VariableDeclaration>(std::make_unique<IdentifiersWithType>(std::move($1), nullptr), std::move($3), true); }
    ;

// Function declarations
    FunctionDecl: FUNC IDENTIFIER Signature Block                                   { $$ = std::make_unique<FunctionDeclaration>(std::move($2), std::move($3), std::move($4)); }
                | FUNC IDENTIFIER Signature                                         { $$ = std::make_unique<FunctionDeclaration>(std::move($2), std::move($3), nullptr); }
    ;

// Method declaration
    MethodDecl: FUNC '(' IDENTIFIER '*' IDENTIFIER ')' IDENTIFIER Signature Block  {
                                                                                        auto type = std::make_unique<IdentifierAsType>(std::move($5));
                                                                                        type->isPointer = true;
                                                                                        $$ = std::make_unique<MethodDeclaration>(std::move($7), std::move($3), std::move(type), std::move($8), std::move($9));
                                                                                    }
                | FUNC '(' IDENTIFIER '*' IDENTIFIER ')' IDENTIFIER Signature      {
                                                                                        auto type = std::make_unique<IdentifierAsType>(std::move($5));
                                                                                        type->isPointer = true;
                                                                                        $$ = std::make_unique<MethodDeclaration>(std::move($7), std::move($3), std::move(type), std::move($8), nullptr);
                                                                                    }
                | FUNC '(' IDENTIFIER IDENTIFIER ')' IDENTIFIER Signature Block    { $$ = std::make_unique<MethodDeclaration>(std::move($6), std::move($3), std::make_unique<IdentifierAsType>(std::move($4)), std::move($7), std::move($8)); }
                | FUNC '(' IDENTIFIER IDENTIFIER ')' IDENTIFIER Signature          { $$ = std::make_unique<MethodDeclaration>(std::move($6), std::move($3), std::make_unique<IdentifierAsType>(std::move($4)), std::move($7), nullptr); }
    ;

// Type declaration
    TypeDecl: TYPE TypeDef                                                          { $$ = MakeList<DeclarationList>(std::move($2)); }
                | TYPE '(' TypeDefMoreTwo ')'                                       { $$ = std::move($3); }
                | TYPE '(' TypeDefMoreTwo SCs ')'                                   { $$ = std::move($3); }
                | TYPE '(' TypeDef ')'                                              { $$ = MakeList<DeclarationList>(std::move($3)); }
                | TYPE '(' TypeDef SCs ')'                                          { $$ = MakeList<DeclarationList>(std::move($3)); }
                | TYPE '(' ')'                                                      { $$ = MakeList<DeclarationList>(); }
    ;

    TypeDefMoreTwo: TypeDef SCs TypeDef                                             { $$ = MakeList<DeclarationList>(std::move($1), std::move($3)); }
                | TypeDefMoreTwo SCs TypeDef                                        { $$ = std::move($1); $$.push_back(std::move($3)); }
    ;

    TypeDef: IDENTIFIER Type                                                        { $$ = std::make_unique<TypeDeclaration>(std::move($1), std::move($2)); }
    ;

/* -------------------------------- Expressions -------------------------------- */

    Operand: FunctionLiteral                                                        { $$ = std::move($1); }
                | CompositeLiteral                                                  { $$ = std::move($1); }
                | IDENTIFIER                                                        { $$ = std::make_unique<IdentifierAsExpression>(std::move($1)); }
                | '(' Expression ')'                                                { $$ = std::move($2); }
                | IDENTIFIER CompositeLiteralBody                                   { $$ = std::make_unique<CompositeLiteral>(std::make_unique<IdentifierAsType>(std::move($1)), std::move($2)); }
    ;


    BasicLiteral: INT_LIT                                                           { $$ = std::make_unique<IntegerExpression>(std::move($1));       }
		| RUNE_LIT                                                          { $$ = std::make_unique<IntegerExpression>(std::move($1));       }
		| FLOAT_LIT                                                         { $$ = std::make_unique<FloatExpression>(std::move($1));         }
                | STRING_LIT                                                        { $$ = std::make_unique<StringExpression>(std::move($1));        }
                | FALSE                                                             { $$ = std::make_unique<BooleanExpression>(false);    }
                | TRUE                                                              { $$ = std::make_unique<BooleanExpression>(true);     }
                | NIL                                                               { $$ = std::make_unique<NilExpression>();             }
    ;

// Composite literals
    CompositeLiteral: SliceDeclType CompositeLiteralBody                            { $$ = std::make_unique<CompositeLiteral>(std::move($1), std::move($2));   }
                | ArrayDeclType CompositeLiteralBody                                { $$ = std::make_unique<CompositeLiteral>(std::move($1), std::move($2));   }
                | StructType CompositeLiteralBody                                   { throw jgolang::Parser::syntax_error(@$, "Structs are not supported yet");                                         }
    ;

    CompositeLiteralBody: '{' ElementList '}'                                       { $$ = std::move($2); }
                | '{' ElementList ',' '}'                                           { $$ = std::move($2); }
                | '{' '}'                                                           { $$ = MakeList<ElementCompositeLiteralList>(); }
    ;

    ElementList: KeyedElement                                                       { $$ = MakeList<ElementCompositeLiteralList>(std::move($1)); }
                | ElementList ',' KeyedElement                                      { $$ = std::move($1); $$.push_back(std::move($3)); }
    ;

    KeyedElement: Expression ':' Expression                                         { $$ = std::make_unique<ElementCompositeLiteral>(std::move($1), std::move($3)); }
                | Expression                                                        { $$ = std::make_unique<ElementCompositeLiteral>(nullptr, std::move($1)); }
                | Expression ':' CompositeLiteralBody                               { $$ = std::make_unique<ElementCompositeLiteral>(std::move($1), std::move($3)); }
                | CompositeLiteralBody                                              { $$ = std::make_unique<ElementCompositeLiteral>(nullptr, std::move($1)); }
    ;

    FunctionLiteral: FUNC Signature Block                                           { $$ = std::make_unique<FunctionLitExpression>(std::move($2), std::move($3)); }
    ;

    Expression: AccessExpression                                                    { $$ = std::move($1); }
                | BasicLiteral                                                      { $$ = std::move($1); }
                | Expression '+' Expression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Addition, std::move($1), std::move($3));        }
                | Expression '-' Expression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Subtraction, std::move($1), std::move($3));     }
                | Expression '*' Expression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Multiplication, std::move($1), std::move($3));  }
                | Expression '/' Expression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Division, std::move($1), std::move($3));        }
                | Expression '%' Expression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Mod, std::move($1), std::move($3));             }
                | Expression '<' Expression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Less, std::move($1), std::move($3));            }
                | Expression '>' Expression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Greater, std::move($1), std::move($3));         }
                | Expression EQUAL Expression                                       { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Equal, std::move($1), std::move($3));           }
                | Expression NOT_EQUAL Expression                                   { $$ = std::make_unique<BinaryExpression>(BinaryExpression::NotEqual, std::move($1), std::move($3));        }
                | Expression LESS_OR_EQUAL Expression                               { $$ = std::make_unique<BinaryExpression>(BinaryExpression::LessOrEqual, std::move($1), std::move($3));     }
                | Expression GREATER_OR_EQUAL Expression                            { $$ = std::make_unique<BinaryExpression>(BinaryExpression::GreatOrEqual, std::move($1), std::move($3));    }
                | Expression AND Expression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::And, std::move($1), std::move($3));             }
                | Expression OR Expression                                          { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Or, std::move($1), std::move($3));              }
                | '+' Expression %prec UNARY_PLUS                                   { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryPlus, std::move($2));             }
                | '-' Expression %prec UNARY_MINUS                                  { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryMinus, std::move($2));            }
                | '!' Expression                                                    { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryNot, std::move($2));              }
                | Expression VARIADIC                                               { $$ = std::make_unique<UnaryExpression>(UnaryExpression::Variadic, std::move($1));              }
    ;

    AccessExpression: Operand                                                       { $$ = std::move($1); }
                | AccessExpression '[' Expression ']'                               { $$ = std::make_unique<AccessExpression>(AccessExpression::Indexing, std::move($1), std::move($3)); }
                | AccessExpression '.' AccessExpression                             { $$ = std::make_unique<AccessExpression>(AccessExpression::FieldSelect, std::move($1), std::move($3)); }
                | AccessExpression '[' ':' ']'                                      { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | AccessExpression '[' Expression ':' ']'                           { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | AccessExpression '[' ':' Expression ']'                           { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | AccessExpression '[' Expression ':' Expression ']'                { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | AccessExpression Arguments                                        { $$ = std::make_unique<CallableExpression>(std::move($1), std::move($2)); }
    ;

    Arguments: '(' ExpressionList ',' ')'                                           { $$ = std::move($2); }
		        | '(' ExpressionList ')'                                            { $$ = std::move($2); }
                | '(' ')'                                                           { $$ = MakeList<ExpressionList>(); }
	;

    ExpressionList: Expression                                                      { $$ = MakeList<ExpressionList>(std::move($1)); }
		| ExpressionList ',' Expression                                             { $$ = std::move($1); $$.push_back(std::move($3)); }
    ;

/* -------------------------------- Statements -------------------------------- */

    Assignment: Expression PLUS_ASSIGNMENT Expression                               { $$ = std::make_unique<AssignmentStatement>(AssignmentStatement::PlusAssign, std::move($1), std::move($3));     }
                | Expression MINUS_ASSIGNMENT Expression                            { $$ = std::make_unique<AssignmentStatement>(AssignmentStatement::MinusAssign, std::move($1), std::move($3));    }
                | Expression MUL_ASSIGNMENT Expression                              { $$ = std::make_unique<AssignmentStatement>(AssignmentStatement::MulAssign, std::move($1), std::move($3));      }
                | Expression DIV_ASSIGNMENT Expression                              { $$ = std::make_unique<AssignmentStatement>(AssignmentStatement::DivAssign, std::move($1), std::move($3));      }
                | Expression MOD_ASSIGNMENT Expression                              { $$ = std::make_unique<AssignmentStatement>(AssignmentStatement::ModAssign, std::move($1), std::move($3));      }
                | ExpressionList '=' ExpressionList                                 { $$ = std::make_unique<AssignmentStatement>(AssignmentStatement::SimpleAssign, std::move($1), std::move($3)); }
    ;

    ShortVarDecl: ExpressionList SHORT_DECL_OP ExpressionList                       {
                                                                                        auto temp = IdentifiersListFromExpressions($1);

                                                                                        if (!temp) {
                                                                                            throw jgolang::Parser::syntax_error(@$, "Lhs of short declaration must contains only identifiers");

                                                                                        } else {
                                                                                            $$ = std::make_unique<ShortVarDeclarationStatement>(std::move(*temp), std::move($3));
                                                                                        }
                                                                                    }
    ;

    Statement: Declaration                                                          { $$ = std::make_unique<DeclarationStatement>(std::move($1)); }
                | Block                                                             { $$ = std::move($1); }
                | SimpleStmt                                                        { $$ = std::move($1); }
                | ReturnStmt                                                        { $$ = std::move($1); }
                | BREAK                                                             { $$ = std::make_unique<KeywordStatement>(KeywordStatement::Break);        }
                | CONTINUE                                                          { $$ = std::make_unique<KeywordStatement>(KeywordStatement::Continue);     }
                | FALLTHROUGH                                                       { $$ = std::make_unique<KeywordStatement>(KeywordStatement::Fallthrough);  }
                | IfStmt                                                            { $$ = std::move($1); }
                | ForStmt                                                           { $$ = std::move($1); }
                | SwitchStmt                                                        { $$ = std::move($1); }
    ;


    SimpleStmt: Expression                                                          { $$ = std::make_unique<ExpressionStatement>(std::move($1)); }
                | Assignment                                                        { $$ = std::move($1); }
                | ShortVarDecl                                                      { $$ = std::move($1); }
                | Expression INCREMENT                                              { $$ = std::make_unique<ExpressionStatement>(std::make_unique<UnaryExpression>(UnaryExpression::Increment, std::move($1))); }
                | Expression DECREMENT                                              { $$ = std::make_unique<ExpressionStatement>(std::make_unique<UnaryExpression>(UnaryExpression::Decrement, std::move($1))); }
    ;

    // Return statements
    ReturnStmt: RETURN ExpressionList                                               { $$ = std::make_unique<ReturnStatement>(std::move($2)); }
                | RETURN                                                            { $$ = std::make_unique<ReturnStatement>(MakeList<ExpressionList>()); }
    ;

/* -------------------------------- Blocks -------------------------------- */

    Block: '{' StatementMoreTwo '}'                                                 { $$ = std::make_unique<BlockStatement>(std::move($2));     }
                | '{' StatementMoreTwo SCs '}'                                      { $$ = std::make_unique<BlockStatement>(std::move($2));     }
                | '{' Statement '}'                                                 { $$ = std::make_unique<BlockStatement>(std::move($2));      }
                | '{' Statement SCs '}'                                             { $$ = std::make_unique<BlockStatement>(std::move($2));      }
                | '{' '}'                                                           { $$ = std::make_unique<BlockStatement>();        }
    ;

    StatementMoreTwo: Statement SCs Statement                                       { $$ = MakeList<StatementList>(std::move($1), std::move($3)); }
                | StatementMoreTwo SCs Statement                                    { $$ = std::move($1); $$.push_back(std::move($3)); }
    ;

    // At the outer level of a control header, an identifier followed by '{'
    // starts the block. Parentheses, arguments and indices use ordinary expressions.
    HeaderOperand: FunctionLiteral                                                  { $$ = std::move($1); }
                | CompositeLiteral                                                  { $$ = std::move($1); }
                | IDENTIFIER                                                        { $$ = std::make_unique<IdentifierAsExpression>(std::move($1)); }
                | '(' Expression ')'                                                { $$ = std::move($2); }
    ;

    HeaderExpression: HeaderAccessExpression                                                    { $$ = std::move($1); }
                | BasicLiteral                                                                  { $$ = std::move($1); }
                | HeaderExpression '+' HeaderExpression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Addition, std::move($1), std::move($3));        }
                | HeaderExpression '-' HeaderExpression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Subtraction, std::move($1), std::move($3));     }
                | HeaderExpression '*' HeaderExpression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Multiplication, std::move($1), std::move($3));  }
                | HeaderExpression '/' HeaderExpression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Division, std::move($1), std::move($3));        }
                | HeaderExpression '%' HeaderExpression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Mod, std::move($1), std::move($3));             }
                | HeaderExpression '<' HeaderExpression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Less, std::move($1), std::move($3));            }
                | HeaderExpression '>' HeaderExpression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Greater, std::move($1), std::move($3));         }
                | HeaderExpression EQUAL HeaderExpression                                       { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Equal, std::move($1), std::move($3));           }
                | HeaderExpression NOT_EQUAL HeaderExpression                                   { $$ = std::make_unique<BinaryExpression>(BinaryExpression::NotEqual, std::move($1), std::move($3));        }
                | HeaderExpression LESS_OR_EQUAL HeaderExpression                               { $$ = std::make_unique<BinaryExpression>(BinaryExpression::LessOrEqual, std::move($1), std::move($3));     }
                | HeaderExpression GREATER_OR_EQUAL HeaderExpression                            { $$ = std::make_unique<BinaryExpression>(BinaryExpression::GreatOrEqual, std::move($1), std::move($3));    }
                | HeaderExpression AND HeaderExpression                                         { $$ = std::make_unique<BinaryExpression>(BinaryExpression::And, std::move($1), std::move($3));             }
                | HeaderExpression OR HeaderExpression                                          { $$ = std::make_unique<BinaryExpression>(BinaryExpression::Or, std::move($1), std::move($3));              }
                | '+' HeaderExpression %prec UNARY_PLUS                                   { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryPlus, std::move($2));             }
                | '-' HeaderExpression %prec UNARY_MINUS                                  { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryMinus, std::move($2));            }
                | '!' HeaderExpression                                                    { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryNot, std::move($2));              }
                | HeaderExpression VARIADIC                                               { $$ = std::make_unique<UnaryExpression>(UnaryExpression::Variadic, std::move($1));              }
    ;

    HeaderAccessExpression: HeaderOperand                                                 { $$ = std::move($1); }
                | HeaderAccessExpression '[' Expression ']'                               { $$ = std::make_unique<AccessExpression>(AccessExpression::Indexing, std::move($1), std::move($3)); }
                | HeaderAccessExpression '.' HeaderAccessExpression                       { $$ = std::make_unique<AccessExpression>(AccessExpression::FieldSelect, std::move($1), std::move($3)); }
                | HeaderAccessExpression '[' ':' ']'                                      { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | HeaderAccessExpression '[' Expression ':' ']'                           { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | HeaderAccessExpression '[' ':' Expression ']'                           { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | HeaderAccessExpression '[' Expression ':' Expression ']'                { throw jgolang::Parser::syntax_error(@$, "array slices are not supported yet"); }
                | HeaderAccessExpression Arguments                                        { $$ = std::make_unique<CallableExpression>(std::move($1), std::move($2)); }
    ;

    HeaderExpressionOptional: /* empty */                                                 { $$ = nullptr; }
                | HeaderExpression                                                        { $$ = std::move($1); }
    ;

    HeaderExpressionList: HeaderExpression                                                      { $$ = MakeList<ExpressionList>(std::move($1)); }
		| HeaderExpressionList ',' HeaderExpression                                             { $$ = std::move($1); $$.push_back(std::move($3)); }
    ;

    HeaderAssignment: HeaderExpression PLUS_ASSIGNMENT HeaderExpression                               { $$ = std::make_unique<AssignmentStatement>(AssignmentStatement::PlusAssign, std::move($1), std::move($3));     }
                | HeaderExpression MINUS_ASSIGNMENT HeaderExpression                            { $$ = std::make_unique<AssignmentStatement>(AssignmentStatement::MinusAssign, std::move($1), std::move($3));    }
                | HeaderExpression MUL_ASSIGNMENT HeaderExpression                              { $$ = std::make_unique<AssignmentStatement>(AssignmentStatement::MulAssign, std::move($1), std::move($3));      }
                | HeaderExpression DIV_ASSIGNMENT HeaderExpression                              { $$ = std::make_unique<AssignmentStatement>(AssignmentStatement::DivAssign, std::move($1), std::move($3));      }
                | HeaderExpression MOD_ASSIGNMENT HeaderExpression                              { $$ = std::make_unique<AssignmentStatement>(AssignmentStatement::ModAssign, std::move($1), std::move($3));      }
                | HeaderExpressionList '=' HeaderExpressionList                                 { $$ = std::make_unique<AssignmentStatement>(AssignmentStatement::SimpleAssign, std::move($1), std::move($3)); }
    ;

    HeaderShortVarDecl: HeaderExpressionList SHORT_DECL_OP HeaderExpressionList     {
                                                                                        auto temp = IdentifiersListFromExpressions($1);

                                                                                        if (!temp) {
                                                                                            throw jgolang::Parser::syntax_error(@$, "Lhs of short declaration must contains only identifiers");

                                                                                        } else {
                                                                                            $$ = std::make_unique<ShortVarDeclarationStatement>(std::move(*temp), std::move($3));
                                                                                        }
                                                                                    }
    ;

    HeaderSimpleStmt: HeaderExpression                                                          { $$ = std::make_unique<ExpressionStatement>(std::move($1)); }
                | HeaderAssignment                                                        { $$ = std::move($1); }
                | HeaderShortVarDecl                                                      { $$ = std::move($1); }
                | HeaderExpression INCREMENT                                              { $$ = std::make_unique<ExpressionStatement>(std::make_unique<UnaryExpression>(UnaryExpression::Increment, std::move($1))); }
                | HeaderExpression DECREMENT                                              { $$ = std::make_unique<ExpressionStatement>(std::make_unique<UnaryExpression>(UnaryExpression::Decrement, std::move($1))); }
    ;

    IfStmt: IF HeaderSimpleStmt ';' HeaderExpression Block ELSE IfStmt   { $$ = std::make_unique<IfStatement>(std::move($2), std::move($4), std::move($5), std::move($7)); }
        | IF HeaderExpression Block ELSE IfStmt                          { $$ = std::make_unique<IfStatement>(nullptr, std::move($2), std::move($3), std::move($5)); }
        | IF HeaderSimpleStmt ';' HeaderExpression Block ELSE Block      { $$ = std::make_unique<IfStatement>(std::move($2), std::move($4), std::move($5), std::move($7)); }
        | IF HeaderExpression Block ELSE Block                           { $$ = std::make_unique<IfStatement>(nullptr, std::move($2), std::move($3), std::move($5)); }
        | IF HeaderSimpleStmt ';' HeaderExpression Block                 { $$ = std::make_unique<IfStatement>(std::move($2), std::move($4), std::move($5), nullptr); }
        | IF HeaderExpression Block                                      { $$ = std::make_unique<IfStatement>(nullptr, std::move($2), std::move($3), nullptr); }
    ;

    ForStmt: FOR HeaderExpression Block                                  { $$ = std::make_unique<WhileStatement>(std::move($2), std::move($3)); }
        | FOR HeaderSimpleStmt ';' HeaderExpressionOptional ';' HeaderSimpleStmt Block
                                                                         { $$ = std::make_unique<ForStatement>(std::move($2), std::move($4), std::move($6), std::move($7)); }
        | FOR HeaderExpressionList '=' RANGE HeaderExpression Block      { $$ = std::make_unique<ForRangeStatement>(std::move($2), std::move($5), std::move($6), false); }
        | FOR HeaderExpressionList SHORT_DECL_OP RANGE HeaderExpression Block
                                                                         { $$ = std::make_unique<ForRangeStatement>(std::move($2), std::move($5), std::move($6), true); }
        | FOR RANGE HeaderExpression Block                               { $$ = std::make_unique<ForRangeStatement>(MakeList<ExpressionList>(), std::move($3), std::move($4), false); }
        | FOR Block                                                      { $$ = std::make_unique<WhileStatement>(std::make_unique<BooleanExpression>(true), std::move($2)); }
    ;

    SwitchStmt: SWITCH HeaderSimpleStmt ';' HeaderExpressionOptional '{' ExprCaseOrDefaultClauseListOrEmpty '}'
                                                                         { $$ = std::make_unique<SwitchStatement>(std::move($2), std::move($4), std::move($6)); }
        | SWITCH HeaderExpressionOptional '{' ExprCaseOrDefaultClauseListOrEmpty '}'
                                                                         { $$ = std::make_unique<SwitchStatement>(nullptr, std::move($2), std::move($4)); }
    ;

    ExprCaseOrDefaultClause: CASE Expression ':' StatementMoreTwo  SCs              { $$ = std::make_unique<SwitchCaseClause>(std::move($2), std::make_unique<BlockStatement>(std::move($4)));       }
                | CASE Expression ':' Statement SCs                                 { $$ = std::make_unique<SwitchCaseClause>(std::move($2), std::make_unique<BlockStatement>(std::move($4)));        }
                | CASE Expression ':'                                               { $$ = std::make_unique<SwitchCaseClause>(std::move($2), std::make_unique<BlockStatement>());          }
                | DEFAULT ':' StatementMoreTwo SCs                                  { $$ = std::make_unique<SwitchCaseClause>(nullptr, std::make_unique<BlockStatement>(std::move($3)));  }
                | DEFAULT ':' Statement SCs                                         { $$ = std::make_unique<SwitchCaseClause>(nullptr, std::make_unique<BlockStatement>(std::move($3)));   }
    ;

    ExprCaseOrDefaultClauseList: ExprCaseOrDefaultClause                            { $$ = MakeList<SwitchCaseList>(std::move($1)); }
                | ExprCaseOrDefaultClauseList ExprCaseOrDefaultClause               { $$ = std::move($1); $$.push_back(std::move($2)); }
    ;

    ExprCaseOrDefaultClauseListOrEmpty: /* empty */                                 { $$ = MakeList<SwitchCaseList>(); }
                | ExprCaseOrDefaultClauseList                                       { $$ = std::move($1); }
    ;

    SCs: ';'                                                                        { }
                | SCs ';'                                                           { }
%%
