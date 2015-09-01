// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_INTERPRETER_BYTECODE_ARRAY_BUILDER_H_
#define V8_INTERPRETER_BYTECODE_ARRAY_BUILDER_H_

#include <vector>

#include "src/ast.h"
#include "src/frames.h"
#include "src/identity-map.h"
#include "src/interpreter/bytecodes.h"
#include "src/zone.h"
#include "src/zone-containers.h"

namespace v8 {
namespace internal {

class Isolate;

namespace interpreter {

class Register;

class BytecodeArrayBuilder {
 public:
  BytecodeArrayBuilder(Isolate* isolate, Zone* zone);
  Handle<BytecodeArray> ToBytecodeArray();

  // Set number of parameters expected by function.
  void set_parameter_count(int number_of_params);
  int parameter_count() const;

  // Set number of locals required for bytecode array.
  void set_locals_count(int number_of_locals);
  int locals_count() const;

  Register Parameter(int parameter_index);

  // Constant loads to accumulator.
  BytecodeArrayBuilder& LoadLiteral(v8::internal::Smi* value);
  BytecodeArrayBuilder& LoadLiteral(Handle<Object> object);
  BytecodeArrayBuilder& LoadUndefined();
  BytecodeArrayBuilder& LoadNull();
  BytecodeArrayBuilder& LoadTheHole();
  BytecodeArrayBuilder& LoadTrue();
  BytecodeArrayBuilder& LoadFalse();

  // Register-accumulator transfers.
  BytecodeArrayBuilder& LoadAccumulatorWithRegister(Register reg);
  BytecodeArrayBuilder& StoreAccumulatorInRegister(Register reg);

  // Operators.
  BytecodeArrayBuilder& BinaryOperation(Token::Value binop, Register reg);

  // Flow Control.
  BytecodeArrayBuilder& Return();

 private:
  static const int kLastParamRegisterIndex =
      -InterpreterFrameConstants::kLastParamFromRegisterPointer / kPointerSize;

  static Bytecode BytecodeForBinaryOperation(Token::Value op);

  void Output(Bytecode bytecode, uint8_t r0, uint8_t r1, uint8_t r2);
  void Output(Bytecode bytecode, uint8_t r0, uint8_t r1);
  void Output(Bytecode bytecode, uint8_t r0);
  void Output(Bytecode bytecode);

  bool OperandIsValid(Bytecode bytecode, int operand_index,
                      uint8_t operand_value) const;

  size_t GetConstantPoolEntry(Handle<Object> object);

  int BorrowTemporaryRegister();
  void ReturnTemporaryRegister(int reg_index);

  Isolate* isolate_;
  ZoneVector<uint8_t> bytecodes_;
  bool bytecode_generated_;

  IdentityMap<size_t> constants_map_;
  ZoneVector<Handle<Object>> constants_;

  int parameter_count_;
  int local_register_count_;
  int temporary_register_count_;
  int temporary_register_next_;

  friend class TemporaryRegisterScope;
  DISALLOW_IMPLICIT_CONSTRUCTORS(BytecodeArrayBuilder);
};

// An interpreter register which is located in the function's register file
// in its stack-frame.
class Register {
 public:
  static const int kMaxRegisterIndex = 128;
  static const int kMinRegisterIndex = -127;

  explicit Register(int index) : index_(index) {
    DCHECK_LE(index_, kMaxRegisterIndex);
    DCHECK_GE(index_, kMinRegisterIndex);
  }

  int index() { return index_; }

  uint8_t ToOperand() { return static_cast<uint8_t>(-index_); }
  static Register FromOperand(uint8_t operand) {
    return Register(-static_cast<int8_t>(operand));
  }

 private:
  void* operator new(size_t size);
  void operator delete(void* p);

  int index_;
};

// A stack-allocated class than allows the instantiator to allocate
// temporary registers that are cleaned up when scope is closed.
class TemporaryRegisterScope {
 public:
  explicit TemporaryRegisterScope(BytecodeArrayBuilder* builder);
  ~TemporaryRegisterScope();
  Register NewRegister();

 private:
  void* operator new(size_t size);
  void operator delete(void* p);

  BytecodeArrayBuilder* builder_;
  int count_;
  int last_register_index_;

  DISALLOW_COPY_AND_ASSIGN(TemporaryRegisterScope);
};


}  // namespace interpreter
}  // namespace internal
}  // namespace v8

#endif  // V8_INTERPRETER_BYTECODE_ARRAY_BUILDER_H_
