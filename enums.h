#pragma once

enum UnaryExpressionEnum {
    UnaryNot,
    UnaryPlus,
    UnaryMinus,
    Increment,
    Decrement,
    Variadic,
};

enum BinaryExpressionEnum {
    Addition,            // +
    Subtraction,         // -
    Multiplication,      // *
    Division,            // /
    Mod,                 // %
    And,                 // &&
    Or,                  // ||
    Equal,               // ==
    Greater,             // >
    Less,                // <
    NotEqual,            // !=
    LessOrEqual,         // <=
    GreatOrEqual,        // >=
};

enum AccessExpressionEnum {
    Indexing,
    FieldSelect,
};


enum KeywordEnum {
    Break,
    Continue,
    Fallthrough
};

enum AssignmentEnum {
    SimpleAssign,       // =
    MinusAssign,        // -=
    PlusAssign,         // +=
    ModAssign,          // %=
    MulAssign,          // *=
    DivAssign,          // /=
    ShortDeclaration,   // :=
};

enum TraversalMethod {
    Upward,
    Downward
};