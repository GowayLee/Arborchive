#include "core/processor/type_processor.h"
#include "core/processor/derivedtype_helper.h"
#include "core/processor/usertype_helper.h"
#include "core/srcloc_recorder.h"
#include "db/dependency_manager.h"
#include "db/storage_facade.h"
#include "model/db/type.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/type.h"
#include "util/logger/macros.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/Basic/Specifiers.h>
#include <iostream>

int getBuiltinTypeSign(const clang::BuiltinType *builtinType);
BuiltinTypeKind GetBuiltinTypeKind(const clang::BuiltinType *BT);

int TypeProcessor::processType(const Type *T) {
  if (!T) {
    LOG_WARNING << "Type is null" << std::endl;
    return -1;
  }

  const QualType qualType = T->getCanonicalTypeInternal();
  const clang::Type *type = qualType.getTypePtr();

  // Directly return specific type IDs instead of creating Type intermediary
  if (const auto derived_result = analyzeDerivedType(type)) {
    _typeId = processDerivedType(type, derived_result, ast_context_);
  } else if (type->isEnumeralType() || type->isTypedefNameType()) {
    _typeId = processUserType(type, ast_context_);
  } else if (type->isFunctionType() || type->isFunctionProtoType()) {
    _typeId = processRoutineType(type, ast_context_);
  } else if (type->isMemberPointerType()) {
    _typeId =
        processPtrToMemberType(cast<MemberPointerType>(type), ast_context_);
  } else if (type->isDecltypeType()) {
    _typeId = processDeclType(cast<DecltypeType>(type), ast_context_);
  } else {
    LOG_WARNING << "Unknown type classification" << std::endl;
    _typeId = -1;
  }

  return _typeId;
}

void TypeProcessor::processRecordDecl(const RecordDecl *RD) {
  auto T = RD->getTypeForDecl();
  if (T) {
    _typeId = processType(T);
    processTypeDecl(RD);
  }
}

void TypeProcessor::processEnumDecl(const EnumDecl *ED) {
  auto T = ED->getTypeForDecl();
  if (T) {
    _typeId = processType(T);
    processTypeDecl(ED);
  }
}

void TypeProcessor::processTypedefDecl(const TypedefDecl *TND) {
  std::string typedefName = TND->getNameAsString();
  LOG_INFO << "Processing TypedefDecl: " << typedefName << std::endl;

  // Create a UserType entry for the typedef
  int typedefKind = static_cast<int>(UserTypeKind::TYPEDEF);
  DbModel::UserType userTypeModel = {GENID(UserType), typedefName, typedefKind};
  KeyType userTypeKey = KeyGen::Type::makeKey(TND, ast_context_);
  INSERT_TYPE_CACHE(userTypeKey, userTypeModel.id);
  STG.insertClassObj(userTypeModel);

  // Process the underlying type and create typedef base mapping
  auto T = TND->getTypeForDecl();
  if (T) {
    int underlyingTypeId = processType(T);
    // Create typedef base mapping
    DbModel::TypedefBase typedefBase = {userTypeModel.id, underlyingTypeId};
    STG.insertClassObj(typedefBase);
    LOG_DEBUG << "Created typedef base: " << typedefName << " -> type_id: " << underlyingTypeId << std::endl;

    _typeId = processType(T);
    processTypeDecl(TND);
  }
}

void TypeProcessor::processTemplateTypeParmDecl(
    const TemplateTypeParmDecl *TTPD) {
  auto T = TTPD->getTypeForDecl();
  if (T) {
    _typeId = processType(T);
    processTypeDecl(TTPD);
  }
}

void TypeProcessor::processTypeDecl(const TypeDecl *TD) {
  LocIdPair *locIdPair = SrcLocRecorder::processDefault(TD, ast_context_);

  DbModel::TypeDecl typeDecl = {_typeDeclId = GENID(TypeDecl), _typeId,
                                locIdPair->spec_id};

  // Determine type_def
  recordTypeDef(TD);

  // Determine top type declaration
  recordTopTypeDecl(TD);
  STG.insertClassObj(typeDecl);
}

void TypeProcessor::processRecordType(const RecordType *RT) {
  // Extract the RecordDecl from the RecordType
  const RecordDecl *RD = RT->getDecl();
  if (!RD)
    return;

  // Get typename
  std::string typeName = RD->getNameAsString();
  if (typeName.empty()) {
    // Handle anonymous types
    typeName = "<anonymous>";
  }

  // Determine the UserTypeKind
  int kind = static_cast<int>(UserTypeKind::UNKNOWN_USERTYPE);
  if (const CXXRecordDecl *CXXRD = dyn_cast<CXXRecordDecl>(RD)) {
    if (CXXRD->isClass())
      kind = CXXRD->getTemplateSpecializationKind() == TSK_Undeclared
                 ? static_cast<int>(UserTypeKind::CLASS)
                 : static_cast<int>(UserTypeKind::TEMPLATE_CLASS);
    else if (CXXRD->isStruct())
      kind = CXXRD->getTemplateSpecializationKind() == TSK_Undeclared
                 ? static_cast<int>(UserTypeKind::STRUCT)
                 : static_cast<int>(UserTypeKind::TEMPLATE_STRUCT);
    else if (CXXRD->isUnion())
      kind = CXXRD->getTemplateSpecializationKind() == TSK_Undeclared
                 ? static_cast<int>(UserTypeKind::UNION)
                 : static_cast<int>(UserTypeKind::TEMPLATE_UNION);
  } else {
    if (RD->isStruct())
      kind = static_cast<int>(UserTypeKind::STRUCT);
    else if (RD->isUnion())
      kind = static_cast<int>(UserTypeKind::UNION);
  }

  LOG_DEBUG << "Processing RecordType user type: " << typeName
            << ", Kind: " << kind << std::endl;

  // Create UserType model
  DbModel::UserType userTypeModel = {GENID(UserType), typeName, kind};
  KeyType userTypeKey = KeyGen::Type::makeKey(RD, ast_context_);
  LOG_DEBUG << "RecordType UserType Key: " << userTypeKey << std::endl;

  // Check cache first
  if (auto cachedId = SEARCH_TYPE_CACHE(userTypeKey))
    return;

  INSERT_TYPE_CACHE(userTypeKey, userTypeModel.id);
  STG.insertClassObj(userTypeModel);

  // Process additional type details (similar to processUserType)
  record_is_pod_class(RT, userTypeModel.id);
  record_is_standard_layout_class(RT, userTypeModel.id);
  record_is_complete(RT, userTypeModel.id);
}

void TypeProcessor::recordTypeDef(const TypeDecl *TD) {
  if (const auto *TND = dyn_cast<TypedefNameDecl>(TD)) {
    DbModel::TypeDef typeDefModel = {_typeDeclId};
    STG.insertClassObj(typeDefModel);
  }
}

void TypeProcessor::recordTopTypeDecl(const TypeDecl *TD) {
  // 顶级类型声明是指直接出现在命名空间或全局作用域中的声明
  // 检查其父声明是否是翻译单元或命名空间
  const clang::DeclContext *parentContext = TD->getDeclContext();
  if (parentContext->isTranslationUnit() ||
      isa<clang::NamespaceDecl>(parentContext)) {
    DbModel::TypeDeclTop typeDeclTop = {_typeDeclId};
    STG.insertClassObj(typeDeclTop);
  }
}

int TypeProcessor::processBuiltinType(const BuiltinType *BT,
                                      ASTContext *ast_context) {
  // 获取类型名称
  // PrintingPolicy pp(ast_context.getLangOpts());

  KeyType typeKey = KeyGen::Type::makeKey(QualType(BT, 0), ast_context);
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    return *cachedId;

  // 获取类型大小和对齐
  clang::QualType qualType(BT, 0);
  int size = ast_context->getTypeSize(qualType) / ast_context->getCharWidth();
  int alignment =
      ast_context->getTypeAlign(qualType) / ast_context->getCharWidth();

  // 创建并返回类型ID
  DbModel::BuiltinType_ builtinTypeModel = {
      GENID(BuiltinType_),
      BT->getNameAsCString(pp_),
      static_cast<int>(GetBuiltinTypeKind(BT)),
      size,
      getBuiltinTypeSign(BT),
      alignment};
  INSERT_TYPE_CACHE(typeKey, builtinTypeModel.id);
  STG.insertClassObj(builtinTypeModel);
  return builtinTypeModel.id;
}

int TypeProcessor::processDerivedType(
    const Type *T,
    const std::optional<std::pair<DerivedTypeKind, QualType>> derived_result,
    ASTContext *ast_context) {
  QualType derivedType(T, 0);
  KeyType derivedTypeKey = KeyGen::Type::makeKey(derivedType, ast_context);
  if (auto cachedId = SEARCH_TYPE_CACHE(derivedTypeKey))
    return *cachedId;

  const int derivedTypeKind = (int)derived_result->first;
  const QualType baseType = derived_result->second;

  KeyType typeKey = KeyGen::Type::makeKey(baseType, ast_context);
  LOG_DEBUG << "DerivedType TypeKey: " << derivedTypeKey << std::endl;

  int derivedTypeId = GENID(DerivedType);
  INSERT_TYPE_CACHE(derivedTypeKey, derivedTypeId);
  std::string derivedTypeName = derivedType.getAsString(pp_);

  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey)) {
    DbModel::DerivedType derivedTypeModel = {derivedTypeId, derivedTypeName,
                                             derivedTypeKind, *cachedId};
    STG.insertClassObj(derivedTypeModel);
  } else {
    DbModel::DerivedType derivedTypeModel = {derivedTypeId, derivedTypeName,
                                             derivedTypeKind, -1};
    STG.insertClassObj(derivedTypeModel);
    PendingUpdate update{
        typeKey, CacheType::TYPE,
        [derivedTypeId, derivedTypeName, derivedTypeKind](int resolvedId) {
          DbModel::DerivedType updated_record = {derivedTypeId, derivedTypeName,
                                                 derivedTypeKind, resolvedId};
          STG.insertClassObj(updated_record);
        }};
    DependencyManager::instance().addDependency(update);
  }

  // Process array sizes for array types
  if (derived_result->first == DerivedTypeKind::ARRAY) {
    const ArrayType *AT = cast<ArrayType>(T);
    processArraySizes(AT, derivedTypeId);
  }
  // Process pointer sizes for pointer/reference types
  else if (derived_result->first == DerivedTypeKind::POINTER ||
           derived_result->first == DerivedTypeKind::REFERENCE ||
           derived_result->first == DerivedTypeKind::RVALUE_REFERENCE) {
    processPointerishSize(T, derivedTypeId);
  }

  return derivedTypeId;
}

int TypeProcessor::processUserType(const Type *TP, ASTContext *ast_context) {
  // Check for typedef types FIRST before getting tag decl
  // This is important because a typedef of an enum should be processed as a typedef
  const TypedefNameDecl *TND = nullptr;
  const TypeDecl *TD = nullptr;

  if (const TypedefType *TT = dyn_cast<TypedefType>(TP)) {
    TND = TT->getDecl();
    TD = TND;
  } else if (const auto *ET = dyn_cast<ElaboratedType>(TP)) {
    // 处理可能的elaborated type
    return processUserType(ET->getNamedType().getTypePtr(), ast_context);
  } else {
    TD = TP->getAsTagDecl();
  }

  if (!TD) {
    return -1;
  }

  // Get typename
  std::string typeName = TD->getNameAsString();

  // Specify type kind
  int kind = static_cast<int>(
      UserTypeKind::UNKNOWN_USERTYPE); // unknown_usertype by default
  LOG_DEBUG << "Processing user type: " << typeName << ", Kind: " << kind
            << std::endl;
  if (const EnumDecl *ED = dyn_cast<EnumDecl>(TD))
    kind = ED->isScoped()
               ? static_cast<int>(UserTypeKind::SCOPED_ENUM)
               : static_cast<int>(UserTypeKind::ENUM); // scoped_enum or enum
  else if (TND) {
    if (isa<TypeAliasDecl>(TND))
      kind = static_cast<int>(UserTypeKind::USING_ALIAS); // using_alias
    else
      kind = static_cast<int>(UserTypeKind::TYPEDEF); // typedef
  } else if (const TemplateTypeParmDecl *TTPD =
                 dyn_cast<TemplateTypeParmDecl>(TD))
    kind = static_cast<int>(
        UserTypeKind::TEMPLATE_PARAMETER); // template_parameter

  DbModel::UserType userTypeModel = {GENID(UserType), typeName, kind};
  KeyType userTypeKey = KeyGen::Type::makeKey(TD, ast_context);
  LOG_DEBUG << "UserType Key: " << userTypeKey << std::endl;
  INSERT_TYPE_CACHE(userTypeKey, userTypeModel.id);
  STG.insertClassObj(userTypeModel);

  // Process more detail about user_type
  record_is_pod_class(TP, userTypeModel.id);
  record_is_standard_layout_class(TP, userTypeModel.id);
  record_is_complete(TP, userTypeModel.id);

  // Process enum constants (only if NOT a typedef)
  if (!TND) {
    if (const EnumDecl *ED = dyn_cast<EnumDecl>(TD)) {
      LOG_DEBUG << "Processing enum constants for: " << userTypeModel.name << std::endl;
      processEnumConstants(ED, userTypeModel.id);
    }
  }
  // Process typedef base type
  if (TND) {
    LOG_DEBUG << "Processing typedef base for: " << userTypeModel.name << std::endl;
    processTypedefBase(TND, userTypeModel.id);
  }

  return userTypeModel.id;
}

void TypeProcessor::processEnumConstants(const EnumDecl *ED, int parentEnumId) {
  LOG_DEBUG << "processEnumConstants: Processing enum with parent ID: " << parentEnumId << std::endl;
  int index = 0;
  for (const EnumConstantDecl *ECD : ED->enumerators()) {
    LOG_DEBUG << "processEnumConstants: Processing constant: " << ECD->getNameAsString()
              << " at index " << index << std::endl;

    // Record location
    LocIdPair *locIdPair = SrcLocRecorder::processDefault(ECD, ast_context_);

    // Create enum constant record
    // Note: type_id is set to parentEnumId since enum constants have the enum type
    GENID(EnumConstant);
    DbModel::EnumConstant enumConstant = {
        IDGenerator::getLastGeneratedId<DbModel::EnumConstant>(),
        parentEnumId,
        index,
        parentEnumId,  // Enum constants have the same type as their parent enum
        ECD->getNameAsString(),
        locIdPair->spec_id};

    STG.insertClassObj(enumConstant);
    LOG_DEBUG << "processEnumConstants: Inserted enum constant with ID: "
              << enumConstant.id << std::endl;

    index++;
  }
}

void TypeProcessor::processTypedefBase(const TypedefNameDecl *TND, int typedefId) {
  // Get the underlying type
  QualType underlyingType = TND->getUnderlyingType();
  int type_id = processType(underlyingType.getTypePtr());

  // Create typedef base mapping
  DbModel::TypedefBase typedefBase = {typedefId, type_id};
  STG.insertClassObj(typedefBase);
}

void TypeProcessor::processArraySizes(const ArrayType *AT, int derivedTypeId) {
  // Get number of elements (0 for incomplete arrays)
  int num_elements = 0;
  if (const ConstantArrayType *CAT = dyn_cast<ConstantArrayType>(AT)) {
    num_elements = CAT->getZExtSize();
  }

  // Calculate byte size and alignment
  int bytesize = 0;
  int alignment = 0;

  if (ast_context_) {
    bytesize = ast_context_->getTypeSize(AT) / ast_context_->getCharWidth();
    alignment = ast_context_->getTypeAlign(AT) / ast_context_->getCharWidth();
  }

  // Create array sizes record
  DbModel::ArraySizes arraySizes = {derivedTypeId, num_elements, bytesize,
                                    alignment};
  STG.insertClassObj(arraySizes);
}

void TypeProcessor::processPointerishSize(const Type *T, int derivedTypeId) {
  // Get pointer size and alignment from AST context
  int size = 0;
  int alignment = 0;

  if (ast_context_) {
    QualType qualType = T->getCanonicalTypeInternal();
    size = ast_context_->getTypeSize(qualType) / ast_context_->getCharWidth();
    alignment =
        ast_context_->getTypeAlign(qualType) / ast_context_->getCharWidth();
  }

  // Create pointerish size record
  DbModel::PointerishSize pointerishSize = {derivedTypeId, size, alignment};
  STG.insertClassObj(pointerishSize);
}

int TypeProcessor::processRoutineType(const Type *TP, ASTContext *ast_context) {
  const FunctionType *FT = TP->getAs<FunctionType>();
  if (!FT)
    return -1;

  int routineTypeId = GENID(RoutineType);
  QualType returnType = FT->getReturnType();
  KeyType returnTypeKey = KeyGen::Type::makeKey(returnType, ast_context);

  if (auto cachedId = SEARCH_TYPE_CACHE(returnTypeKey)) {
    DbModel::RoutineType routineTypeModel = {routineTypeId, *cachedId};
    STG.insertClassObj(routineTypeModel);
  } else {
    DbModel::RoutineType routineTypeModel = {routineTypeId, -1};
    STG.insertClassObj(routineTypeModel);
    PendingUpdate update{
        returnTypeKey, CacheType::TYPE, [routineTypeId](int resolvedId) {
          DbModel::RoutineType updated_record = {routineTypeId, resolvedId};
          STG.insertClassObj(updated_record);
        }};
    DependencyManager::instance().addDependency(update);
  }

  if (const FunctionProtoType *FPT = dyn_cast<FunctionProtoType>(FT)) {
    for (unsigned index = 0; index < FPT->getNumParams(); ++index) {
      QualType paramType = FPT->getParamType(index);
      KeyType paramTypeKey = KeyGen::Type::makeKey(paramType, ast_context);
      if (auto cachedId = SEARCH_TYPE_CACHE(paramTypeKey)) {
        DbModel::RoutineTypeArg routineTypeArgModel = {
            routineTypeId, static_cast<int>(index), *cachedId};
        STG.insertClassObj(routineTypeArgModel);
      } else {
        DbModel::RoutineTypeArg routineTypeArgModel = {
            routineTypeId, static_cast<int>(index), -1};
        STG.insertClassObj(routineTypeArgModel);
        PendingUpdate update{paramTypeKey, CacheType::TYPE,
                             [routineTypeId, index](int resolvedId) {
                               DbModel::RoutineTypeArg updated_record = {
                                   routineTypeId, static_cast<int>(index),
                                   resolvedId};
                               STG.insertClassObj(updated_record);
                             }};
        DependencyManager::instance().addDependency(update);
      }
    }
  }
  return routineTypeId;
}

int TypeProcessor::processPtrToMemberType(const MemberPointerType *MPT,
                                          ASTContext *ast_context) {
  int ptrToMemberId = GENID(PtrToMember);
  QualType pointeeType = MPT->getPointeeType();
  const Type *classType = MPT->getAs<Type>();

  KeyType pointeeTypeKey = KeyGen::Type::makeKey(pointeeType, ast_context);
  KeyType classTypeKey =
      KeyGen::Type::makeKey(classType->getCanonicalTypeInternal(), ast_context);

  auto pointeeIdOpt = SEARCH_TYPE_CACHE(pointeeTypeKey);
  auto classIdOpt = SEARCH_TYPE_CACHE(classTypeKey);

  int pointeeTypeId = pointeeIdOpt.value_or(-1);
  int classTypeId = classIdOpt.value_or(-1);

  DbModel::PtrToMember ptrToMemberModel = {ptrToMemberId, pointeeTypeId,
                                           classTypeId};
  STG.insertClassObj(ptrToMemberModel);

  if (!pointeeIdOpt) {
    PendingUpdate update{pointeeTypeKey, CacheType::TYPE,
                         [ptrToMemberId, classTypeId](int resolvedId) {
                           DbModel::PtrToMember updated_record = {
                               ptrToMemberId, resolvedId, classTypeId};
                           STG.insertClassObj(updated_record);
                         }};
    DependencyManager::instance().addDependency(update);
  }

  if (!classIdOpt) {
    PendingUpdate update{classTypeKey, CacheType::TYPE,
                         [ptrToMemberId, pointeeTypeId](int resolvedId) {
                           DbModel::PtrToMember updated_record = {
                               ptrToMemberId, pointeeTypeId, resolvedId};
                           STG.insertClassObj(updated_record);
                         }};
    DependencyManager::instance().addDependency(update);
  }

  return ptrToMemberId;
}

int TypeProcessor::processDeclType(const DecltypeType *DT,
                                   ASTContext *ast_context) {
  int declTypeId = GENID(DeclType);
  const Expr *expr = DT->getUnderlyingExpr();
  QualType baseType = DT->getUnderlyingType();
  bool parenthesesWouldChange = DT->isReferenceType();

  int exprId = -1;
  if (expr) {
    KeyType exprKey = KeyGen::Expr_::makeKey(expr, ast_context);
    if (auto cachedId = SEARCH_EXPR_CACHE(exprKey)) {
      exprId = *cachedId;
    }
  }

  KeyType typeKey = KeyGen::Type::makeKey(baseType, ast_context);
  auto typeIdOpt = SEARCH_TYPE_CACHE(typeKey);
  int typeId = typeIdOpt.value_or(-1);

  DbModel::DeclType declTypeModel = {declTypeId, exprId, typeId,
                                     parenthesesWouldChange};
  STG.insertClassObj(declTypeModel);

  if (expr && exprId == -1) {
    KeyType exprKey = KeyGen::Expr_::makeKey(expr, ast_context);
    PendingUpdate update{
        exprKey, CacheType::EXPR,
        [declTypeId, typeId, parenthesesWouldChange](int resolvedId) {
          DbModel::DeclType updated_record = {declTypeId, resolvedId, typeId,
                                              parenthesesWouldChange};
          STG.insertClassObj(updated_record);
        }};
    DependencyManager::instance().addDependency(update);
  }

  if (!typeIdOpt) {
    PendingUpdate update{
        typeKey, CacheType::TYPE,
        [declTypeId, exprId, parenthesesWouldChange](int resolvedId) {
          DbModel::DeclType updated_record = {declTypeId, exprId, resolvedId,
                                              parenthesesWouldChange};
          STG.insertClassObj(updated_record);
        }};
    DependencyManager::instance().addDependency(update);
  }

  return declTypeId;
}

int getBuiltinTypeSign(const clang::BuiltinType *builtinType) {
  switch (builtinType->getKind()) {
  // 有符号类型
  case BuiltinType::Char_S:
  case BuiltinType::SChar:
  case BuiltinType::Short:
  case BuiltinType::Int:
  case BuiltinType::Long:
  case BuiltinType::LongLong:
  case BuiltinType::Float:
  case BuiltinType::Double:
  case BuiltinType::LongDouble:
  case BuiltinType::Float128:
    return 1;

  // 无符号类型
  case BuiltinType::Char_U:
  case BuiltinType::UChar:
  case BuiltinType::UShort:
  case BuiltinType::UInt:
  case BuiltinType::ULong:
  case BuiltinType::ULongLong:
    return 0;

  // 不适用(如void,bool等)
  default:
    return -1;
  }
}

BuiltinTypeKind GetBuiltinTypeKind(const clang::BuiltinType *BT) {
  switch (BT->getKind()) {
  case clang::BuiltinType::Kind::Void:
    return BuiltinTypeKind::VOID;
  case clang::BuiltinType::Kind::Bool:
    return BuiltinTypeKind::BOOLEAN;
  case clang::BuiltinType::Kind::Char_S:
  case clang::BuiltinType::Kind::Char_U:
    return BuiltinTypeKind::CHAR;
  case clang::BuiltinType::Kind::UChar:
    return BuiltinTypeKind::UNSIGNED_CHAR;
  case clang::BuiltinType::Kind::SChar:
    return BuiltinTypeKind::SIGNED_CHAR;
  case clang::BuiltinType::Kind::Short:
    return BuiltinTypeKind::SHORT;
  case clang::BuiltinType::Kind::UShort:
    return BuiltinTypeKind::UNSIGNED_SHORT;
  case clang::BuiltinType::Kind::Int:
    return BuiltinTypeKind::INT;
  case clang::BuiltinType::Kind::UInt:
    return BuiltinTypeKind::UNSIGNED_INT;
  case clang::BuiltinType::Kind::Long:
    return BuiltinTypeKind::LONG;
  case clang::BuiltinType::Kind::ULong:
    return BuiltinTypeKind::UNSIGNED_LONG;
  case clang::BuiltinType::Kind::LongLong:
    return BuiltinTypeKind::LONG_LONG;
  case clang::BuiltinType::Kind::ULongLong:
    return BuiltinTypeKind::UNSIGNED_LONG_LONG;
  case clang::BuiltinType::Kind::Float:
    return BuiltinTypeKind::FLOAT;
  case clang::BuiltinType::Kind::Double:
    return BuiltinTypeKind::DOUBLE;
  case clang::BuiltinType::Kind::LongDouble:
    return BuiltinTypeKind::LONG_DOUBLE;
  case clang::BuiltinType::Kind::WChar_S:
  case clang::BuiltinType::Kind::WChar_U:
    return BuiltinTypeKind::WCHAR_T;
  case clang::BuiltinType::Kind::NullPtr:
    return BuiltinTypeKind::DECLTYPE_NULLPTR;
  case clang::BuiltinType::Kind::Int128:
    return BuiltinTypeKind::INT128;
  case clang::BuiltinType::Kind::UInt128:
    return BuiltinTypeKind::UNSIGNED_INT128;
  case clang::BuiltinType::Kind::Float128:
    return BuiltinTypeKind::FLOAT128;
  case clang::BuiltinType::Kind::Char16:
    return BuiltinTypeKind::CHAR16_T;
  case clang::BuiltinType::Kind::Char32:
    return BuiltinTypeKind::CHAR32_T;
  case clang::BuiltinType::Kind::Char8:
    return BuiltinTypeKind::CHAR8_T;
  case clang::BuiltinType::Kind::Float16:
    return BuiltinTypeKind::FLOAT16;
  case clang::BuiltinType::Kind::Half:
    return BuiltinTypeKind::FP16;
  case clang::BuiltinType::Kind::BFloat16:
    return BuiltinTypeKind::STD_BFLOAT16;
  default:
    return BuiltinTypeKind::UNKOWNTYPE;
  }
}
