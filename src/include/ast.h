
// This file is a part of Simple-XX/SimpleCompiler
// (https://github.com/Simple-XX/SimpleCompiler).
//
// ast.h for Simple-XX/SimpleCompiler.

#ifndef _AST_H_
#define _AST_H_

#include <memory>
#include <string>
#include <vector>
#include "type.h"

using namespace std;

class MetaAST;

typedef std::unique_ptr<MetaAST> ASTPtr;
typedef std::vector<ASTPtr> ASTPtrList;

class MetaAST {
    public:
        virtual ~MetaAST() = default;
        virtual string to_string(void) = 0;
};

// Compile Unit 编译单元
class CompUnitAST : public MetaAST {
    private:
        ASTPtrList units; 
    public:
        CompUnitAST(ASTPtrList u) : units(move(u)) {}
        // construction
        ~CompUnitAST() override {
            for (auto &unit : units) {
                if (unit) unit.reset();
            }
        }
        // destruction
        string to_string(void) override {
            string output;
            for (auto &unit : units)
            {
                output = output + "\n" + unit->to_string();
            }
            return "CompUnit: [" + output + "]\n";
        }
};

// Statement 语句
class StmtAST : public MetaAST {
    private:
        ASTPtr stmt; 
    public:
        StmtAST(ASTPtr s) : stmt(move(s)) {}
        // construction
        ~StmtAST() override {
            if (stmt) stmt.reset();
        }
        // destruction
        string to_string(void) override {
            return "Statement: {" + stmt->to_string() + "}\n";
        }
};

// FunctionDefinition 函数定义
class FuncDefAST : public MetaAST {
    private:
        Type type; 
        // function return type 函数返回类型
        string name; 
        // function name 函数名
        ASTPtrList params; 
        // function params 函数参数列表
        ASTPtr body; 
        // function body 函数体
    public:
        FuncDefAST(Type t, const string &n, ASTPtrList p, ASTPtr b) : type(t), name(n), params(move(p)), body(move(b)) {}
        // construction
        ~FuncDefAST() override {
            for (auto &param : params) {
                if (param) param.reset();
            }
            if (body) body.reset();
        }
        // destruction
        string to_string(void) override {
            string output = "FunctionDef(" + type_to_string(type) + "): " + name + ' ';
            for (auto &param : params)
            {
                if (param) output = output + param->to_string();
            }
            if (body) output = output + body->to_string();
            return output;
        }
};

// FunctionCall 函数调用
class FuncCallAST : public MetaAST {
    private:
        string name;
        ASTPtrList args;
    public:
        FuncCallAST(const string &n, ASTPtrList a = ASTPtrList{}) : name(n), args(move(a)) {}
        // construction
        ~FuncCallAST() override {
            for (auto &arg: args) {
                if (arg) arg.reset();
            }
        }
        // destruction
        string to_string(void) override {
            return "FuncCallAST";
        }
};

// VarDeclaration 变量声明
class VarDeclAST : public MetaAST {
    private:
        ASTPtrList vars;
        // many vars, for example: int a,b,c,d;
        bool isConst;
        // const or not
    public:
        VarDeclAST(bool i, ASTPtrList v) : vars(move(v)), isConst(i) {}
        // construction
        ~VarDeclAST() override {
            for (auto &var: vars) {
                if (var) var.reset();
            }
        }
        // destruction
        string to_string(void) override {
            string output;
            for (auto &unit : vars)
            {
                output = output + "\n" + unit->to_string();
            }
            if (isConst) {
                return "VarDeclAST (CONST): {" + output + "}";
            }
            return "VarDeclAST: {" + output + "}";
        }
};

// VarDefinition 变量定义
class VarDefAST : public MetaAST {
    private:
        ASTPtr var;
        // Ident
        ASTPtr initVal; 
        // initial value
        bool isConst;
        // const or not
    public:
        VarDefAST(bool i, ASTPtr v, ASTPtr init = nullptr) : var(move(v)), initVal(move(init)), isConst(i) {}
        // construction
        ~VarDefAST() override {
            if (var) var.reset();
            if (initVal) initVal.reset();
        }
        // destruction
        string to_string(void) override {
            if (isConst) {
                return "VarDefAST (CONST): {" + var->to_string() + "}";
            }
            return "VarDefAST: { " + var->to_string() + " }";
        }
};

// Ident 变量
class IdAST : public MetaAST {
    private:
        string name;
        VarType type;
        ASTPtrList dim;
        bool isConst;

    public:
        IdAST(const string &n, VarType t, bool i, ASTPtrList d = ASTPtrList{}) : name(n), type(t), dim(move(d)), isConst(i) {}
        // construction
        ~IdAST() override {
            for (auto &d : dim) {
                d.reset();
            }
        }
        // destruction
        string to_string(void) override {
            if (isConst) {
                return "IdAST (CONST) (" + vartype_to_string(type) + "): " + name;
            }
            return "IdAST(" + vartype_to_string(type) + "): " + name;
        }
};

// InitialValue 初始值
class InitValAST : public MetaAST {
    private:
        VarType type;
        ASTPtrList values;
    public:
        InitValAST(VarType t ,ASTPtrList v) : type(t), values(move(v)) {}
        // construction
        ~InitValAST() override {
            for (auto &value: values) {
                if (value) value.reset();
            }
        }
        // destruction
        string to_string(void) override {
            return "InitValAST(" + vartype_to_string(type) + ")";
        }
};

// Block 块作用域
class BlockAST : public MetaAST {
    private:
        ASTPtrList stmts; 
        // block statements 一串语句
    public:
        BlockAST(ASTPtrList s) : stmts(move(s)) {}
        // construction
        ~BlockAST() override {
            for (auto &s : stmts) {
                if (s) s.reset();
            }
        }
        // destruction
        string to_string(void) override {
            string output;
            for (auto &unit : stmts)
            {
                output = output + "\n" + unit->to_string();
            }
            return "BlockAST: {" + output + "}";
        }
};

// BinaryExpression 二元表达式 (A op B)
class BinaryAST : public MetaAST {
    private:
        Operator op;
        // operator
        ASTPtr left;
        // left expression
        ASTPtr right;
        // right expression
    public:
        BinaryAST(Operator o, ASTPtr l, ASTPtr r) : op(o), left(move(l)), right(move(r)) {}
        // construction
        ~BinaryAST() override {
            if (left) left.reset();
            if (right) right.reset();
        }
        // destruction
        string to_string(void) override {
            return  '(' + (left->to_string()) + ' ' + op_to_string(op) + ' ' + (right->to_string()) + ')';
        }
};

// UnaryExpression 一元表达式 (op A)
class UnaryAST : public MetaAST {
    private:
        Operator op;
        // operator
        ASTPtr exp;

        // expression
    public:
        UnaryAST(Operator o, ASTPtr e) : op(o), exp(move(e)) {}
        // construction
        ~UnaryAST() override {
            if (exp) exp.reset();
        }
        // destruction
        string to_string(void) override {
            return '(' +  op_to_string(op) + ' ' + exp->to_string() + ')';
        }
};

// Number 数字（int）
class NumAST : public MetaAST {
    private:
        int val;
        // number value
    public:
        NumAST(int v) : val(v) {}
        // construction
        ~NumAST() override {}
        // destruction
        string to_string(void) override {
            return std::to_string(val);
        }
};

// If 条件表达式
class IfAST : public MetaAST {
    private:
        ASTPtr conditionExp;
        // condition expression, decide which branch (then or else) to eval
        ASTPtr thenAST;
        // then branch
        ASTPtr elseAST;
        // else branch 
    public:
        IfAST(ASTPtr c, ASTPtr t, ASTPtr e = nullptr) : conditionExp(move(c)), thenAST(move(t)), elseAST(move(e)) {}
        // construction
        ~IfAST() override {
            if (conditionExp) conditionExp.reset();
            if (thenAST) thenAST.reset();
            if (elseAST) elseAST.reset();
        }
        // destruction
        string to_string(void) override {
            if (elseAST)
                return "IfAST: { if (" + conditionExp->to_string() + " ) then ( " + thenAST->to_string() + ") else (" + elseAST->to_string() + " ) }";
            else return "IfAST: { if (" + conditionExp->to_string() + " ) then ( " + thenAST->to_string() + " ) }";
        }
};

// While 循环
class WhileAST : public MetaAST {
    private:
        ASTPtr conditionExp;
        // condition expression, decide whether to continue or not
        ASTPtr body;
        // loop body
    public:
        WhileAST(ASTPtr c, ASTPtr b) : conditionExp(move(c)), body(move(b)) {}
        // construction
        ~WhileAST() override {
            if (conditionExp) conditionExp.reset();
            if (body) body.reset();
        }
        // destruction
        string to_string(void) override {
            return "WhileAST: { while (" + conditionExp->to_string() + " ) do ( " + body->to_string() + " ) }";
        }
};

// Control 控制语句 (break continue return)
class ControlAST : public MetaAST {
    private:
        Control type;
        // control type: break_c continue_c return_c
        ASTPtr returnStmt;
        // to which statement (destination)
    public:
        ControlAST(Control t, ASTPtr r = nullptr) : type(t), returnStmt(move(r)) {}
        // construction
        ~ControlAST() override {
            if (returnStmt) returnStmt.reset();
        }
        // destruction
        string to_string(void) override {
            if (type == Control::break_c) {
                return "ControlAST: BREAK";
            }
            if (type == Control::continue_c) {
                return "ControlAST: CONTINUE";
            }
            if (type == Control::return_c) {
                if (returnStmt) {
                    return "ControlAST: RETURN (" + returnStmt->to_string() + ")";
                }
                return "ControlAST: RETURN ";
            }
            return "ERROR";
        }
};

// Assignment 赋值语句 (break continue return)
class AssignAST : public MetaAST {
    private:
        ASTPtr left;
        // LVal
        ASTPtr right;
        // Expression
    public:
        AssignAST(ASTPtr l, ASTPtr r) : left(move(l)), right(move(r)) {}
        // construction
        ~AssignAST() override {
            if (left) left.reset();
            if (right) right.reset();
        }
        // destruction
        string to_string(void) override {
            return " AssignAST: { " + left->to_string() + " = " + right->to_string() + " }";
        }
};

// LeftValue 左值
class LValAST : public MetaAST {
    private:
        string name;
        VarType type;
        ASTPtrList position;
    public:
        LValAST(const string &n, VarType t ,ASTPtrList p = ASTPtrList{}) : name(n), type(t), position(move(p)) {}
        // construction
        ~LValAST() override {
            for (auto &pos: position) {
                if (pos) pos.reset();
            }
        }
        // destruction
        string to_string(void) override {
            return "LValAST:(" + vartype_to_string(type) + "):  { " + name + " }";
        }
};

// 空指令 
class EmptyAST : public MetaAST {
    public:
        EmptyAST() {}
        // construction
        ~EmptyAST() override {}
        // destruction
        string to_string(void) override {
            return "EmptyAST";
        }
};

#endif /* _AST_H_ */