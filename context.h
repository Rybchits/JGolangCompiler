#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>

template <typename T>
class Context {
    
private:
    std::vector<std::unordered_map<std::string, T>> scopes;

public:
    Context() { 
        pushScope(); 
    };
    
    void pushScope() { 
        scopes.push_back(std::unordered_map<std::string, T>()); 
    };

    void popScope() { 
        scopes.pop_back(); 
    }

    std::unordered_map<std::string, T> getLastScope() {
        return scopes.back();
    }

    bool add(std::string id, T element) { 
        if (id == "_") {
            return false;
        }

        return scopes.back()[id] = element;
    }

    T find(std::string name) {
        
        for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {
            if ((*scope).count(name)) return (*scope)[name];
        }
        return nullptr;
    };

    T findAtLastScope(std::string name) {
        return scopes.back().count(name)? scopes.back()[name] : nullptr;
    }
};