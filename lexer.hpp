#pragma once

#include "parser.tab.h"

#include <string>
#include <cstdio>
#include <algorithm>
#include <iostream>


// Printing functions
void Print(std::string & message);
void PrintComment(const std::string & comment);
void PrintStringLiteral(const std::string & literal);
void PrintKeyword(const std::string & keyword);
void PrintLogicalOperator(const std::string & o);
void PrintAssignmentOperator(const std::string & o);
void PrintArithmeticOperator(const std::string & o);
void PrintSeparatingOperator(const std::string & o);
void PrintIntegerLiteral(const std::string & type, long long integer);
void PrintIdentifier(const std::string & identifier);
void PrintRune(const int32_t rune);
void PrintFloatingPointLiteral(const double floatingPoint);
void PrintErrorAndExit(const std::string & e);

char* DuplicateString(const char* source);


void Print(const std::string & message) {
    std::cout << message << std::endl;
}

void PrintComment(const std::string & comment) {
    Print("Comment:");
    Print("\"" + comment + "\"");
}

void PrintStringLiteral(const std::string & literal) {
    Print("String literal:");
    Print("\"" + literal + "\"");
}

void PrintKeyword(const std::string & keyword) {
    Print("Keyword: " + keyword);
}

void PrintLogicalOperator(const std::string & o) {
    Print("Logic operator: " + o);
}

void PrintAssignmentOperator(const std::string & o) {
    Print("Assignment operator: " + o);
}

void PrintArithmeticOperator(const std::string & o) {
    Print("Ariphmetic operator: " + o);
}
    
void PrintSeparatingOperator(const std::string & o) {
    Print("Separating operator: " + o);
}

void PrintIntegerLiteral(const std::string & type, long long integer) {
    Print("Integer " + type + " : " + std::to_string(integer));
}

void PrintIdentifier(const std::string & identifier) {
    Print("Identifier: " + identifier);
}

void PrintRune(const int32_t rune) {
    Print("Rune: " + std::to_string(rune));
}

void PrintFloatingPointLiteral(const double floatingPoint) {
    std::cout << "Floating point literal: " << floatingPoint << std::endl;
}


void PrintErrorAndExit(const std::string & e) {
    Print("Error: " + e);
    exit(1);
}

char* DuplicateString(const char* source) {
    const auto length = std::strlen(source) + 1;
    char* destination = new char[length];
    std::fill(destination, destination + length, 0);
    std::copy(source, source + length, destination);
    return destination;
}
