#pragma once
#include <vector>
#include "../Parser/Ast.h"

namespace IL {
    // When an address is referred to as 0 in memory it's basically the first address
    // in the current stack frame not the whole memory

    enum class InstructionType {
        None,
        Mov, Add, Sub, Mul, Div, Mod, Push, Pop, PushFrame, PopFrame, Set, Jmp, JmpIf, JmpIfNot,
        BT, // Bigger Than
        LT, // Lower Than
        EQ, // Equals
        NEQ, // Not Equals
        BEQ, // Bigger Than Or Equal
        LEQ, // Lower Than Or Equal
        SEQ, // Strict Equals
        SNEQ, // Strict Not Equals
    };

    enum class Register {
        Accumulator = 0, // The register used for calculations
        Return = 1, // The function return register
        Result = 2, // The register used for the result for everything, like memory fetches, calculations, etc
    };

    inline static Register Registers[] = {
        Register::Accumulator, Register::Return, Register::Result
    };

    enum class ElementType {
        Instruction, Operand
    };

    enum class OperandType {
        Number, String, Identifier, Boolean, Array, Object, Function, Register, MemoryAddress, InstructionAddress
    };

    struct Operand {
        OperandType type = OperandType::Number;
        bool should_free = false;
        const void* value = nullptr;

        Operand() = default;
        Operand(const Operand& other) = delete;
        Operand(OperandType _type, bool _should_free, const void* _value): type(_type),
            should_free(_should_free), value(_value) {

        };
        Operand& operator=(const Operand& other) = delete;

        Operand(Operand&& other) noexcept {
            type = other.type;
            should_free = other.should_free;
            value = other.value;

            other.type = OperandType::Number;
            other.should_free = false;
            other.value = nullptr;
        }

        ~Operand() {
            if (!should_free) return;
            switch (type) {
                case OperandType::MemoryAddress: delete static_cast<const size_t *>(value); break;
                case OperandType::Object: delete static_cast<const std::unordered_map<std::string_view, Operand> *>(value); break;
                case OperandType::Array: delete static_cast<const std::vector<Operand> *>(value); break;
                default: break;
            }
        }

        Operand& operator=(Operand&& other) noexcept {
            type = other.type;
            should_free = other.should_free;
            value = other.value;

            other.type = OperandType::Number;
            other.should_free = false;
            other.value = nullptr;

            return *this;
        }
    };
    struct Element {
        ElementType type;
        InstructionType instruction;
        Operand operand;

        Element() : type(ElementType::Instruction), instruction(InstructionType::Mov), operand() {}

        // A custom constructor makes push_back/emplace_back much cleaner
        Element(ElementType t, InstructionType inst)
            : type(t), instruction(inst), operand() {}

        Element(ElementType t, InstructionType inst, Operand&& op)
            : type(t), instruction(inst), operand(std::move(op)) {}

        // Declare explicit move constructor so std::vector can move it
        Element(Element&& other) noexcept
            : type(other.type), instruction(other.instruction), operand(std::move(other.operand)) {}

        Element& operator=(Element&& other) noexcept {
            if (this != &other) {
                type = other.type;
                instruction = other.instruction;
                operand = std::move(other.operand);
            }
            return *this;
        }

        // Delete copy operations entirely since Operand cannot be copied
        Element(const Element&) = delete;
        Element& operator=(const Element&) = delete;
    };

    struct Program {
        std::vector<Element> elements;
    };

    struct ArrayOperandInfo {
        size_t size;
        Operand* array;
    };



    class Emitter {
    public:
        static std::unordered_map<std::string_view, InstructionType> operator_to_instruction_type;
        size_t stack_ptr = 0;
        std::vector<size_t> stack_frames_ptrs;
        std::unordered_map<std::string_view, size_t> var_name_to_ptr;

        std::vector<Element> current_elements;
        Program emitProgram(const Ast::Program* ast);

        void emitNode(const Ast::Statement* statement);
        void emitVarDecl(const Ast::VarDecl* var_decl);
        void emitVarReInit(const Ast::VarReInit* var_reinit);
        void emitIf(const Ast::If* if_stmt);

        Operand emitExpr(const Ast::Expression* expression); // the expression result will be in the result register
        Operand emitBinaryExpr(const Ast::BinaryExpr* binary_expr);
        Operand emitIdentifierExpr(const Ast::Identifier* identifier);
        Operand emitArrayExpr(const Ast::Array* array);
        Operand emitObjectExpr(const Ast::Object* object);
        Operand emitIndexAccessExpr(const Ast::IndexAccess* index_access);
        Operand emitMemberAccessExpr(const Ast::MemberAccess* member_access);

        size_t evaluateMemAddr(size_t addr) const;
    };
}
