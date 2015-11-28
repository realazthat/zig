/*
 * Copyright (c) 2015 Andrew Kelley
 *
 * This file is part of zig, which is MIT licensed.
 * See http://opensource.org/licenses/MIT
 */

#include "parser.hpp"

#include <stdarg.h>
#include <stdio.h>

__attribute__ ((format (printf, 2, 3)))
__attribute__ ((noreturn))
static void ast_error(Token *token, const char *format, ...) {
    int line = token->start_line + 1;
    int column = token->start_column + 1;

    va_list ap;
    va_start(ap, format);
    fprintf(stderr, "Error: Line %d, column %d: ", line, column);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(EXIT_FAILURE);
}

const char *node_type_str(NodeType node_type) {
    switch (node_type) {
        case NodeTypeRoot:
            return "Root";
        case NodeTypeFnDef:
            return "FnDef";
        case NodeTypeFnDecl:
            return "FnDecl";
        case NodeTypeFnProto:
            return "FnProto";
        case NodeTypeParamDecl:
            return "ParamDecl";
        case NodeTypeType:
            return "Type";
        case NodeTypeBlock:
            return "Block";
        case NodeTypeStatementReturn:
            return "StatementReturn";
        case NodeTypeExpression:
            return "Expression";
        case NodeTypeFnCall:
            return "FnCall";
        case NodeTypeExternBlock:
            return "ExternBlock";
        case NodeTypeDirective:
            return "Directive";
    }
    zig_unreachable();
}

void ast_print(AstNode *node, int indent) {
    for (int i = 0; i < indent; i += 1) {
        fprintf(stderr, " ");
    }

    switch (node->type) {
        case NodeTypeRoot:
            fprintf(stderr, "%s\n", node_type_str(node->type));
            for (int i = 0; i < node->data.root.top_level_decls.length; i += 1) {
                AstNode *child = node->data.root.top_level_decls.at(i);
                ast_print(child, indent + 2);
            }
            break;
        case NodeTypeFnDef:
            {
                fprintf(stderr, "%s\n", node_type_str(node->type));
                AstNode *child = node->data.fn_def.fn_proto;
                ast_print(child, indent + 2);
                ast_print(node->data.fn_def.body, indent + 2);
                break;
            }
        case NodeTypeFnProto:
            {
                Buf *name_buf = &node->data.fn_proto.name;
                fprintf(stderr, "%s '%s'\n", node_type_str(node->type), buf_ptr(name_buf));

                for (int i = 0; i < node->data.fn_proto.params.length; i += 1) {
                    AstNode *child = node->data.fn_proto.params.at(i);
                    ast_print(child, indent + 2);
                }

                ast_print(node->data.fn_proto.return_type, indent + 2);

                break;
            }
        case NodeTypeBlock:
            {
                fprintf(stderr, "%s\n", node_type_str(node->type));
                for (int i = 0; i < node->data.block.statements.length; i += 1) {
                    AstNode *child = node->data.block.statements.at(i);
                    ast_print(child, indent + 2);
                }
                break;
            }
        case NodeTypeParamDecl:
            {
                Buf *name_buf = &node->data.param_decl.name;
                fprintf(stderr, "%s '%s'\n", node_type_str(node->type), buf_ptr(name_buf));

                ast_print(node->data.param_decl.type, indent + 2);

                break;
            }
        case NodeTypeType:
            switch (node->data.type.type) {
                case AstNodeTypeTypePrimitive:
                    {
                        Buf *name_buf = &node->data.type.primitive_name;
                        fprintf(stderr, "%s '%s'\n", node_type_str(node->type), buf_ptr(name_buf));
                        break;
                    }
                case AstNodeTypeTypePointer:
                    {
                        const char *const_or_mut_str = node->data.type.is_const ? "const" : "mut";
                        fprintf(stderr, "'%s' PointerType\n", const_or_mut_str);

                        ast_print(node->data.type.child_type, indent + 2);
                        break;
                    }
            }
            break;
        case NodeTypeStatementReturn:
            fprintf(stderr, "ReturnStatement\n");
            if (node->data.statement_return.expression)
                ast_print(node->data.statement_return.expression, indent + 2);
            break;
        case NodeTypeExternBlock:
            {
                fprintf(stderr, "%s\n", node_type_str(node->type));
                for (int i = 0; i < node->data.extern_block.fn_decls.length; i += 1) {
                    AstNode *child = node->data.extern_block.fn_decls.at(i);
                    ast_print(child, indent + 2);
                }
                break;
            }
        case NodeTypeFnDecl:
            fprintf(stderr, "%s\n", node_type_str(node->type));
            ast_print(node->data.fn_decl.fn_proto, indent + 2);
            break;
        case NodeTypeExpression:
            switch (node->data.expression.type) {
                case AstNodeExpressionTypeNumber:
                    fprintf(stderr, "NumberLiteralExpression %s\n", buf_ptr(&node->data.expression.data.number));
                    break;
                case AstNodeExpressionTypeString:
                    fprintf(stderr, "StringLiteralExpression '%s'\n", buf_ptr(&node->data.expression.data.string));
                    break;
                case AstNodeExpressionTypeFnCall:
                    fprintf(stderr, "FnCallExpression\n");
                    ast_print(node->data.expression.data.fn_call, indent + 2);
                    break;
                case AstNodeExpressionTypeUnreachable:
                    fprintf(stderr, "UnreachableExpression\n");
                    break;
            }
            break;
        case NodeTypeFnCall:
            fprintf(stderr, "%s '%s'\n", node_type_str(node->type), buf_ptr(&node->data.fn_call.name));
            for (int i = 0; i < node->data.fn_call.params.length; i += 1) {
                AstNode *child = node->data.fn_call.params.at(i);
                ast_print(child, indent + 2);
            }
            break;
        case NodeTypeDirective:
            fprintf(stderr, "%s\n", node_type_str(node->type));
            break;
    }
}

struct ParseContext {
    Buf *buf;
    AstNode *root;
    ZigList<Token> *tokens;
    ZigList<AstNode *> *directive_list;
};

static AstNode *ast_create_node_no_line_info(NodeType type) {
    AstNode *node = allocate<AstNode>(1);
    node->type = type;
    return node;
}

static void ast_update_node_line_info(AstNode *node, Token *first_token) {
    node->line = first_token->start_line;
    node->column = first_token->start_column;
}

static AstNode *ast_create_node(NodeType type, Token *first_token) {
    AstNode *node = ast_create_node_no_line_info(type);
    ast_update_node_line_info(node, first_token);
    return node;
}

static AstNode *ast_create_node_with_node(NodeType type, AstNode *other_node) {
    AstNode *node = ast_create_node_no_line_info(type);
    node->line = other_node->line;
    node->column = other_node->column;
    return node;
}

static AstNode *ast_create_void_type_node(ParseContext *pc, Token *token) {
    AstNode *node = ast_create_node(NodeTypeType, token);
    node->data.type.type = AstNodeTypeTypePrimitive;
    buf_init_from_str(&node->data.type.primitive_name, "void");
    return node;
}

static void ast_buf_from_token(ParseContext *pc, Token *token, Buf *buf) {
    buf_init_from_mem(buf, buf_ptr(pc->buf) + token->start_pos, token->end_pos - token->start_pos);
}

static void parse_string_literal(ParseContext *pc, Token *token, Buf *buf) {
    // skip the double quotes at beginning and end
    // convert escape sequences

    buf_resize(buf, 0);
    bool escape = false;
    for (int i = token->start_pos + 1; i < token->end_pos - 1; i += 1) {
        uint8_t c = *((uint8_t*)buf_ptr(pc->buf) + i);
        if (escape) {
            switch (c) {
                case '\\':
                    buf_append_char(buf, '\\');
                    break;
                case 'r':
                    buf_append_char(buf, '\r');
                    break;
                case 'n':
                    buf_append_char(buf, '\n');
                    break;
                case 't':
                    buf_append_char(buf, '\t');
                    break;
                case '"':
                    buf_append_char(buf, '"');
                    break;
            }
            escape = false;
        } else if (c == '\\') {
            escape = true;
        } else {
            buf_append_char(buf, c);
        }
    }
    assert(!escape);
}

__attribute__ ((noreturn))
void ast_invalid_token_error(ParseContext *pc, Token *token) {
    Buf token_value = BUF_INIT;
    ast_buf_from_token(pc, token, &token_value);
    ast_error(token, "invalid token: '%s'", buf_ptr(&token_value));
}

static AstNode *ast_parse_expression(ParseContext *pc, int *token_index, bool mandatory);


static void ast_expect_token(ParseContext *pc, Token *token, TokenId token_id) {
    if (token->id != token_id) {
        ast_invalid_token_error(pc, token);
    }
}

static AstNode *ast_parse_directive(ParseContext *pc, int token_index, int *new_token_index) {
    Token *number_sign = &pc->tokens->at(token_index);
    token_index += 1;
    ast_expect_token(pc, number_sign, TokenIdNumberSign);

    AstNode *node = ast_create_node(NodeTypeDirective, number_sign);

    Token *name_symbol = &pc->tokens->at(token_index);
    token_index += 1;
    ast_expect_token(pc, name_symbol, TokenIdSymbol);

    ast_buf_from_token(pc, name_symbol, &node->data.directive.name);

    Token *l_paren = &pc->tokens->at(token_index);
    token_index += 1;
    ast_expect_token(pc, l_paren, TokenIdLParen);

    Token *param_str = &pc->tokens->at(token_index);
    token_index += 1;
    ast_expect_token(pc, param_str, TokenIdStringLiteral);

    parse_string_literal(pc, param_str, &node->data.directive.param);

    Token *r_paren = &pc->tokens->at(token_index);
    token_index += 1;
    ast_expect_token(pc, r_paren, TokenIdRParen);

    *new_token_index = token_index;
    return node;
}

static void ast_parse_directives(ParseContext *pc, int *token_index,
        ZigList<AstNode *> *directives)
{
    for (;;) {
        Token *token = &pc->tokens->at(*token_index);
        if (token->id == TokenIdNumberSign) {
            AstNode *directive_node = ast_parse_directive(pc, *token_index, token_index);
            directives->append(directive_node);
        } else {
            return;
        }
    }
    zig_unreachable();
}


/*
Type : token(Symbol) | PointerType | token(Unreachable)
PointerType : token(Star) token(Const) Type  | token(Star) token(Mut) Type;
*/
static AstNode *ast_parse_type(ParseContext *pc, int token_index, int *new_token_index) {
    Token *token = &pc->tokens->at(token_index);
    token_index += 1;

    AstNode *node = ast_create_node(NodeTypeType, token);

    if (token->id == TokenIdKeywordUnreachable) {
        node->data.type.type = AstNodeTypeTypePrimitive;
        buf_init_from_str(&node->data.type.primitive_name, "unreachable");
    } else if (token->id == TokenIdSymbol) {
        node->data.type.type = AstNodeTypeTypePrimitive;
        ast_buf_from_token(pc, token, &node->data.type.primitive_name);
    } else if (token->id == TokenIdStar) {
        node->data.type.type = AstNodeTypeTypePointer;
        Token *const_or_mut = &pc->tokens->at(token_index);
        token_index += 1;
        if (const_or_mut->id == TokenIdKeywordMut) {
            node->data.type.is_const = false;
        } else if (const_or_mut->id == TokenIdKeywordConst) {
            node->data.type.is_const = true;
        } else {
            ast_invalid_token_error(pc, const_or_mut);
        }

        node->data.type.child_type = ast_parse_type(pc, token_index, &token_index);
    } else {
        ast_invalid_token_error(pc, token);
    }

    *new_token_index = token_index;
    return node;
}

/*
ParamDecl<node> : token(Symbol) token(Colon) Type {
};
*/
static AstNode *ast_parse_param_decl(ParseContext *pc, int token_index, int *new_token_index) {
    Token *param_name = &pc->tokens->at(token_index);
    token_index += 1;
    ast_expect_token(pc, param_name, TokenIdSymbol);

    AstNode *node = ast_create_node(NodeTypeParamDecl, param_name);


    ast_buf_from_token(pc, param_name, &node->data.param_decl.name);

    Token *colon = &pc->tokens->at(token_index);
    token_index += 1;
    ast_expect_token(pc, colon, TokenIdColon);

    node->data.param_decl.type = ast_parse_type(pc, token_index, &token_index);

    *new_token_index = token_index;
    return node;
}


static void ast_parse_param_decl_list(ParseContext *pc, int token_index, int *new_token_index,
        ZigList<AstNode *> *params)
{
    Token *l_paren = &pc->tokens->at(token_index);
    token_index += 1;
    ast_expect_token(pc, l_paren, TokenIdLParen);

    Token *token = &pc->tokens->at(token_index);
    if (token->id == TokenIdRParen) {
        token_index += 1;
        *new_token_index = token_index;
        return;
    }

    for (;;) {
        AstNode *param_decl_node = ast_parse_param_decl(pc, token_index, &token_index);
        params->append(param_decl_node);

        Token *token = &pc->tokens->at(token_index);
        token_index += 1;
        if (token->id == TokenIdRParen) {
            *new_token_index = token_index;
            return;
        } else {
            ast_expect_token(pc, token, TokenIdComma);
        }
    }
    zig_unreachable();
}

static void ast_parse_fn_call_param_list(ParseContext *pc, int token_index, int *new_token_index,
        ZigList<AstNode*> *params)
{
    Token *l_paren = &pc->tokens->at(token_index);
    token_index += 1;
    ast_expect_token(pc, l_paren, TokenIdLParen);

    Token *token = &pc->tokens->at(token_index);
    if (token->id == TokenIdRParen) {
        token_index += 1;
        *new_token_index = token_index;
        return;
    }

    for (;;) {
        AstNode *expr = ast_parse_expression(pc, &token_index, true);
        params->append(expr);

        Token *token = &pc->tokens->at(token_index);
        token_index += 1;
        if (token->id == TokenIdRParen) {
            *new_token_index = token_index;
            return;
        } else {
            ast_expect_token(pc, token, TokenIdComma);
        }
    }
    zig_unreachable();
}

/*
FnCall : token(Symbol) token(LParen) list(Expression, token(Comma)) token(RParen) ;
*/
static AstNode *ast_parse_fn_call(ParseContext *pc, int token_index, int *new_token_index) {
    Token *fn_name = &pc->tokens->at(token_index);
    token_index += 1;
    ast_expect_token(pc, fn_name, TokenIdSymbol);

    AstNode *node = ast_create_node(NodeTypeFnCall, fn_name);


    ast_buf_from_token(pc, fn_name, &node->data.fn_call.name);

    ast_parse_fn_call_param_list(pc, token_index, &token_index, &node->data.fn_call.params);

    *new_token_index = token_index;
    return node;
}

/*
Expression : token(Number) | token(String) | token(Unreachable) | FnCall
*/
static AstNode *ast_parse_expression(ParseContext *pc, int *token_index, bool mandatory) {
    Token *token = &pc->tokens->at(*token_index);
    AstNode *node = ast_create_node(NodeTypeExpression, token);
    if (token->id == TokenIdKeywordUnreachable) {
        node->data.expression.type = AstNodeExpressionTypeUnreachable;
        *token_index += 1;
    } else if (token->id == TokenIdSymbol) {
        node->data.expression.type = AstNodeExpressionTypeFnCall;
        node->data.expression.data.fn_call = ast_parse_fn_call(pc, *token_index, token_index);
    } else if (token->id == TokenIdNumberLiteral) {
        node->data.expression.type = AstNodeExpressionTypeNumber;
        ast_buf_from_token(pc, token, &node->data.expression.data.number);
        *token_index += 1;
    } else if (token->id == TokenIdStringLiteral) {
        node->data.expression.type = AstNodeExpressionTypeString;
        parse_string_literal(pc, token, &node->data.expression.data.string);
        *token_index += 1;
    } else if (mandatory) {
        ast_invalid_token_error(pc, token);
    } else {
        return nullptr;
    }

    return node;
}

/*
Statement : ExpressionStatement  | ReturnStatement ;

ExpressionStatement : Expression token(Semicolon) ;

ReturnStatement : token(Return) option(Expression) token(Semicolon) ;
*/
static AstNode *ast_parse_statement(ParseContext *pc, int token_index, int *new_token_index) {
    Token *token = &pc->tokens->at(token_index);
    if (token->id == TokenIdKeywordReturn) {
        AstNode *node = ast_create_node(NodeTypeStatementReturn, token);
        token_index += 1;
        node->data.statement_return.expression = ast_parse_expression(pc, &token_index, false);

        Token *semicolon = &pc->tokens->at(token_index);
        token_index += 1;
        ast_expect_token(pc, semicolon, TokenIdSemicolon);
        *new_token_index = token_index;
        return node;
    } else if (token->id == TokenIdSymbol ||
               token->id == TokenIdStringLiteral ||
               token->id == TokenIdKeywordUnreachable ||
               token->id == TokenIdNumberLiteral)
    {
        AstNode *node = ast_parse_expression(pc, &token_index, true);

        Token *semicolon = &pc->tokens->at(token_index);
        token_index += 1;
        ast_expect_token(pc, semicolon, TokenIdSemicolon);
        *new_token_index = token_index;
        return node;
    } else {
        ast_invalid_token_error(pc, token);
    }
}

/*
Block : token(LBrace) many(Statement) token(RBrace);
*/
static AstNode *ast_parse_block(ParseContext *pc, int token_index, int *new_token_index) {
    Token *l_brace = &pc->tokens->at(token_index);
    token_index += 1;
    ast_expect_token(pc, l_brace, TokenIdLBrace);

    AstNode *node = ast_create_node(NodeTypeBlock, l_brace);


    for (;;) {
        Token *token = &pc->tokens->at(token_index);
        if (token->id == TokenIdRBrace) {
            token_index += 1;
            *new_token_index = token_index;
            return node;
        } else {
            AstNode *statement_node = ast_parse_statement(pc, token_index, &token_index);
            node->data.block.statements.append(statement_node);
        }
    }
    zig_unreachable();
}

/*
FnProto : many(Directive) option(FnVisibleMod) token(Fn) token(Symbol) ParamDeclList option(token(Arrow) Type)
*/
static AstNode *ast_parse_fn_proto(ParseContext *pc, int *token_index, bool mandatory) {
    Token *token = &pc->tokens->at(*token_index);

    FnProtoVisibMod visib_mod;

    if (token->id == TokenIdKeywordPub) {
        visib_mod = FnProtoVisibModPub;
        *token_index += 1;

        Token *fn_token = &pc->tokens->at(*token_index);
        *token_index += 1;
        ast_expect_token(pc, fn_token, TokenIdKeywordFn);
    } else if (token->id == TokenIdKeywordExport) {
        visib_mod = FnProtoVisibModExport;
        *token_index += 1;

        Token *fn_token = &pc->tokens->at(*token_index);
        *token_index += 1;
        ast_expect_token(pc, fn_token, TokenIdKeywordFn);
    } else if (token->id == TokenIdKeywordFn) {
        visib_mod = FnProtoVisibModPrivate;
        *token_index += 1;
    } else if (mandatory) {
        ast_invalid_token_error(pc, token);
    } else {
        return nullptr;
    }

    AstNode *node = ast_create_node(NodeTypeFnProto, token);
    node->data.fn_proto.visib_mod = visib_mod;
    node->data.fn_proto.directives = pc->directive_list;
    pc->directive_list = nullptr;


    Token *fn_name = &pc->tokens->at(*token_index);
    *token_index += 1;
    ast_expect_token(pc, fn_name, TokenIdSymbol);

    ast_buf_from_token(pc, fn_name, &node->data.fn_proto.name);


    ast_parse_param_decl_list(pc, *token_index, token_index, &node->data.fn_proto.params);

    Token *arrow = &pc->tokens->at(*token_index);
    if (arrow->id == TokenIdArrow) {
        *token_index += 1;
        node->data.fn_proto.return_type = ast_parse_type(pc, *token_index, token_index);
    } else {
        node->data.fn_proto.return_type = ast_create_void_type_node(pc, arrow);
    }

    return node;
}

/*
FnDef : FnProto Block
*/
static AstNode *ast_parse_fn_def(ParseContext *pc, int *token_index, bool mandatory) {
    AstNode *fn_proto = ast_parse_fn_proto(pc, token_index, mandatory);
    if (!fn_proto)
        return nullptr;
    AstNode *node = ast_create_node_with_node(NodeTypeFnDef, fn_proto);

    node->data.fn_def.fn_proto = fn_proto;
    node->data.fn_def.body = ast_parse_block(pc, *token_index, token_index);

    return node;
}

/*
FnDecl : FnProto token(Semicolon)
*/
static AstNode *ast_parse_fn_decl(ParseContext *pc, int token_index, int *new_token_index) {
    AstNode *fn_proto = ast_parse_fn_proto(pc, &token_index, true);
    AstNode *node = ast_create_node_with_node(NodeTypeFnDecl, fn_proto);

    node->data.fn_decl.fn_proto = fn_proto;

    Token *semicolon = &pc->tokens->at(token_index);
    token_index += 1;
    ast_expect_token(pc, semicolon, TokenIdSemicolon);

    *new_token_index = token_index;
    return node;
}

/*
Directive : token(NumberSign) token(Symbol) token(LParen) token(String) token(RParen)
*/
/*
ExternBlock : many(Directive) token(Extern) token(LBrace) many(FnProtoDecl) token(RBrace)
*/
static AstNode *ast_parse_extern_block(ParseContext *pc, int *token_index, bool mandatory) {
    Token *extern_kw = &pc->tokens->at(*token_index);
    if (extern_kw->id != TokenIdKeywordExtern) {
        if (mandatory)
            ast_invalid_token_error(pc, extern_kw);
        else
            return nullptr;
    }
    *token_index += 1;

    AstNode *node = ast_create_node(NodeTypeExternBlock, extern_kw);

    node->data.extern_block.directives = pc->directive_list;
    pc->directive_list = nullptr;

    Token *l_brace = &pc->tokens->at(*token_index);
    *token_index += 1;
    ast_expect_token(pc, l_brace, TokenIdLBrace);

    for (;;) {
        Token *directive_token = &pc->tokens->at(*token_index);
        assert(!pc->directive_list);
        pc->directive_list = allocate<ZigList<AstNode*>>(1);
        ast_parse_directives(pc, token_index, pc->directive_list);

        Token *token = &pc->tokens->at(*token_index);
        if (token->id == TokenIdRBrace) {
            if (pc->directive_list->length > 0) {
                ast_error(directive_token, "invalid directive");
            }
            pc->directive_list = nullptr;

            *token_index += 1;
            return node;
        } else {
            AstNode *child = ast_parse_fn_decl(pc, *token_index, token_index);
            node->data.extern_block.fn_decls.append(child);
        }
    }


    zig_unreachable();
}

static void ast_parse_top_level_decls(ParseContext *pc, int *token_index, ZigList<AstNode *> *top_level_decls) {
    for (;;) {
        Token *directive_token = &pc->tokens->at(*token_index);
        assert(!pc->directive_list);
        pc->directive_list = allocate<ZigList<AstNode*>>(1);
        ast_parse_directives(pc, token_index, pc->directive_list);

        AstNode *fn_decl_node = ast_parse_fn_def(pc, token_index, false);
        if (fn_decl_node) {
            top_level_decls->append(fn_decl_node);
            continue;
        }

        AstNode *extern_node = ast_parse_extern_block(pc, token_index, false);
        if (extern_node) {
            top_level_decls->append(extern_node);
            continue;
        }

        if (pc->directive_list->length > 0) {
            ast_error(directive_token, "invalid directive");
        }
        pc->directive_list = nullptr;

        return;
    }
    zig_unreachable();
}

AstNode *ast_parse(Buf *buf, ZigList<Token> *tokens) {
    ParseContext pc = {0};
    pc.buf = buf;
    pc.root = ast_create_node(NodeTypeRoot, &tokens->at(0));
    pc.tokens = tokens;

    int token_index = 0;
    ast_parse_top_level_decls(&pc, &token_index, &pc.root->data.root.top_level_decls);

    if (token_index != tokens->length - 1) {
        ast_invalid_token_error(&pc, &tokens->at(token_index));
    }

    return pc.root;
}