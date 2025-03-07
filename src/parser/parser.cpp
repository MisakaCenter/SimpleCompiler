
// This file is a part of Simple-XX/SimpleCompiler
// (https://github.com/Simple-XX/SimpleCompiler).
//
// parser.cpp for Simple-XX/SimpleCompiler.

#include "parser.h"

Parser::Parser(Lexer &lex) : lexer(lex) {
    return;
}

Parser::~Parser() {
    return;
}

// 获取下一个 token
void Parser::next(void) {
    token = lexer.lexing();
    return;
}

// 匹配指定 Token
bool Parser::match_token(Tag tag) {
    if (token->tag == tag) {
        return true;
    }
    else {
        return false;
    }
}

// 进行解析，返回解析结果(AST)
ASTPtr Parser::parsing(void) {
    this->next(); // 读入第一个token

    ASTPtr prog = program();
    return prog;
}

// 程序由代码片段组成，代码片段由声明与定义组成
ASTPtr Parser::program(void) {
    ASTPtrList nodes;
    while (is_done() == false) {
        if (match_token(Tag::KW_CONST)) {
            ASTPtr variable_decl = var_decl();
            if (!variable_decl) {
                error->display_err();
                exit(1);
            }
            nodes.push_back(move(variable_decl));
        } else if (match_token(Tag::KW_VOID)) {
            ASTPtr func = function_def();
            if (!func) {
                error->display_err();
                exit(2);
            }
            nodes.push_back(move(func));
        } else if (match_token(Tag::KW_INT)){ // var or func
            next(); // int
            if (!match_token(Tag::ID)) {
                error->display_err();
                exit(3);
            }
            Id* token_casted = (Id*)token;
            string name = token_casted->name;
            next(); // id

            //function def
            if (match_token(Tag::LPAREN)) {
                next(); // (
                ASTPtrList args;
                if (!match_token(Tag::RPAREN)) {
                    while (true) {
                        // TODO: only support int
                        if ((!match_token(Tag::KW_INT))) {
                            error->display_err();
                            exit(996);
                        }
                        next(); // type
                        if (!match_token(Tag::ID)) {
                            error->display_err();
                            exit(998);
                        }
                        // arg name
                        Id* token_casted = (Id*)token;
                        string arg_name = token_casted->name;
                        next(); // id
                        if (match_token(Tag::LBRACKET)) // [
                        {
                            ASTPtrList dim;
                            dim.push_back(make_unique<NumAST>(0));
                            next(); // [
                            if (!match_token(Tag::RBRACKET))
                            {
                                exit(997);
                            }
                            next(); // ]
                            while (match_token(Tag::LBRACKET))
                            {
                                next(); // [
                                ASTPtr _dim = binary_add();
                                if (!_dim)
                                {
                                    error->display_err();
                                    exit(995);
                                }
                                dim.push_back(move(_dim));
                                if (!match_token(Tag::RBRACKET))
                                {
                                    error->display_err();
                                    exit(994);
                                }
                                next(); // ]
                            }
                            args.push_back(make_unique<IdAST>(arg_name, VarType::array_t, false, move(dim)));
                        } else {
                            args.push_back(make_unique<IdAST>(arg_name, VarType::var_t, false));
                        }
                        if (!match_token(Tag::COMMA))
                            break;
                        next(); // ,
                    }
                    if (!match_token(Tag::RPAREN)) {
                        error->display_err();
                        exit(993);
                    }
                }
                next(); // )
                ASTPtr body = block();
                ASTPtr func = make_unique<FuncDefAST>(Type::int_t, name, move(args), move(body));
                nodes.push_back(move(func));
            } else { // var def
                ASTPtrList varDefs;
                // first , because id is consumed
                ASTPtrList dims;
                while (match_token(Tag::LBRACKET)) {
                    next(); // [
                    ASTPtr exp = binary_add();
                    if (!exp) {
                        exit(453);
                    }
                    dims.push_back(move(exp));
                    if (!match_token(Tag::RBRACKET)) {
                        exit(454);
                    }
                    next(); // ]
                }
                ASTPtr var;
                ASTPtr varDef;
                if (dims.empty())
                    var = make_unique<IdAST>(name, VarType::var_t, false);
                else 
                    var = make_unique<IdAST>(name, VarType::array_t, false, move(dims));
                if (match_token(Tag::ASSIGN)) {
                    next(); // =
                    ASTPtr init = init_val();
                    if (!init) {
                        exit(456);
                    }
                    varDef = make_unique<VarDefAST>(false, move(var), move(init));
                } else {
                    varDef = make_unique<VarDefAST>(false, move(var));
                }
                varDefs.push_back(move(varDef));
                
                while (match_token(Tag::COMMA)) {
                    next(); // ,
                    varDef = var_def(false);
                    if (!varDef) {
                        exit(133);
                    }
                    varDefs.push_back(move(varDef));
                }
                if (!match_token(Tag::SEMICON)) {
                    exit(134);
                }
                ASTPtr decl = make_unique<VarDeclAST>(false, move(varDefs));
                nodes.push_back(move(decl));
                next(); // ;
            }
        } else {
            error->display_err();
            exit(233);
        }
    }
    return make_unique<CompUnitAST>(move(nodes));
}

// 二元表达式
ASTPtr Parser::binary(const function<ASTPtr()> &parser, initializer_list<Operator> ops) {
    auto lhs = parser();
    if (!lhs) {
        cout << "error 100" << endl;
        exit(100);
    }
    while (find(ops.begin(), ops.end(), tag_to_op(token->tag)) != ops.end()) {
        Operator op = tag_to_op(token->tag);
        next();
        auto rhs = parser();
        if (!rhs) {
            cout << "error 101" << endl;
            exit(101);
        }
        lhs = make_unique<BinaryAST>(op, move(lhs), move(rhs));
    }
    return lhs;
}

ASTPtr Parser::binary_relation(void){
    return binary([this]
                  { return binary_add(); }, {Operator::gt_op, Operator::ge_op,  Operator::le_op, Operator::le_op});
}

ASTPtr Parser::binary_eq(void){
    return binary([this]
                  { return binary_relation(); }, {Operator::equ_op, Operator::nequ_op});
}

ASTPtr Parser::binary_add(void){
    return binary([this]
                  { return binary_mul(); }, {Operator::add_op, Operator::sub_op});
}

ASTPtr Parser::binary_mul(void){
    return binary([this]
                  { return unary(); }, {Operator::mul_op, Operator::div_op, Operator::mod_op});
}

ASTPtr Parser::binary_and(void){
    return binary([this]
                  { return binary_eq(); }, {Operator::and_op});
}

ASTPtr Parser::binary_or(void){
    return binary([this]
                  { return binary_and(); }, {Operator::or_op});
}

// 一元表达式
ASTPtr Parser::unary(void)
{
    if (match_token(Tag::LPAREN)) {
        // (  EXP  )
        next(); // 消耗左括号
        ASTPtr exp = binary_add();
        if (token->tag != Tag::RPAREN){
            cout << "error 102" << endl;
            exit(102);
        }
        next(); // 消耗右括号
        return exp;
    } else if (match_token(Tag::NUM)){
        // NUM
        Num* token_casted = (Num*)token;
        ASTPtr num = make_unique<NumAST>(token_casted->val);
        next();
        return num;
    } else if (match_token(Tag::ADD)){
        // + EXP
        next();
        ASTPtr exp = unary();
        if (!exp) {
            exit(103);
        }
        return make_unique<UnaryAST>(Operator::add_op, move(exp));
    } else if (match_token(Tag::SUB)){
        // - EXP
        next();
        ASTPtr exp = unary();
        if (!exp) {
            exit(104);
        }
        return make_unique<UnaryAST>(Operator::sub_op, move(exp));
    } else if (match_token(Tag::NOT)){
        // ! EXP
        next();
        ASTPtr exp = unary();
        if (!exp) {
            exit(105);
        }
        return make_unique<UnaryAST>(Operator::not_op, move(exp));
    } else if (match_token(Tag::ID)){
        Id* token_casted = (Id*)token;
        string id_name = token_casted->name;
        next();
        // Function call: Id (params)
        if (match_token(Tag::LPAREN)) {
            next();
            // id(): no params
            if (match_token(Tag::RPAREN)) {
                ASTPtr function_call = make_unique<FuncCallAST>(id_name);
                next(); // 消耗右括号
                return function_call;
            } else {
                ASTPtrList params;
                while (true) {
                    ASTPtr param = binary_add();
                    if (!param) {
                        exit(106);
                    }
                    params.push_back(move(param));
                    // id(a,b,c)
                    if (match_token(Tag::COMMA) == false) break;
                    next(); // ,
                }
                if (match_token(Tag::RPAREN) == false) {
                    exit(107);
                }
                next(); // )
                return make_unique<FuncCallAST>(id_name, move(params));
            }
        } else if (match_token(Tag::LBRACKET)) { // LVal: array (id[exp])
            ASTPtrList position;
            while (match_token(Tag::LBRACKET)) {
                next(); // [
                ASTPtr sub_position = binary_add();
                position.push_back(move(sub_position));
                if (match_token(Tag::RBRACKET) == false) {
                    exit(108);
                }
                next();
            }
            return make_unique<LValAST>(id_name, array_t, move(position));
        } else { // LVal: var (id)
            return make_unique<LValAST>(id_name, var_t);
        }
    }
    cout << "error 55" << endl;
    exit(55);
}

ASTPtr Parser::statement(void) {
    if (match_token(Tag::SEMICON)) {
        next(); // ;
        return make_unique<StmtAST>(make_unique<EmptyAST>());
    }
    else if (match_token(Tag::LBRACE)) {
        ASTPtr body = block();
        if (!body) {
            exit(106);
        }
        return make_unique<StmtAST>(move(body));
    }
    else if (match_token(Tag::KW_WHILE)) {
        ASTPtr stmt = while_loop();
        if (!stmt) {
            exit(107);
        }
        return make_unique<StmtAST>(move(stmt));
    }
    else if (match_token(Tag::KW_IF)) {
        ASTPtr stmt = if_else();
        if (!stmt) {
            exit(108);
        }
        return make_unique<StmtAST>(move(stmt));
    }
    else if (match_token(Tag::KW_BREAK) || match_token(Tag::KW_CONTINUE) || match_token(Tag::KW_RETURN)) {
        Tag temp = token->tag;
        next();
        ASTPtr stmt;
        if (token->tag == Tag::SEMICON)
        { // break; return; continue;
            Control command;
            switch (temp) {
                case Tag::KW_BREAK: {
                    command = Control::break_c;
                    break;
                }
                case Tag::KW_CONTINUE: {
                    command = Control::continue_c;
                    break;
                }
                case Tag::KW_RETURN: {
                    command = Control::return_c;
                    break;
                }
                default: break;
            }
            stmt = make_unique<ControlAST>(command);
        } else { // return exp;
            ASTPtr return_exp = binary_add();
            if (!return_exp) {
                exit(109);
            }
            if (!match_token(Tag::SEMICON)) {
                exit(110);
            }
            stmt = make_unique<ControlAST>(Control::return_c, move(return_exp));
        }
        next(); // ;
        return make_unique<StmtAST>(move(stmt));
    } else {
        ASTPtr exp = binary_add();
        if (!exp) {
            exit(111);
        }
        if (dynamic_cast<LValAST *>(exp.get())) {
            // LVal = exp;
            if (match_token(Tag::ASSIGN)) {
                next(); // =
                ASTPtr rhs = binary_add();
                if (!rhs) {
                    exit(112);
                }
                ASTPtr stmt = make_unique<AssignAST>(move(exp), move(rhs));
                if (!match_token(Tag::SEMICON)) {
                    exit(113);
                }
                next(); // ;
                return make_unique<StmtAST>(move(stmt));
            } else if (match_token(Tag::SEMICON)) {
                // exp;
                next(); // ;
                return make_unique<StmtAST>(move(exp));
            } else {
                exit(114);
            }
        } else {
            // exp;
            if (!match_token(Tag::SEMICON)) {
                exit(115);
            }
            next(); // ;
            return make_unique<StmtAST>(move(exp));
        }
    }
    exit(56);
}

ASTPtr Parser::if_else(void) {
    next(); // if () then else
    if (!match_token(Tag::LPAREN)) {
        exit(116);
    }
    next(); // (
    ASTPtr condition = binary_or();
    if (!condition) {
        exit(117);
    }
    if (!match_token(Tag::RPAREN)) {
        exit(118);
    }
    next(); // )
    ASTPtr thenStatement = statement();
    if (!thenStatement) {
        exit(118);
    }
    if (match_token(Tag::KW_ELSE)) {
        next(); // else
        ASTPtr elseStatement = statement();
        if (!elseStatement) {
            exit(119);
        }
        return make_unique<IfAST>(move(condition), move(thenStatement), move(elseStatement));
    } else {
        return make_unique<IfAST>(move(condition), move(thenStatement));
    }
    exit(57);
}

ASTPtr Parser::while_loop(void) {
    next(); // while () stmt
    if (!match_token(Tag::LPAREN)) {
        exit(116);
    }
    next(); // (
    ASTPtr condition = binary_or();
    if (!condition) {
        exit(117);
    }
    if (!match_token(Tag::RPAREN)) {
        exit(118);
    }
    next(); // )
    ASTPtr stmt = statement();
    if (!stmt) {
        exit(119);
    }
    return make_unique<WhileAST>(move(condition), move(stmt));
}

ASTPtr Parser::init_val(void) {
    if (match_token(Tag::LBRACE)) {
        next();
        if (match_token(Tag::RBRACE)) {
            next();
            return make_unique<InitValAST>(VarType::array_t, ASTPtrList{});
        } else {
            ASTPtrList inits;
            while (true) {
                ASTPtr init = init_val();
                if (!init) {
                    cout << "error 999" << endl;
                    exit(999);
                }
                inits.push_back(move(init));
                if (!match_token(Tag::COMMA))
                    break;
                next(); // ,
            }
            if (!match_token(RBRACE)) {
                exit(998);
            }
            next(); // }
            return make_unique<InitValAST>(VarType::array_t, move(inits));
        }
    } else {
        ASTPtr exp = binary_add();
        if (!exp) {
            cout << "error 1000" << endl;
            exit(1000);
        }
        ASTPtrList expList;
        expList.push_back(move(exp));
        return make_unique<InitValAST>(VarType::var_t, move(expList));
    }
}

ASTPtr Parser::var_decl() {
    bool isConst = false;
    if (match_token(Tag::KW_CONST)) {
        isConst = true;
        next();
    }
    
    // TODO: only support int here
    if (!match_token(Tag::KW_INT)) {
        cout << "Only Support Type 'int'." << endl;
        exit(450);
    }
    next();

    ASTPtrList vars;
    ASTPtr varDef = var_def(isConst);
    if (!varDef) {
        exit(451);
    }
    vars.push_back(move(varDef));

    while (match_token(Tag::COMMA)) {
        next(); // ,
        ASTPtr varDef = var_def(isConst);
        if (!varDef) {
            exit(451);
        }
        vars.push_back(move(varDef));
    }
    
    if (!match_token(Tag::SEMICON)) {
        exit(452);
    }
    next();
    return make_unique<VarDeclAST>(isConst, move(vars));
}

ASTPtr Parser::var_def(bool isConst) {
    if (!match_token(Tag::ID)) {
        exit(452);
    }
    Id* token_casted = (Id*)token;
    string id_name = token_casted->name;
    ASTPtrList dims;
    next(); // id
    while (match_token(Tag::LBRACKET)) {
        next(); // [
        ASTPtr exp = binary_add();
        if (!exp) {
            exit(453);
        }
        dims.push_back(move(exp));
        if (!match_token(Tag::RBRACKET)) {
            exit(454);
        }
        next(); // ]
    }
    ASTPtr var;
    if (dims.empty())
        var = make_unique<IdAST>(id_name, VarType::var_t, isConst);
    else 
        var = make_unique<IdAST>(id_name, VarType::array_t, isConst, move(dims));
    if (match_token(Tag::ASSIGN)) {
        next(); // =
        ASTPtr init = init_val();
        if (!init) {
            exit(456);
        }
        return make_unique<VarDefAST>(isConst, move(var), move(init));
    } else {
        if (isConst) {
            exit(457);
        }
        return make_unique<VarDefAST>(isConst, move(var));
    }
}

ASTPtr Parser::block(void) {
    next(); // {
    if (match_token(Tag::RBRACE)) {
        next(); // }
        return make_unique<BlockAST>(ASTPtrList{});
    } else {
        ASTPtrList stmts;
        while (!match_token(Tag::RBRACE)) {
            if (match_token(Tag::KW_CONST) || match_token(Tag::KW_INT)) {
                ASTPtr var = var_decl();
                if (!var) {
                    exit(460);
                }
                stmts.push_back(move(var));
            } else {
                ASTPtr stmt = statement();
                if (!stmt) {
                    exit(461);
                }
                stmts.push_back(move(stmt));
            }
        }
        next(); // }
        return make_unique<BlockAST>(move(stmts));
    }
}

ASTPtr Parser::function_def(void) {
    // function type
    Type type;
    if (match_token(Tag::KW_INT)) type = Type::int_t;
    if (match_token(Tag::KW_CHAR)) type = Type::char_t;
    if (match_token(Tag::KW_VOID)) type = Type::void_t;
    next(); // type
    if (!match_token(Tag::ID)) {
        exit(999);
    }
    // function name
    Id* token_casted = (Id*)token;
    string id_name = token_casted->name;
    next(); // id
    if (!match_token(Tag::LPAREN)) {
        exit(998);
    }
    next(); // (
    ASTPtrList args;
    if (!match_token(Tag::RPAREN)) {
        while (true) {
            // TODO: only support int
            if ((!match_token(Tag::KW_INT))) {
                exit(996);
            }
            next(); // type
            if (!match_token(Tag::ID)) {
                exit(998);
            }
            // arg name
            Id* token_casted = (Id*)token;
            string arg_name = token_casted->name;
            next(); // id
            if (match_token(Tag::LBRACKET)) // [
            {
                ASTPtrList dim;
                dim.push_back(make_unique<NumAST>(0));
                next(); // [
                if (!match_token(Tag::RBRACKET))
                {
                    exit(997);
                }
                next(); // ]
                while (match_token(Tag::LBRACKET))
                {
                    next(); // [
                    ASTPtr _dim = binary_add();
                    if (!_dim)
                    {
                        exit(995);
                    }
                    dim.push_back(move(_dim));
                    if (!match_token(Tag::RBRACKET))
                    {
                        exit(994);
                    }
                    next(); // ]
                }
                args.push_back(make_unique<IdAST>(arg_name, VarType::array_t, false, move(dim)));
            } else {
                args.push_back(make_unique<IdAST>(arg_name, VarType::var_t, false));
            }
            if (!match_token(Tag::COMMA))
                break;
            next(); // ,
        }
        if (!match_token(Tag::RPAREN)) {
            exit(993);
        }
    }
    next(); // )
    ASTPtr body = block();
    return make_unique<FuncDefAST>(type, id_name, move(args), move(body));
}

bool Parser::is_done(void) const {
    return lexer.is_done();
}