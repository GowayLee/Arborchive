#include "core/processor/derivedtype_helper.h"
#include "core/processor/type_processor.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/type.h"
#include "util/id_generator.h"
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
  INSERT_TYPE_CACHE(keyType, _typeId);
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
    // typeModel.associate_id =
    //     processPtrToMemberType(cast<MemberPointerType>(type));
  } else if (type->isDecltypeType()) {
    // @decltype
    typeModel.type = static_cast<int>(TypeType::DECL_TYPE);
    // typeModel.associate_id = processDeclType(cast<DecltypeType>(type));
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
  int typeId = -1;
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    int typeId = *cachedId;
  else {
    // TODO: Dependency Manager...
  }
  DbModel::DerivedType derivedTypeModel = {GENID(DerivedType),
                                           getDerivedTypeName(TP, ast_context),
                                           getDerivedTypeKind(TP), typeId};
  STG.insertClassObj(derivedTypeModel);
  return derivedTypeModel.id;
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
  return userTypeModel.id;
}

int TypeProcessor::processRoutineType(const Type *TP, ASTContext &ast_context) {
  // 获取函数类型的具体信息
  const FunctionType *FT = TP->getAs<FunctionType>();
  if (!FT)
    return -1;

  QualType returnType = FT->getReturnType();
  KeyType routineTypeKey = KeyGen::Type::makeKey(returnType, ast_context);
  int routineTypeId = -1;
  if (auto cachedId = SEARCH_TYPE_CACHE(routineTypeKey))
    routineTypeId = *cachedId;
  else {
    // TODO: Dependency Manager...
  }
  DbModel::RoutineType routineTypeModel = {GENID(RoutineType), routineTypeId};
  STG.insertClassObj(routineTypeModel);

  // 如果是FunctionProtoType，处理参数
  if (const FunctionProtoType *FPT = dyn_cast<FunctionProtoType>(FT))
    for (unsigned index = 0; index < FPT->getNumParams(); ++index) {
      QualType paramType = FPT->getParamType(index);
      KeyType routineArgTypeKey = KeyGen::Type::makeKey(paramType, ast_context);
      int routineArgTypeId = -1;
      if (auto cachedId = SEARCH_TYPE_CACHE(routineArgTypeKey))
        routineArgTypeId = *cachedId;
      else {
        // TODO: Dependency Manager...
      }
      DbModel::RoutineTypeArg routineTypeArgModel = {
          routineTypeModel.id, static_cast<int>(index), routineArgTypeId};
      STG.insertClassObj(routineTypeArgModel);
    }
  return routineTypeModel.id;
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
