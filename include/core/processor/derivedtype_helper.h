#ifndef _DERIVEDTYPE_HELPER_H_
#define _DERIVEDTYPE_HELPER_H_

#include "model/db/type.h"
#include "util/logger/macros.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Type.h>

using namespace clang;

std::optional<std::pair<DerivedTypeKind, QualType>>
analyzeDerivedType(const Type *T) {
  if (!T)
    return std::nullopt;

  // 1. Pointer Type
  if (const auto *PT = llvm::dyn_cast<PointerType>(T)) {
    QualType pointeeType = PT->getPointeeType();

    // 6. 判断是否为函数指针 (Routine Pointer)
    if (pointeeType->isFunctionType())
      return std::make_pair(DerivedTypeKind::ROUTINEPTR, pointeeType);

    // 普通指针
    return std::make_pair(DerivedTypeKind::POINTER, pointeeType);
  }

  // 2. LValue Reference Type
  if (const auto *LRT = llvm::dyn_cast<LValueReferenceType>(T)) {
    QualType pointeeType = LRT->getPointeeType();

    // 7. 判断是否为函数引用 (Routine Reference)
    if (pointeeType->isFunctionType())
      return std::make_pair(DerivedTypeKind::ROUTINEREFERENCE, pointeeType);

    // 普通引用
    return std::make_pair(DerivedTypeKind::REFERENCE, pointeeType);
  }

  // 8. RValue Reference Type (C++11)
  if (const auto *RRT = llvm::dyn_cast<RValueReferenceType>(T))
    return std::make_pair(DerivedTypeKind::RVALUE_REFERENCE,
                          RRT->getPointeeType());

  // 4. Array Type
  if (const auto *AT = llvm::dyn_cast<ArrayType>(T))
    return std::make_pair(DerivedTypeKind::ARRAY, AT->getElementType());

  // 5. GNU Vector Type
  if (const auto *VT = llvm::dyn_cast<VectorType>(T))
    return std::make_pair(DerivedTypeKind::GNU_VECTOR, VT->getElementType());

  // 10. Block Type (Objective-C blocks)
  if (const auto *BT = llvm::dyn_cast<BlockPointerType>(T))
    return std::make_pair(DerivedTypeKind::BLOCK, BT->getPointeeType());

  // 3. Type with Specifiers
  // 检查是否有限定符 (const, volatile, restrict)
  QualType QT(T, 0);
  if (QT.hasLocalQualifiers())
    return std::make_pair(DerivedTypeKind::TYPE_WITH_SPECIFIERS,
                          QT.getLocalUnqualifiedType());

  // 检查 AttributedType (带属性的类型，如 __attribute__)
  if (const auto *AT = llvm::dyn_cast<AttributedType>(T))
    return std::make_pair(DerivedTypeKind::TYPE_WITH_SPECIFIERS,
                          AT->getModifiedType());

  // 不是派生类型
  return std::nullopt;
}

#endif // _DERIVEDTYPE_HELPER_H_
