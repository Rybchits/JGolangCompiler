%{
    #include "ast.h"
    #include "lex.yy.cc"

    yyFlexLexer* lexer;
    PackageAST *Root;

    int yylex() {
        return lexer->yylex();
    }

    void yyerror(char const *s) {
        fprintf(stderr, "Error: %s", s);
        exit(1);
    }
%}


%union {
    long long integerVal;
    char* stringVal;
    char* identifierVal;
    double floatVal;
    int32_t runeVal;

    TypeAST *typeNode;
    IdentifiersWithType *typedIdentifiers;
    DeclarationAST *declarationNode;
    StatementAST *statementNode;
    BlockStatement *blockStatementNode;
    ExpressionAST *expressionNode;
    VariableDeclaration *varDecl;
    ElementCompositeLiteral *elementCompositLiteral;
    SwitchCaseClause *switchCaseClause;
    FunctionSignature *functionSignature;
    ShortVarDeclarationStatement *shortDeclaration;
    
    DeclarationList *declarationList;
    FunctionList *functionList;
    ExpressionList *expressionList;
    StatementList *statementList;
    IdentifiersList *identifierList;
    SwitchCaseList *switchCaseClauseList;
    TypeList *typeList;
    ElementCompositeLiteralList *elementsCompositeLiteral;
    std::list<IdentifiersWithType *> *identifiersWithTypeList; 
}

%type <identifierList> IdentifiersList
%type <functionList> InterfaceMembersMoreTwo
%type <typeList> TypesWithIdentifiersList
%type <expressionList> ExpressionList Arguments ExprWithWrapCompositeObjList
%type <statementList> StatementMoreTwo ExprDefaultClauseOptional
%type <switchCaseClauseList> ExprCaseClauseList ExprCaseClauseListOrEmpty
%type <declarationList> Declaration TopLevelDecl TopLevelDeclList TopLevelDeclListOrEmpty
%type <identifiersWithTypeList> FieldDeclMoreTwo Result Parameters NamedArgsList
%type <elementsCompositeLiteral> CompositeLiteralBody ElementList
%type <declarationList> VariableDecl VariableSpecMoreTwo ConstDecl ConstSpecMoreTwo TypeDefMoreTwo TypeDecl

%type <blockStatementNode> Block
%type <functionSignature> Signature
%type <switchCaseClause> ExprCaseClause
%type <elementCompositLiteral> KeyedElement
%type <varDecl> VariableSpec ConstSpec
%type <typedIdentifiers> IdentifiersWithType VariadicNamedArgument
%type <declarationNode> FunctionDecl MethodDecl TypeDef
%type <shortDeclaration> ShortVarDecl ShortVarDeclWithWrapCompositeObj
%type <statementNode> Statement SimpleStmt Assignment IncDecStmt ReturnStmt IfStmt ForStmt SwitchStmt
%type <statementNode> SimpleStmtWithExprWrapCompositeObj SimpleStmtWithWrapCompositeObjOptional AssignmentWithWrapCompositeObj IncDecStmtWithWrapCompositeObj
%type <expressionNode> Expression ExprWithWrapCompositeObjOptional Operand BasicLiteral CompositeLiteral FunctionLiteral AccessExpression
%type <expressionNode> OperandWithCompositeObj ObjectCompositeLit OperandWithWrapCompositeObj AccessExprWithWrapCompositeObj ExprWithWrapCompositeObj
%type <typeNode> Type TypeOnly LiteralType StructType SliceDeclType ArrayDeclType FunctionType VariadicType InterfaceType


%token <integerVal>INT_LIT
%token <floatVal> FLOAT_LIT
%token <runeVal> RUNE_LIT
%token <stringVal> STRING_LIT
%token <identifierVal> IDENTIFIER

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
    Root: PACKAGE IDENTIFIER SCs TopLevelDeclListOrEmpty                           { Root = new PackageAST($2, *$4); }
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
                | FieldDeclMoreTwo SCs IdentifiersWithType                             { $$ = $1; $$ -> push_back($3); }
                | FieldDeclMoreTwo SCs IDENTIFIER                                      { $$ = $1; $$ -> push_back(new IdentifiersWithType(*(new IdentifiersList({""})), new IdentifierAsType($3))); }

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

    SliceDeclType: '[' ']' Type                                                     { $$ = new ArraySignature($3); }
    ;

    ArrayDeclType: '[' INT_LIT ']' Type                                             { $$ = new ArraySignature($4, $2); }
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

    Result: '(' ')'                                                                 { yyerror("Many returns are not supported yet"); }
                | '(' NamedArgsList ')'                                             { yyerror("Many returns are not supported yet"); }
                | '(' NamedArgsList ',' ')'                                         { yyerror("Many returns are not supported yet"); }
                | '(' TypesWithIdentifiersList ')'                                  { yyerror("Many returns are not supported yet"); }
                | '(' IdentifiersList ')'                                           { yyerror("Many returns are not supported yet"); }
                | '(' TypesWithIdentifiersList ',' ')'                              { yyerror("Many returns are not supported yet"); }
                | '(' IdentifiersList ',' ')'                                       { yyerror("Many returns are not supported yet"); }

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

    ConstSpec: IdentifiersWithType '=' ExpressionList                               { $$ = new VariableDeclaration($1, *$3, true); }
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
    ;

    OperandWithCompositeObj: Operand                                                { $$ = $1; }
                | ObjectCompositeLit                                                { $$ = $1; }
                | '(' Expression ')'                                                { $$ = $2; }
    ;

    ObjectCompositeLit: IDENTIFIER CompositeLiteralBody                             { $$ = new CompositeLiteral(new IdentifierAsType($1), *$2); }
    ;

    BasicLiteral: INT_LIT                                                           { $$ = new IntegerExpression($1);     }
		        | FLOAT_LIT                                                         { $$ = new FloatExpression($1);       }
		        | RUNE_LIT                                                          { $$ = new RuneExpression($1);        }
                | STRING_LIT                                                        { $$ = new StringExpression($1);      }
                | FALSE                                                             { $$ = new BooleanExpression(false);  }
                | TRUE                                                              { $$ = new BooleanExpression(true);   }
                | NIL                                                               { $$ = new NilExpression();           }
    ;

// Composite literals
    CompositeLiteral: SliceDeclType CompositeLiteralBody                            { $$ = new CompositeLiteral($1, *$2); }
                | ArrayDeclType CompositeLiteralBody                                { $$ = new CompositeLiteral($1, *$2); }
                | StructType CompositeLiteralBody                                   { $$ = new CompositeLiteral($1, *$2); }
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
                | Expression '+' Expression                                         { $$ = new BinaryExpression(BinaryExpressionEnum::Addition, $1, $3);        }
                | Expression '-' Expression                                         { $$ = new BinaryExpression(BinaryExpressionEnum::Subtraction, $1, $3);     }
                | Expression '*' Expression                                         { $$ = new BinaryExpression(BinaryExpressionEnum::Multiplication, $1, $3);  }
                | Expression '/' Expression                                         { $$ = new BinaryExpression(BinaryExpressionEnum::Division, $1, $3);        }
                | Expression '%' Expression                                         { $$ = new BinaryExpression(BinaryExpressionEnum::Mod, $1, $3);             }
                | Expression '<' Expression                                         { $$ = new BinaryExpression(BinaryExpressionEnum::Less, $1, $3);            }
                | Expression '>' Expression                                         { $$ = new BinaryExpression(BinaryExpressionEnum::Greater, $1, $3);         }
                | Expression EQUAL Expression                                       { $$ = new BinaryExpression(BinaryExpressionEnum::Equal, $1, $3);           }
                | Expression NOT_EQUAL Expression                                   { $$ = new BinaryExpression(BinaryExpressionEnum::NotEqual, $1, $3);        }
                | Expression LESS_OR_EQUAL Expression                               { $$ = new BinaryExpression(BinaryExpressionEnum::LessOrEqual, $1, $3);     }
                | Expression GREATER_OR_EQUAL Expression                            { $$ = new BinaryExpression(BinaryExpressionEnum::GreatOrEqual, $1, $3);    }
                | Expression AND Expression                                         { $$ = new BinaryExpression(BinaryExpressionEnum::And, $1, $3);             }
                | Expression OR Expression                                          { $$ = new BinaryExpression(BinaryExpressionEnum::Or, $1, $3);              }
                | '+' Expression %prec UNARY_PLUS                                   { $$ = new UnaryExpression(UnaryExpressionEnum::UnaryPlus, $2);             }
                | '-' Expression %prec UNARY_MINUS                                  { $$ = new UnaryExpression(UnaryExpressionEnum::UnaryMinus, $2);            }
                | '!' Expression                                                    { $$ = new UnaryExpression(UnaryExpressionEnum::UnaryNot, $2);              }
                | Expression VARIADIC                                               { $$ = new UnaryExpression(UnaryExpressionEnum::Variadic, $1);              }
    ;

    AccessExpression: OperandWithCompositeObj                                       { $$ = $1; }
                | AccessExpression '[' Expression ']'                               { $$ = new AccessExpression(AccessExpressionEnum::Indexing, $1, $3); }
                | AccessExpression '.' AccessExpression                             { $$ = new AccessExpression(AccessExpressionEnum::FieldSelect, $1, $3); }
                | AccessExpression '[' ':' ']'                                      { yyerror("array slices are not supported yet"); }
                | AccessExpression '[' Expression ':' ']'                           { yyerror("array slices are not supported yet"); }
                | AccessExpression '[' ':' Expression ']'                           { yyerror("array slices are not supported yet"); }
                | AccessExpression '[' Expression ':' Expression ']'                { yyerror("array slices are not supported yet"); }
                | AccessExpression Arguments                                        { $$ = new CallableExpression($1, *$2); }
    ;

    Arguments: '(' ExpressionList ',' ')'                                           { $$ = $2; }
		        | '(' ExpressionList ')'                                            { $$ = $2; }
                | '(' ')'                                                           { $$ = new ExpressionList(); }
	;

    ExpressionList: Expression                                                      { $$ = new ExpressionList({$1}); }
		| ExpressionList ',' Expression                                             { $$ = $1; $$ -> push_back($3); }
    ;

/* ---------------- Special Expressions For Using Composite Literal In Constructions --------------------- */

    OperandWithWrapCompositeObj: Operand                                            { $$ = $1; }
                | '(' ObjectCompositeLit ')'                                        { $$ = $2; }
                | '(' ExprWithWrapCompositeObj ')'                                  { $$ = $2; }
    ;

    AccessExprWithWrapCompositeObj: OperandWithWrapCompositeObj                     { $$ = $1; }
                | AccessExprWithWrapCompositeObj '[' ExprWithWrapCompositeObj ']'                               { $$ = new AccessExpression(AccessExpressionEnum::Indexing, $1, $3); }
                | AccessExprWithWrapCompositeObj '.' AccessExprWithWrapCompositeObj                             { $$ = new AccessExpression(AccessExpressionEnum::FieldSelect, $1, $3); }
                | AccessExprWithWrapCompositeObj '[' ':' ']'                                                    { yyerror("array slices are not supported yet"); }
                | AccessExprWithWrapCompositeObj '[' ExprWithWrapCompositeObj ':' ']'                           { yyerror("array slices are not supported yet"); }
                | AccessExprWithWrapCompositeObj '[' ':' ExprWithWrapCompositeObj ']'                           { yyerror("array slices are not supported yet"); }
                | AccessExprWithWrapCompositeObj '[' ExprWithWrapCompositeObj ':' ExprWithWrapCompositeObj ']'  { yyerror("array slices are not supported yet"); }
                | AccessExprWithWrapCompositeObj Arguments                                                      { $$ = new CallableExpression($1, *$2); }
    ;

    ExprWithWrapCompositeObj: AccessExprWithWrapCompositeObj                            { $$ = $1; }
                | BasicLiteral                                                          { $$ = $1; }
                | ExprWithWrapCompositeObj '+' ExprWithWrapCompositeObj                 { $$ = new BinaryExpression(BinaryExpressionEnum::Addition, $1, $3);        }
                | ExprWithWrapCompositeObj '-' ExprWithWrapCompositeObj                 { $$ = new BinaryExpression(BinaryExpressionEnum::Subtraction, $1, $3);     }
                | ExprWithWrapCompositeObj '*' ExprWithWrapCompositeObj                 { $$ = new BinaryExpression(BinaryExpressionEnum::Multiplication, $1, $3);  }
                | ExprWithWrapCompositeObj '/' ExprWithWrapCompositeObj                 { $$ = new BinaryExpression(BinaryExpressionEnum::Division, $1, $3);        }
                | ExprWithWrapCompositeObj '%' ExprWithWrapCompositeObj                 { $$ = new BinaryExpression(BinaryExpressionEnum::Mod, $1, $3);             }
                | ExprWithWrapCompositeObj '<' ExprWithWrapCompositeObj                 { $$ = new BinaryExpression(BinaryExpressionEnum::Less, $1, $3);            }
                | ExprWithWrapCompositeObj '>' ExprWithWrapCompositeObj                 { $$ = new BinaryExpression(BinaryExpressionEnum::Greater, $1, $3);         }
                | ExprWithWrapCompositeObj EQUAL ExprWithWrapCompositeObj               { $$ = new BinaryExpression(BinaryExpressionEnum::Equal, $1, $3);           }
                | ExprWithWrapCompositeObj NOT_EQUAL ExprWithWrapCompositeObj           { $$ = new BinaryExpression(BinaryExpressionEnum::NotEqual, $1, $3);        }
                | ExprWithWrapCompositeObj LESS_OR_EQUAL ExprWithWrapCompositeObj       { $$ = new BinaryExpression(BinaryExpressionEnum::LessOrEqual, $1, $3);     }
                | ExprWithWrapCompositeObj GREATER_OR_EQUAL ExprWithWrapCompositeObj    { $$ = new BinaryExpression(BinaryExpressionEnum::GreatOrEqual, $1, $3);    }
                | ExprWithWrapCompositeObj AND ExprWithWrapCompositeObj                 { $$ = new BinaryExpression(BinaryExpressionEnum::And, $1, $3);             }
                | ExprWithWrapCompositeObj OR ExprWithWrapCompositeObj                  { $$ = new BinaryExpression(BinaryExpressionEnum::Or, $1, $3);              }
                | '+' ExprWithWrapCompositeObj %prec UNARY_PLUS                         { $$ = new UnaryExpression(UnaryExpressionEnum::UnaryPlus, $2);             }
                | '-' ExprWithWrapCompositeObj %prec UNARY_MINUS                        { $$ = new UnaryExpression(UnaryExpressionEnum::UnaryMinus, $2);            }
                | '!' ExprWithWrapCompositeObj                                          { $$ = new UnaryExpression(UnaryExpressionEnum::UnaryNot, $2);              }
    ;

    ExprWithWrapCompositeObjOptional: /* empty */                                   { $$ = nullptr; }
                | ExprWithWrapCompositeObj                                          { $$ = $1; }
    ;

    SimpleStmtWithExprWrapCompositeObj: ExprWithWrapCompositeObj                    { $$ = new ExpressionStatement($1); }
                | AssignmentWithWrapCompositeObj                                    { $$ = $1; }
                | IncDecStmtWithWrapCompositeObj                                    { $$ = $1; }
                | ShortVarDeclWithWrapCompositeObj                                  { $$ = $1; }
    ;

    SimpleStmtWithWrapCompositeObjOptional: /* empty */                             { $$ = nullptr; }
                | SimpleStmtWithExprWrapCompositeObj                                { $$ = $1; }
    ;

    AssignmentWithWrapCompositeObj: ExprWithWrapCompositeObj PLUS_ASSIGNMENT ExprWithWrapCompositeObj       { $$ = new AssignmentStatement(AssignmentEnum::PlusAssign, *(new ExpressionList({$1})), *(new ExpressionList({$3})));     }
                | ExprWithWrapCompositeObj MINUS_ASSIGNMENT ExprWithWrapCompositeObj                        { $$ = new AssignmentStatement(AssignmentEnum::MinusAssign, *(new ExpressionList({$1})), *(new ExpressionList({$3})));    }
                | ExprWithWrapCompositeObj MUL_ASSIGNMENT ExprWithWrapCompositeObj                          { $$ = new AssignmentStatement(AssignmentEnum::MulAssign, *(new ExpressionList({$1})), *(new ExpressionList({$3})));      }
                | ExprWithWrapCompositeObj DIV_ASSIGNMENT ExprWithWrapCompositeObj                          { $$ = new AssignmentStatement(AssignmentEnum::DivAssign, *(new ExpressionList({$1})), *(new ExpressionList({$3})));      }
                | ExprWithWrapCompositeObj MOD_ASSIGNMENT ExprWithWrapCompositeObj                          { $$ = new AssignmentStatement(AssignmentEnum::ModAssign, *(new ExpressionList({$1})), *(new ExpressionList({$3})));      }
                | ExprWithWrapCompositeObjList '=' ExprWithWrapCompositeObjList                             { $$ = new AssignmentStatement(AssignmentEnum::SimpleAssign, *$1, *$3); }
    ;

    ShortVarDeclWithWrapCompositeObj: ExprWithWrapCompositeObjList SHORT_DECL_OP ExprWithWrapCompositeObjList       {
                                                                                                                        IdentifiersList* temp = IdentifiersListFromExpressions(*$1);
                                                                                                                        if (temp == nullptr) {
                                                                                                                            yyerror("Lhs of short declaration must contains only identifiers");
                                                                                                                        } else { 
                                                                                                                            $$ = new ShortVarDeclarationStatement(*temp, *$3);
                                                                                                                        } 
                                                                                                                    }
    ;

    IncDecStmtWithWrapCompositeObj: ExprWithWrapCompositeObj INCREMENT              { $$ = new ExpressionStatement(new UnaryExpression(UnaryExpressionEnum::Increment, $1)); }
                | ExprWithWrapCompositeObj DECREMENT                                { $$ = new ExpressionStatement(new UnaryExpression(UnaryExpressionEnum::Decrement, $1)); }
    ;

    ExprWithWrapCompositeObjList: ExprWithWrapCompositeObj                          { $$ = new ExpressionList({$1}); }
        | ExprWithWrapCompositeObjList ',' ExprWithWrapCompositeObj                 { $$ = $1; $$ -> push_back($3); }
    ;

/* -------------------------------- Statements -------------------------------- */

    Assignment: Expression PLUS_ASSIGNMENT Expression                               { $$ = new AssignmentStatement(AssignmentEnum::PlusAssign, *(new ExpressionList({$1})), *(new ExpressionList({$3})));     }
                | Expression MINUS_ASSIGNMENT Expression                            { $$ = new AssignmentStatement(AssignmentEnum::MinusAssign, *(new ExpressionList({$1})), *(new ExpressionList({$3})));    }
                | Expression MUL_ASSIGNMENT Expression                              { $$ = new AssignmentStatement(AssignmentEnum::MulAssign, *(new ExpressionList({$1})), *(new ExpressionList({$3})));      }
                | Expression DIV_ASSIGNMENT Expression                              { $$ = new AssignmentStatement(AssignmentEnum::DivAssign, *(new ExpressionList({$1})), *(new ExpressionList({$3})));      }
                | Expression MOD_ASSIGNMENT Expression                              { $$ = new AssignmentStatement(AssignmentEnum::ModAssign, *(new ExpressionList({$1})), *(new ExpressionList({$3})));      }
                | ExpressionList '=' ExpressionList                                 { $$ = new AssignmentStatement(AssignmentEnum::SimpleAssign, *$1, *$3); }
    ;

    ShortVarDecl: ExpressionList SHORT_DECL_OP ExpressionList                       {
                                                                                        IdentifiersList* temp = IdentifiersListFromExpressions(*$1);
                                                                                        if (temp == nullptr) {
                                                                                            yyerror("Lhs of short declaration must contains only identifiers");
                                                                                        } else {
                                                                                            $$ = new ShortVarDeclarationStatement(*temp, *$3);
                                                                                        } 
                                                                                    }
    ;

    Statement: Declaration                                                          { $$ = new DeclarationStatement(*$1); }
                | Block                                                             { $$ = $1; }
                | SimpleStmt                                                        { $$ = $1; }
                | ReturnStmt                                                        { $$ = $1; }
                | BREAK                                                             { $$ = new KeywordStatement(KeywordEnum::Break);        }
                | CONTINUE                                                          { $$ = new KeywordStatement(KeywordEnum::Continue);     }
                | FALLTHROUGH                                                       { $$ = new KeywordStatement(KeywordEnum::Fallthrough);  }
                | IfStmt                                                            { $$ = $1; }
                | ForStmt                                                           { $$ = $1; }
                | SwitchStmt                                                        { $$ = $1; }
    ;

    // Increment / decrement statement
    IncDecStmt: Expression INCREMENT                                                { $$ = new ExpressionStatement(new UnaryExpression(UnaryExpressionEnum::Increment, $1)); }
                | Expression DECREMENT                                              { $$ = new ExpressionStatement(new UnaryExpression(UnaryExpressionEnum::Decrement, $1)); }
    ;

    SimpleStmt: Expression                                                          { $$ = new ExpressionStatement($1); }
                | Assignment                                                        { $$ = $1; }
                | IncDecStmt                                                        { $$ = $1; }
                | ShortVarDecl                                                      { $$ = $1; }
    ;

    // Return statements
    ReturnStmt: RETURN ExpressionList                                               { $$ = new ReturnStatement(*$2); }
                | RETURN                                                            { $$ = new ReturnStatement(*(new ExpressionList())); }
    ;

/* -------------------------------- Blocks -------------------------------- */

    Block: '{' StatementMoreTwo '}'                                                 { $$ = new BlockStatement(*$2); }
                | '{' StatementMoreTwo SCs '}'                                      { $$ = new BlockStatement(*$2); }
                | '{' Statement '}'                                                 { $$ = new BlockStatement(*(new StatementList({$2}))); }
                | '{' Statement SCs '}'                                             { $$ = new BlockStatement(*(new StatementList({$2}))); }
                | '{' '}'                                                           { $$ = new BlockStatement(*(new StatementList())); }
    ;

    StatementMoreTwo: Statement SCs Statement                                       { $$ = new StatementList({$1, $3}); }
                | StatementMoreTwo SCs Statement                                    { $$ = $1; $$ -> push_back($3); }
    ;

    // If statements
    IfStmt: IF SimpleStmtWithExprWrapCompositeObj ';' ExprWithWrapCompositeObj Block ELSE IfStmt        { $$ = new IfStatement($2, $4, $5, $7); }
                | IF ExprWithWrapCompositeObj Block ELSE IfStmt                                         { $$ = new IfStatement(nullptr, $2, $3, $5); }
                | IF SimpleStmtWithExprWrapCompositeObj ';' ExprWithWrapCompositeObj Block ELSE Block   { $$ = new IfStatement($2, $4, $5, $7); }
                | IF ExprWithWrapCompositeObj Block ELSE Block                                          { $$ = new IfStatement(nullptr, $2, $3, $5); }
                | IF SimpleStmtWithExprWrapCompositeObj ';' ExprWithWrapCompositeObj Block              { $$ = new IfStatement($2, $4, $5, nullptr); }
                | IF ExprWithWrapCompositeObj Block                                                     { $$ = new IfStatement(nullptr, $2, $3, nullptr); }
    ;

    // For statement
    ForStmt: FOR ExprWithWrapCompositeObj Block                                                                                                         { $$ = new WhileStatement($2, $3); }
                | FOR SimpleStmtWithWrapCompositeObjOptional ';' ExprWithWrapCompositeObjOptional ';' SimpleStmtWithWrapCompositeObjOptional Block      { $$ = new ForStatement($2, $4, $6, $7); }
                | FOR ExprWithWrapCompositeObjList '=' RANGE ExprWithWrapCompositeObj Block                                                             { $$ = new ForRangeStatement(*$2, $5, $6, false); }
                | FOR ExprWithWrapCompositeObjList SHORT_DECL_OP RANGE ExprWithWrapCompositeObj Block                                                   { $$ = new ForRangeStatement(*$2, $5, $6, true); }
                | FOR RANGE ExprWithWrapCompositeObj Block                                                                                              { $$ = new ForRangeStatement(*(new ExpressionList()), $3, $4, false); }
                | FOR Block									                                                                                            { $$ = new WhileStatement(new BooleanExpression(true), $2); }
    ;

    // Switch statements
    SwitchStmt: SWITCH SimpleStmtWithExprWrapCompositeObj ';' ExprWithWrapCompositeObjOptional '{' ExprCaseClauseListOrEmpty ExprDefaultClauseOptional '}'      { $$ = new SwitchStatement($2, $4, *$6, *$7); }
                | SWITCH ExprWithWrapCompositeObjOptional '{' ExprCaseClauseListOrEmpty ExprDefaultClauseOptional '}'                                             { $$ = new SwitchStatement(nullptr, $2, *$4, *$5); }
    ;

    ExprDefaultClauseOptional: /* empty */                                          { $$ = new StatementList(); }
                | DEFAULT ':' StatementMoreTwo SCs                                  { $$ = $3; }
                | DEFAULT ':' Statement SCs                                         { $$ = new StatementList({$3}); }
    ;

    ExprCaseClause: CASE Expression ':' StatementMoreTwo  SCs                       { $$ = new SwitchCaseClause($2, *$4); }
                | CASE Expression ':' Statement SCs                                 { $$ = new SwitchCaseClause($2, *(new StatementList({$4}))); }
    ;

    ExprCaseClauseList: ExprCaseClause                                              { $$ = new SwitchCaseList({$1}); }
                | ExprCaseClauseList ExprCaseClause                                 { $$ = $1; $$ -> push_back($2); }
    ;

    ExprCaseClauseListOrEmpty: /* empty */                                          { $$ = new SwitchCaseList(); }
                | ExprCaseClauseList                                                { $$ = $1; }
    ;

    SCs: ';'                                                                        { }
                | SCs ';'                                                           { }
%%

