#include "core/processor/derivedtype_helper.h"
#include "core/processor/type_processor.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/type.h"
#include "util/id_generator.h"
#include "util/key_generator/type.h"
#include "util/logger/macros.h"
#include <clang/AST/Type.h>
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
  if (type->isBuiltinType()) {
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
    // typeModel.associate_id = processUserType(type);
  } else if (type->isFunctionType() || type->isFunctionProtoType()) {
    // @routinetype
    typeModel.type = static_cast<int>(TypeType::ROUTINE_TYPE);
    // typeModel.associate_id = processRoutineType(type);
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
