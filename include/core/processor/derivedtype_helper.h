#ifndef _DERIVEDTYPE_HELPER_H_
#define _DERIVEDTYPE_HELPER_H_

#include "model/db/type.h"
#include "util/logger/macros.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Type.h>

using namespace clang;

// Determine whether a type is derived
bool isDerivedType(const Type *type) {
  if (!type)
    return false;

  return type->isPointerType() || type->isReferenceType() ||
         type->isArrayType() || type->isVectorType() ||
         type->isFunctionPointerType() || type->isBlockPointerType() ||
         isa<AttributedType>(type) || type->isSpecifierType();
}

// Get the kind of derived type
int getDerivedTypeKind(const Type *type) {
  // 指针类型
  if (type->isPointerType())
    return static_cast<int>(DerivedTypeKind::POINTER);

  // 引用类型
  if (const auto *refType = dyn_cast<ReferenceType>(type)) {
    if (isa<LValueReferenceType>(refType))
      return static_cast<int>(DerivedTypeKind::REFERENCE);
    else if (isa<RValueReferenceType>(refType))
      return static_cast<int>(DerivedTypeKind::RVALUE_REFERENCE);
  }

  // 数组类型
  if (type->isArrayType())
    return static_cast<int>(DerivedTypeKind::ARRAY);

  // 向量类型
  if (type->isVectorType())
    return static_cast<int>(DerivedTypeKind::GNU_VECTOR);

  // 函数指针类型
  if (type->isFunctionPointerType())
    return static_cast<int>(DerivedTypeKind::ROUTINEPTR);

  // 函数引用类型
  if (const auto *refType = dyn_cast<ReferenceType>(type))
    if (refType->getPointeeType()->isFunctionType())
      return static_cast<int>(DerivedTypeKind::ROUTINEREFERENCE);

  // Block类型
  if (type->isBlockPointerType())
    return static_cast<int>(DerivedTypeKind::BLOCK);

  // 带有类型修饰符的类型
  if (isa<AttributedType>(type) || type->isSpecifierType())
    return static_cast<int>(DerivedTypeKind::TYPE_WITH_SPECIFIERS);

  return -1;
}

QualType getUnderlyingType(const Type *type) {
  // 指针类型
  if (const auto *ptrType = dyn_cast<PointerType>(type))
    return ptrType->getPointeeType();

  // 引用类型
  if (const auto *refType = dyn_cast<ReferenceType>(type))
    return refType->getPointeeType();

  // 数组类型
  if (const auto *arrayType = dyn_cast<ArrayType>(type))
    return arrayType->getElementType();

  // 向量类型
  if (const auto *vectorType = dyn_cast<VectorType>(type))
    return vectorType->getElementType();

  // Block类型
  if (const auto *blockType = dyn_cast<BlockPointerType>(type))
    return blockType->getPointeeType();

  // 带属性的类型
  if (const auto *attrType = dyn_cast<AttributedType>(type))
    return attrType->getModifiedType();

  // 处理带有限定符的类型（const、volatile等）
  QualType qualType(type, 0);
  if (qualType.hasLocalQualifiers())
    return qualType.getUnqualifiedType();

  // 处理类型别名和糖化类型
  if (const auto *typedefType = dyn_cast<TypedefType>(type))
    return typedefType->desugar();

  if (const auto *elaboratedType = dyn_cast<ElaboratedType>(type))
    return elaboratedType->getNamedType();

  // 其他情况返回原类型
  return QualType(type, 0);
}

std::string getDerivedTypeName(const Type *type, ASTContext &context) {
  QualType qualType(type, 0);

  PrintingPolicy policy = context.getPrintingPolicy();
  policy.SuppressTagKeyword = true;
  policy.SuppressScope = false;

  return qualType.getAsString(policy);
}

#endif // _DERIVEDTYPE_HELPER_H_
