#pragma once

#include "../ast.h"
#include "../visitor.h"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

class CloneVisitor final : public Visitor {
private:
    std::unordered_map<std::int64_t, NodeASTPtr> copies;

    template<typename T>
    std::unique_ptr<T> takeChild(const std::unique_ptr<T>& source) {
        return source ? take<T>(source->nodeId) : nullptr;
    }

    template<typename List>
    List takeList(const List& source) {
        List result;
        for (const auto& child : source) {
            result.push_back(takeChild(child));
        }
        return result;
    }

    template<typename T>
    void store(const T& source, std::unique_ptr<T> copy) {
        if constexpr (std::is_base_of_v<ExpressionAST, T>) {
            copy->typeExpression = source.typeExpression;
        }
        if constexpr (std::is_base_of_v<TypeAST, T>) {
            copy->isPointer = source.isPointer;
            copy->isVariadic = source.isVariadic;
        }
        if (!copies.try_emplace(source.nodeId, std::move(copy)).second) {
            throw std::logic_error("Duplicate AST copy for node " + std::to_string(source.nodeId));
        }
    }

    void onFinishVisit(NodeAST& node) override;
    void onFinishVisit(PackageAST& node) override;
    void onFinishVisit(VariableDeclaration& node) override;
    void onFinishVisit(TypeDeclaration& node) override;
    void onFinishVisit(FunctionDeclaration& node) override;
    void onFinishVisit(MethodDeclaration& node) override;
    void onFinishVisit(IdentifierAsExpression& node) override;
    void onFinishVisit(IntegerExpression& node) override;
    void onFinishVisit(BooleanExpression& node) override;
    void onFinishVisit(FloatExpression& node) override;
    void onFinishVisit(StringExpression& node) override;
    void onFinishVisit(NilExpression& node) override;
    void onFinishVisit(FunctionLitExpression& node) override;
    void onFinishVisit(UnaryExpression& node) override;
    void onFinishVisit(BinaryExpression& node) override;
    void onFinishVisit(CallableExpression& node) override;
    void onFinishVisit(AccessExpression& node) override;
    void onFinishVisit(ElementCompositeLiteral& node) override;
    void onFinishVisit(CompositeLiteral& node) override;
    void onFinishVisit(BlockStatement& node) override;
    void onFinishVisit(KeywordStatement& node) override;
    void onFinishVisit(ExpressionStatement& node) override;
    void onFinishVisit(AssignmentStatement& node) override;
    void onFinishVisit(ForStatement& node) override;
    void onFinishVisit(WhileStatement& node) override;
    void onFinishVisit(ForRangeStatement& node) override;
    void onFinishVisit(ReturnStatement& node) override;
    void onFinishVisit(IfStatement& node) override;
    void onFinishVisit(SwitchCaseClause& node) override;
    void onFinishVisit(SwitchStatement& node) override;
    void onFinishVisit(DeclarationStatement& node) override;
    void onFinishVisit(IdentifiersWithType& node) override;
    void onFinishVisit(FunctionSignature& node) override;
    void onFinishVisit(ArraySignature& node) override;
    void onFinishVisit(IdentifierAsType& node) override;
    void onFinishVisit(StructSignature& node) override;
    void onFinishVisit(InterfaceType& node) override;
    void onFinishVisit(ShortVarDeclarationStatement& node) override;

public:
    template<typename T = NodeAST>
    [[nodiscard]] std::unique_ptr<T> take(std::int64_t nodeId) {
        static_assert(std::is_base_of_v<NodeAST, T>);
        auto found = copies.find(nodeId);
        if (found == copies.end()) {
            throw std::logic_error("Missing AST copy for node " + std::to_string(nodeId));
        }
        if (!dynamic_cast<T*>(found->second.get())) {
            throw std::logic_error("Unexpected AST copy type for node " + std::to_string(nodeId));
        }
        auto result = std::unique_ptr<T>(static_cast<T*>(found->second.release()));
        copies.erase(found);
        return result;
    }
};

template<typename T>
[[nodiscard]] std::unique_ptr<T> cloneNode(T& node) {
    static_assert(std::is_base_of_v<NodeAST, T>);
    CloneVisitor visitor;
    node.acceptVisitor(&visitor);
    return visitor.take<T>(node.nodeId);
}
