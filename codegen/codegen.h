#include "visitor.h"
#include "java_entity.h"
#include "constant.h"

#include <unordered_map>

class Generator : public Visitor {
public:
    void generate(std::unordered_map<std::string, JavaClass*> & classPool);
    //void generate(Constant constant);
    //void generate(JavaFunction* method);
};
