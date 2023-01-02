#include <vector>
#include <unordered_map>
#include <string>

template <typename T>
class Context {
    
private:
    std::vector<std::unordered_map<std::string, T>> scopes;

public:
    void pushScope() { scopes.push_back(std::unordered_map<std::string, T>()); };
    void popScope() { scopes.pop_back(); }

    bool add(std::string id, T constant) { return scopes.back()[id] = constant; }

    std::unordered_map<std::string, T> getLastScope() { return scopes.back(); }

    T find(std::string name) {
        for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {
            
            if ((*scope).count(name)) {
                return (*scope)[name];
            }
        }
        return nullptr;
    };

    Context() { pushScope(); };
};