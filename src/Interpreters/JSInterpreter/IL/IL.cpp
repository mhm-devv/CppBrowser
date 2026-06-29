#include "IL.h"

using namespace std::string_view_literals;

std::unordered_map<std::string_view, IL::InstructionType> IL::Emitter::operator_to_instruction_type = {
    {"+"sv, InstructionType::Add}, {"-"sv, InstructionType::Sub},
    {"*"sv, InstructionType::Mul}, {"/"sv, InstructionType::Div},
    {"%"sv, InstructionType::Mod}, {">"sv, InstructionType::BT},
    {"<"sv, InstructionType::LT}, {">="sv, InstructionType::BEQ},
    {"<="sv, InstructionType::LEQ}, {"=="sv, InstructionType::EQ},
    {"!="sv, InstructionType::NEQ}, {"!=="sv, InstructionType::SNEQ},
    {"==="sv, InstructionType::SEQ}
};

// Program is an ILProgram.
// While Ast::Program is an Abstract Syntax Tree Program.

IL::Program IL::Emitter::emitProgram(const Ast::Program* ast) {
    current_elements.clear();
    for (auto& statement : ast->statements) {
        emitNode(statement.get());
    }
    return Program{std::move(current_elements)};
}

void IL::Emitter::emitNode(const Ast::Statement* statement) {}
void IL::Emitter::emitVarDecl(const Ast::VarDecl* var_decl) {
    Operand value = emitExpr(var_decl->value.get());

    current_elements.push_back(Element{ElementType::Instruction, InstructionType::Push, std::move(value)});
    var_name_to_ptr.insert({var_decl->name, stack_ptr});
}
void IL::Emitter::emitVarReInit(const Ast::VarReInit* var_reinit) {
    if (const auto itr = var_name_to_ptr.find(var_reinit->name); itr != var_name_to_ptr.end()) {
        current_elements.push_back(Element{ElementType::Instruction, InstructionType::Set, Operand{OperandType::MemoryAddress, true, new size_t{evaluateMemAddr(itr->second)}}});
        current_elements.push_back(Element{ElementType::Operand, InstructionType::None, emitExpr(var_reinit->value.get())});
    }
    throw std::runtime_error("Unknown Identifier at IL.cpp, IL::Emitter::emitVarReInit");
}

void IL::Emitter::emitIf(const Ast::If *if_stmt) {
    Operand if_condition = emitExpr(if_stmt->condition.get());
    const size_t jmp_if_not_before_addr = current_elements.size();
    current_elements.resize(current_elements.size() + 2);
    emitNode(if_stmt->statement.get());

    const size_t jmp_if_after_addr = current_elements.size();
    current_elements.resize(current_elements.size() + 1);

    const size_t jmp_if_not_to_addr = current_elements.size();
    current_elements[jmp_if_not_before_addr] = Element{ElementType::Instruction, InstructionType::JmpIfNot, Operand{OperandType::InstructionAddress, true, new size_t{jmp_if_not_to_addr}}};
    current_elements[jmp_if_not_before_addr + 1] = Element{ElementType::Operand, InstructionType::None, std::move(if_condition)};

    std::vector<size_t> jmp_else_ifs_after_addrs;

    for (const auto& else_if_stmt : if_stmt->else_if_stmts) {
        Operand else_if_condition = emitExpr(else_if_stmt->condition.get());
        const size_t jmp_else_if_not_addr = current_elements.size();
        current_elements.resize(current_elements.size() + 2);
        emitNode(else_if_stmt->statement.get());

        jmp_else_ifs_after_addrs.push_back(current_elements.size());
        current_elements.resize(current_elements.size() + 1);

        const size_t jmp_else_if_not_to_addr = current_elements.size();
        current_elements[jmp_else_if_not_addr] = Element{ElementType::Instruction, InstructionType::JmpIfNot,  Operand{OperandType::InstructionAddress, true, new size_t{jmp_else_if_not_to_addr}}};
        current_elements[jmp_else_if_not_addr + 1] = Element{ElementType::Operand, InstructionType::None, std::move(else_if_condition)};

    }

    if (if_stmt->else_stmt) {
        emitNode(if_stmt->else_stmt.get());
    }

    const size_t jmp_if_after_to_addr = current_elements.size();
    current_elements[jmp_if_after_addr] = Element{ElementType::Instruction, InstructionType::Jmp, Operand{OperandType::InstructionAddress, true, new size_t{jmp_if_after_to_addr}}};

    for (auto& addr : jmp_else_ifs_after_addrs) {
        current_elements[addr] = Element{ElementType::Instruction, InstructionType::Jmp, Operand{OperandType::InstructionAddress, true, new size_t{jmp_if_after_to_addr}}};
    }
}

IL::Operand IL::Emitter::emitExpr(const Ast::Expression* expression) {
    switch (expression->type) {
        case Ast::ExpressionType::Number: return Operand{OperandType::Number, false, &dynamic_cast<const Ast::Number*>(expression)->number};
        case Ast::ExpressionType::Binary: return emitBinaryExpr(dynamic_cast<const Ast::BinaryExpr*>(expression));
        case Ast::ExpressionType::Identifier: return emitIdentifierExpr(dynamic_cast<const Ast::Identifier*>(expression));
        case Ast::ExpressionType::String: return Operand{OperandType::String, false, &dynamic_cast<const Ast::String*>(expression)->string};
        case Ast::ExpressionType::Boolean: return Operand{OperandType::Boolean, false, &dynamic_cast<const Ast::Boolean*>(expression)->boolean};
        case Ast::ExpressionType::Array: return emitArrayExpr(dynamic_cast<const Ast::Array*>(expression));
        case Ast::ExpressionType::Object: return emitObjectExpr(dynamic_cast<const Ast::Object*>(expression));
        case Ast::ExpressionType::IndexAccess: return emitIndexAccessExpr(dynamic_cast<const Ast::IndexAccess*>(expression));
        case Ast::ExpressionType::MemberAccess: return emitMemberAccessExpr(dynamic_cast<const Ast::MemberAccess*>(expression));
        default: throw std::runtime_error("unknown expression type at IL::Emitter::emitExpr");
    }
}
IL::Operand IL::Emitter::emitBinaryExpr(const Ast::BinaryExpr* binary_expr) {
    Operand left = emitExpr(binary_expr->left.get());
    Operand right = emitExpr(binary_expr->right.get());

    if (right.type != OperandType::Register) {
        current_elements.push_back(Element{ElementType::Instruction, InstructionType::Mov, Operand{OperandType::Register, false, &Registers[static_cast<int>(Register::Accumulator)]}});
        current_elements.push_back(Element{ElementType::Operand, InstructionType::None, std::move(right)});
    }

    if (const auto itr = operator_to_instruction_type.find(binary_expr->_operator); itr != operator_to_instruction_type.end()) {
        current_elements.push_back(Element{ElementType::Instruction, itr->second, std::move(left)});
        return Operand{OperandType::Register, false, &Registers[static_cast<int>(Register::Accumulator)]};
    }
    throw std::runtime_error("Unknown Operator at IL.cpp, IL::Emitter::emitBinaryExpr");
}
IL::Operand IL::Emitter::emitIdentifierExpr(const Ast::Identifier* identifier) {
    if (const auto itr = var_name_to_ptr.find(identifier->identifier); itr != var_name_to_ptr.end()) {
        return Operand{OperandType::MemoryAddress, true, new size_t{evaluateMemAddr(itr->second)}};
    }
    throw std::runtime_error("Unknown Identifier at IL.cpp, IL::Emitter::emitIdentifierExpr");
}
IL::Operand IL::Emitter::emitArrayExpr(const Ast::Array* array) {
    auto* il_array = new std::vector<Operand>();
    il_array->resize(array->values.size());
    for (size_t i = 0; i < array->values.size(); i++) {
        Operand _operand = emitExpr(array->values[i].get());

        (*il_array)[i].type = _operand.type;
        (*il_array)[i].should_free = _operand.should_free;
        (*il_array)[i].value = _operand.value;

        _operand.should_free = false;
    }

    return Operand{OperandType::Array, true, il_array};
}
IL::Operand IL::Emitter::emitObjectExpr(const Ast::Object* object) {
    auto* object_map = new std::unordered_map<std::string_view, Operand>();

    for (const auto&[str, expr] : object->values) {
        object_map->insert({str, emitExpr(expr.get())});
    }

    return Operand{OperandType::Object, true, object_map};
}
IL::Operand IL::Emitter::emitIndexAccessExpr(const Ast::IndexAccess* index_access) { return {};}
IL::Operand IL::Emitter::emitMemberAccessExpr(const Ast::MemberAccess* member_access) { return {};}

size_t IL::Emitter::evaluateMemAddr(const size_t addr) const {
    return addr - stack_frames_ptrs.back();
}
