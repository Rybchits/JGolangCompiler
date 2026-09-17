#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <utility>

template <typename T>
class Context {
private:
    std::vector<std::unordered_map<std::string, T>> scopes;

public:
    Context() {
        pushScope();
    }

    void pushScope() {
        scopes.emplace_back();
    }

    void popScope() {
        scopes.pop_back();
    }

    const std::unordered_map<std::string, T>& getLastScope() const {
        return scopes.back();
    }

    bool add(std::string id, T element) {
        if (id == "_") {
            return false;
        }
        scopes.back().insert_or_assign(std::move(id), std::move(element));
        return true;
    }

    T* find(const std::string& name) {
        for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {
            auto entry = scope->find(name);
            if (entry != scope->end()) return &entry->second;
        }
        return nullptr;
    }

    const T* find(const std::string& name) const {
        for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {
            auto entry = scope->find(name);
            if (entry != scope->end()) return &entry->second;
        }
        return nullptr;
    }

    T* findAtLastScope(const std::string& name) {
        auto entry = scopes.back().find(name);
        return entry != scopes.back().end() ? &entry->second : nullptr;
    }

    const T* findAtLastScope(const std::string& name) const {
        auto entry = scopes.back().find(name);
        return entry != scopes.back().end() ? &entry->second : nullptr;
    }
};
