#include <iostream>
#include <vector>
#include <llvm/IR/Value.h>

#include <json/json.h>

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

using std::string;
using namespace std;

class Node {
public:
	const char m_DELIM = ':';
	const char* m_PREFIX = "--";
	virtual ~Node() {}
	virtual llvm::Value* codeGen(CodeGenContext& context) { return NULL; }
	virtual string getTypeName() const = 0;
	virtual void print(string prefix) const{}
	virtual Json::Value jsonGen() const { return Json::Value(); }
};

class NExpression : public Node {
public:
	string getTypeName() const override {
		return "NExpression";
	}

    virtual void print(string prefix) const override{
        cout << prefix << getTypeName() << endl;
    }

    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();
        return root;
    }
};

class NStatement : public Node {
public:
	string getTypeName() const override {
		return "NStatement";
	}
    virtual void print(string prefix) const override{
        cout << prefix << getTypeName() << endl;
    }

    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();
        return root;
    }
};

class NInteger : public NExpression {
public:
	long long value;
	NInteger(long long value) : value(value) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	string getTypeName() const override {
        return "NInteger";
    }

    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName() + this->m_DELIM + std::to_string(value);
        return root;
    }
};

class NDouble : public NExpression {
public:
	double value;
	NDouble(double value) : value(value) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NIdentifier : public NExpression {
public:
	std::string name;
	NIdentifier(const std::string& name) : name(name) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	string getTypeName() const override {
		return "NIdentifier";
	}

    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName() + this->m_DELIM + name;
        return root;
    }
};

class NMethodCall : public NExpression {
public:
	const NIdentifier& id;
	ExpressionList arguments;
	NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
		id(id), arguments(arguments) { }
	NMethodCall(const NIdentifier& id) : id(id) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	string getTypeName() const override {
		return "NMethodCall";
	}

    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();
        root["children"].append(this->id.jsonGen());
        for(auto it=arguments.begin(); it!=arguments.end(); it++){
            root["children"].append((*it)->jsonGen());
        }
        return root;
    }
};

class NBinaryOperator : public NExpression {
public:
	int op;
	NExpression& lhs;
	NExpression& rhs;
	NBinaryOperator(NExpression& lhs, int op, NExpression& rhs) :
		lhs(lhs), rhs(rhs), op(op) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	string getTypeName() const override {
		return "NBinaryOperator";
	}

    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName() + this->m_DELIM + std::to_string(op);

        root["children"].append(lhs.jsonGen());
        root["children"].append(rhs.jsonGen());

        return root;
    }
};

class NAssignment : public NExpression {
public:
	NIdentifier& lhs;
	NExpression& rhs;
	NAssignment(NIdentifier& lhs, NExpression& rhs) :
		lhs(lhs), rhs(rhs) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	string getTypeName() const override {
		return "NAssignment";
	}
    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();
        root["children"].append(lhs.jsonGen());
        root["children"].append(rhs.jsonGen());
        return root;
    }
};

class NBlock : public NExpression {
public:
	StatementList statements;
	NBlock() { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	string getTypeName() const override {
		return "NBlock";
	}


    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();
        for(auto it=statements.begin(); it!=statements.end(); it++){
            root["children"].append((*it)->jsonGen());
        }
        return root;
    }
};

class NExpressionStatement : public NStatement {
public:
	NExpression& expression;
	NExpressionStatement(NExpression& expression) :
		expression(expression) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	string getTypeName() const override {
		return "NExpressionStatement";
	}



    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();
        root["children"].append(expression.jsonGen());
        return root;
    }
};

class NReturnStatement : public NStatement {
public:
	NExpression& expression;
	NReturnStatement(NExpression& expression) :
		expression(expression) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	string getTypeName() const override {
        return "NReturnStatement";
    }

    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();
        root["children"].append(expression.jsonGen());
        return root;
    }


};

class NVariableDeclaration : public NStatement {
public:
	const NIdentifier& type;
	NIdentifier& id;
	NExpression *assignmentExpr;
	NVariableDeclaration(const NIdentifier& type, NIdentifier& id) :
		type(type), id(id) { assignmentExpr = NULL; }
	NVariableDeclaration(const NIdentifier& type, NIdentifier& id, NExpression *assignmentExpr) :
		type(type), id(id), assignmentExpr(assignmentExpr) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	string getTypeName() const override {
		return "NVariableDeclaration";
	}


    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();
        root["children"].append(type.jsonGen());
        root["children"].append(id.jsonGen());
        if( assignmentExpr != nullptr ){
            root["children"].append(assignmentExpr->jsonGen());
        }
        return root;
    }
};

class NExternDeclaration : public NStatement {
public:
    const NIdentifier& type;
    const NIdentifier& id;
    VariableList arguments;
    NExternDeclaration(const NIdentifier& type, const NIdentifier& id,
            const VariableList& arguments) :
        type(type), id(id), arguments(arguments) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
    string getTypeName() const override {
		return "NFunctionDeclaration";
	}
		Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();
        root["children"].append(type.jsonGen());
        root["children"].append(id.jsonGen());

        for(auto it=arguments.begin(); it!=arguments.end(); it++){
            root["children"].append((*it)->jsonGen());
        }

        return root;
    }
};

class NFunctionDeclaration : public NStatement {
public:
	const NIdentifier& type;
	const NIdentifier& id;
	VariableList arguments;
	NBlock& block;
	NFunctionDeclaration(const NIdentifier& type, const NIdentifier& id,
			const VariableList& arguments, NBlock& block) :
		type(type), id(id), arguments(arguments), block(block) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	string getTypeName() const override {
		return "NFunctionDeclaration";
	}
	Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();
        root["children"].append(type.jsonGen());
        root["children"].append(id.jsonGen());

        for(auto it=arguments.begin(); it!=arguments.end(); it++){
            root["children"].append((*it)->jsonGen());
        }

        root["children"].append(block.jsonGen());


        return root;
    }
};

class NIfStatement: public NStatement{
public:

    NExpression*  Cond;
    NBlock* Then;          // should not be null
    NBlock* Else;         // can be null


    NIfStatement(){}

    NIfStatement(NExpression* cond, NBlock* blk, NBlock* blk2 = nullptr)
            : Cond(cond), Then(blk), Else(blk2){

    }

    virtual llvm::Value *codeGen(CodeGenContext &context) ;
    string getTypeName() const override {
        return "NIfStatement";
    }
    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();
        root["children"].append(Cond->jsonGen());
        root["children"].append(Then->jsonGen());
        if( Then ){
            root["children"].append(Else->jsonGen());
        }
        return root;
    }


};

class NForStatement: public NStatement{
public:
    NExpression* initial, *condition, *increment;
   	NBlock * block;

    NForStatement(){}

    NForStatement(NBlock* b, NExpression* init = nullptr, NExpression* cond = nullptr, NExpression* incre = nullptr)
            : block(b), initial(init), condition(cond), increment(incre){

    }

    virtual llvm::Value *codeGen(CodeGenContext &context) ;
    string getTypeName() const override{
        return "NForStatement";
    }
    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();

        if( initial )
            root["children"].append(initial->jsonGen());
        if( condition )
            root["children"].append(condition->jsonGen());
        if( increment )
            root["children"].append(increment->jsonGen());

        return root;
    }

};
class NLiteral: public NExpression{
public:
    string value;

    NLiteral(){}

    NLiteral(const string &str) {
        value = str.substr(1, str.length()-2);
    }
    virtual llvm::Value *codeGen(CodeGenContext &context) ;
    string getTypeName() const override{
        return "NLiteral";
    }
    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName() + this->m_DELIM + value;

        return root;
    }

};

class NStructMember: public NExpression{
public:
	NIdentifier* id;
	NIdentifier* member;

    NStructMember(){}

    NStructMember(NIdentifier* structName, NIdentifier* member)
            : id(structName),member(member) {
    }

    virtual llvm::Value *codeGen(CodeGenContext &context)  ;
    string getTypeName() const override{
        return "NStructMember";
    }
    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();

        root["children"].append(id->jsonGen());
        root["children"].append(member->jsonGen());

        return root;
    }

};
class NStructAssignment: public NExpression{
public:
    NStructMember* structMember;
    NExpression* expression;

    NStructAssignment(){}

    NStructAssignment(NStructMember* member,NExpression* exp)
            : structMember(member), expression(exp){

    }


    virtual llvm::Value *codeGen(CodeGenContext &context) ;
    string getTypeName() const override{
        return "NStructAssignment";
    }
    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName();

        root["children"].append(structMember->jsonGen());
        root["children"].append(expression->jsonGen());

        return root;
    }

};

class NStructDeclaration: public NStatement{
public:
    NIdentifier* name;
    VariableList* members;

    NStructDeclaration(){}

    NStructDeclaration(NIdentifier *id, VariableList *arguments)
            : name(id), members(arguments){

    }


    virtual llvm::Value* codeGen(CodeGenContext& context)  ;
    string getTypeName() const override {
        return "NStructDeclaration";
    }
    Json::Value jsonGen() const override {
        Json::Value root;
        root["name"] = getTypeName() + this->m_DELIM + this->name->name;

        for(auto it=members->begin(); it!=members->end(); it++){
            root["children"].append((*it)->jsonGen());
        }

        return root;
    }
};

