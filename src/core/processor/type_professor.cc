#include "core/processor/derivedtype_helper.h"
#include "core/processor/type_processor.h"
#include "core/processor/usertype_helper.h"
#include "core/srcloc_recorder.h"
#include "db/dependency_manager.h"
#include "db/storage_facade.h"
#include "model/db/type.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/type.h"
#include "util/logger/macros.h"
#include <clang/AST/Type.h>
#include <clang/Basic/Specifiers.h>
#include <iostream>

int getBuiltinTypeSign(const clang::BuiltinType *builtinType);

void TypeProcessor::routerProcess(const TypeDecl *TD) {
  auto T = TD->getTypeForDecl();
  if (!T) { // FIXME:
    LOG_WARNING << "TypeDecl is null" << std::endl;
    return;
  }

  QualType qualType = T->getCanonicalTypeInternal();
  const clang::Type *type = qualType.getTypePtr();

  LocIdPair *locIdPair =
      SrcLocRecorder::processDefault(TD, &TD->getASTContext());

  DbModel::Type typeModel;
  _typeId = GENID(Type);
  typeModel.id = _typeId;
  // Insert Cache
  KeyType keyType = KeyGen::Type::makeKey(TD, TD->getASTContext());
  INSERT_TYPE_CACHE(keyType, _typeId); // FIXME: testing is needed
  // Classify type
  if (type->isBuiltinType()) { // FIXME: Miss track on `slight-case.cc`
    // @builtintype
    typeModel.type = static_cast<int>(TypeType::BUILTIN_TYPE);
    typeModel.associate_id =
        processBuiltinType(cast<BuiltinType>(type), TD->getASTContext());
  } else if (isDerivedType(type)) {
    // @derivedtype
    typeModel.type = static_cast<int>(TypeType::DERIVED_TYPE);
    typeModel.associate_id = processDerivedType(type, TD->getASTContext());
  } else if (type->isRecordType() || type->isEnumeralType()) {
    // @usertype
    typeModel.type = static_cast<int>(TypeType::USER_TYPE);
    typeModel.associate_id = processUserType(type, TD->getASTContext());
  } else if (type->isFunctionType() || type->isFunctionProtoType()) {
    // @routinetype
    typeModel.type = static_cast<int>(TypeType::ROUTINE_TYPE);
    typeModel.associate_id = processRoutineType(type, TD->getASTContext());
  } else if (type->isMemberPointerType()) {
    // @ptrtomember
    typeModel.type = static_cast<int>(TypeType::PTR_TO_MEMBER);
    typeModel.associate_id = processPtrToMemberType(
        cast<MemberPointerType>(type), TD->getASTContext());
  } else if (type->isDecltypeType()) {
    // @decltype
    typeModel.type = static_cast<int>(TypeType::DECL_TYPE);
    typeModel.associate_id =
        processDeclType(cast<DecltypeType>(type), TD->getASTContext());
  } else {
    LOG_WARNING << "Unknown type classification" << std::endl;
    // 不匹配已知类型分类的情况
    typeModel.type = -1; // 未知类型
    typeModel.associate_id = -1;
  }

  DbModel::TypeDecl typeDecl = {_typeDeclId = GENID(TypeDecl), _typeId,
                                locIdPair->spec_id};

  // Determine type_def
  recordTypeDef(TD);

  // Determine top type declaration
  recordTopTypeDecl(TD);
  STG.insertClassObj(typeModel);
  STG.insertClassObj(typeDecl);
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
                                      ASTContext &ast_context) {
  // 获取类型名称
  PrintingPolicy pp(ast_context.getLangOpts());
  pp.SuppressTagKeyword = true;
  pp.SuppressScope = false;

  // 获取类型大小和对齐
  clang::QualType qualType(BT, 0);
  int size = ast_context.getTypeSize(qualType) / ast_context.getCharWidth();
  int alignment =
      ast_context.getTypeAlign(qualType) / ast_context.getCharWidth();

  // 创建并返回类型ID
  DbModel::BuiltinType_ builtinTypeModel = {GENID(BuiltinType_),
                                            BT->getNameAsCString(pp),
                                            static_cast<int>(BT->getKind()),
                                            size,
                                            alignment,
                                            getBuiltinTypeSign(BT)};
  STG.insertClassObj(builtinTypeModel);
  return builtinTypeModel.id;
}

int TypeProcessor::processDerivedType(const Type *TP, ASTContext &ast_context) {
  QualType QT = getUnderlyingType(TP);
  KeyType typeKey = KeyGen::Type::makeKey(QT, ast_context);
  LOG_DEBUG << "DerivedType TypeKey: " << typeKey << std::endl;

  int derivedTypeId = GENID(DerivedType);
  std::string derivedTypeName = getDerivedTypeName(TP, ast_context);
  int derivedTypeKind = getDerivedTypeKind(TP);

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
  return derivedTypeId;
}

int TypeProcessor::processUserType(const Type *TP, ASTContext &ast_context) {
  // Get typedecl
  const TypeDecl *TD = TP->getAsTagDecl();
  if (!TD) {
    // 对于typedef和using alias的情况，需要特殊处理
    if (const TypedefType *TT = dyn_cast<TypedefType>(TP))
      TD = TT->getDecl();
    else if (const auto *ET = dyn_cast<ElaboratedType>(TP))
      // 处理可能的elaborated type
      return processUserType(ET->getNamedType().getTypePtr(), ast_context);
  }
  if (!TD)
    return -1;

  // Get typename
  std::string typeName = TD->getNameAsString();
  LOG_DEBUG << "Processing user type: " << typeName << std::endl;

  // Specify type kind
  int kind = static_cast<int>(
      UserTypeKind::UNKNOWN_USERTYPE); // unknown_usertype by default
  if (const RecordDecl *RD = dyn_cast<RecordDecl>(TD)) {
    if (const CXXRecordDecl *CXXRD = dyn_cast<CXXRecordDecl>(RD)) {
      if (CXXRD->isClass())
        kind =
            CXXRD->getTemplateSpecializationKind() == TSK_Undeclared
                ? static_cast<int>(UserTypeKind::CLASS)
                : static_cast<int>(
                      UserTypeKind::TEMPLATE_CLASS); // class or template_class
      else if (CXXRD->isStruct())
        kind = CXXRD->getTemplateSpecializationKind() == TSK_Undeclared
                   ? static_cast<int>(UserTypeKind::STRUCT)
                   : static_cast<int>(
                         UserTypeKind::TEMPLATE_STRUCT); // struct or
                                                         // template_struct
      else if (CXXRD->isUnion())
        kind =
            CXXRD->getTemplateSpecializationKind() == TSK_Undeclared
                ? static_cast<int>(UserTypeKind::UNION)
                : static_cast<int>(
                      UserTypeKind::TEMPLATE_UNION); // union or template_union
    } else {
      if (RD->isStruct())
        kind = static_cast<int>(UserTypeKind::STRUCT); // struct
      else if (RD->isUnion())
        kind = static_cast<int>(UserTypeKind::UNION); // union
    }
  } else if (const EnumDecl *ED = dyn_cast<EnumDecl>(TD))
    kind = ED->isScoped()
               ? static_cast<int>(UserTypeKind::SCOPED_ENUM)
               : static_cast<int>(UserTypeKind::ENUM); // scoped_enum or enum
  else if (const TypedefNameDecl *TND = dyn_cast<TypedefNameDecl>(TD)) {
    if (isa<TypeAliasDecl>(TND))
      kind = static_cast<int>(UserTypeKind::SCOPED_ENUM); // using_alias
    else
      kind = static_cast<int>(UserTypeKind::TYPEDEF); // typedef
  } else if (const TemplateTypeParmDecl *TTPD =
                 dyn_cast<TemplateTypeParmDecl>(TD))
    kind = static_cast<int>(
        UserTypeKind::TEMPLATE_PARAMETER); // template_parameter

  DbModel::UserType userTypeModel = {GENID(UserType), typeName, kind};
  KeyType userTypeKey = KeyGen::Type::makeKey(TD, ast_context);
  INSERT_USERTYPE_CACHE(userTypeKey, userTypeModel.id);
  STG.insertClassObj(userTypeModel);

  // Process more detail about user_type
  record_is_pod_class(TP, ast_context, userTypeModel.id);
  record_is_standard_layout_class(TP, ast_context, userTypeModel.id);
  record_is_complete(TP, ast_context, userTypeModel.id);

  return userTypeModel.id;
}

int TypeProcessor::processRoutineType(const Type *TP, ASTContext &ast_context) {
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
                                          ASTContext &ast_context) {
  int ptrToMemberId = GENID(PtrToMember);
  QualType pointeeType = MPT->getPointeeType();
  const Type *classType = MPT->getClass();

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
                                   ASTContext &ast_context) {
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
