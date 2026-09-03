#ifndef _TABLE_INIT_H_
#define _TABLE_INIT_H_

#include "db/table_defs/class.h"
#include "db/table_defs/variable.h"
#include "table_defs/attribute.h"
#include "table_defs/comment.h"
#include "table_defs/compilation.h"
#include "table_defs/concept.h"
#include "table_defs/container.h"
#include "table_defs/declaration.h"
#include "table_defs/element.h"
#include "table_defs/expr.h"
#include "table_defs/function.h"
#include "table_defs/initialization.h"
#include "table_defs/lambda.h"
#include "table_defs/location.h"
#include "table_defs/preprocessor.h"
#include "table_defs/specifiers.h"
#include "table_defs/stmt.h"
#include "table_defs/type.h"

using namespace sqlite_orm;

// Definition of function used to delcare all the table existed
inline auto initStorage(const std::string &path) {
  // clang-format off
  return make_storage(
      path,
      // Compilation Tables
      CompTableFn::compilations(),
      CompTableFn::compilatio_args(),
      CompTableFn::compilatio_build_mode(),
      CompTableFn::compilation_compiling_files(),
      CompTableFn::compilatio_time(),
      CompTableFn::compilation_finished(),
      CompTableFn::extractor_version(),
      // Comment Tables
      CommentTableFn::comments(),
      CommentTableFn::commentbinding(),
      // Location Tables
      LocTableFn::locations(),
      LocTableFn::locations_default(),
      LocTableFn::locations_stmt(),
      LocTableFn::locations_expr(),
      // Container Tables
      ContainerTableFn::container(),
      ContainerTableFn::files(),
      ContainerTableFn::folders(),
      ContainerTableFn::namespaces(),
      ContainerTableFn::namespace_inline(),
      ContainerTableFn::namespacembrs(),
      // Declaration Tables
      DeclTableFn::declarations(),
      DeclTableFn::frienddecls(),
      DeclTableFn::namespace_decls(),
      DeclTableFn::usings(),
      DeclTableFn::using_container(),
      // Function Tables
      FuncTableFn::functions(),
      FuncTableFn::fun_decls(),
      FuncTableFn::fun_def(),
      FuncTableFn::function_return_type(),
      FuncTableFn::pure_functions(),
      FuncTableFn::function_deleted(),
      FuncTableFn::function_defaulted(),
      FuncTableFn::function_prototyped(),
      FuncTableFn::fun_specialized(),
      FuncTableFn::fun_implicit(),
      FuncTableFn::is_function_template(),
      FuncTableFn::function_instantiation(),
      FuncTableFn::function_template_argument(),
      FuncTableFn::function_template_argument_value(),
      FuncTableFn::function_entry_point(),
      FuncTableFn::deduction_guide_for_class(),
      FuncTableFn::fun_decl_throws(),
      FuncTableFn::fun_decl_empty_throws(),
      FuncTableFn::fun_decl_noexcept(),
      FuncTableFn::fun_decl_empty_noexcept(),
      FuncTableFn::fun_decl_typedef_type(),
      FuncTableFn::coroutine(),
      FuncTableFn::coroutine_new(),
      FuncTableFn::coroutine_delete(),
      // Variable Tables
      // TO BE REMOVED: Variable and LocalScopeVar tables - intermediary tables no longer needed
      VarTableFn::variable(), // deprecated - intermediary table no longer needed
      // VarTableFn::localscopevariable(),
      VarTableFn::localvariables(),
      VarTableFn::params(),
      VarTableFn::globalvariables(),
      VarTableFn::membervariables(),
      VarTableFn::var_decls(),
      VarTableFn::var_def(),
      VarTableFn::var_specialized(),
      VarTableFn::is_variable_template(),
      VarTableFn::variable_instantiation(),
      VarTableFn::variable_template_argument(),
      VarTableFn::variable_template_argument_value(),
      VarTableFn::var_decl_specifiers(),
      VarTableFn::is_structured_binding(),
      VarTableFn::var_requires(),
      VarTableFn::fieldoffsets(),
      VarTableFn::bitfield(),
      VarTableFn::arbor_field_layout_traits(),
      VarTableFn::arbor_indirect_field_paths(),
      // Type Tables
      TypeTableFn::types(), // deprecated - intermediary table no longer needed
      TypeTableFn::type_decls(),
      TypeTableFn::type_def(),
      TypeTableFn::type_decl_top(),
      TypeTableFn::builtintypes(),
      TypeTableFn::derivedtypes(),
      TypeTableFn::usertypes(),
      TypeTableFn::routinetypes(),
      TypeTableFn::routinetypeargs(),
      TypeTableFn::ptrtomembers(),
      TypeTableFn::decltypes(),
      TypeTableFn::is_pod_class(),
      TypeTableFn::is_standard_layout_class(),
      TypeTableFn::is_complete(),
      TypeTableFn::is_class_template(),
      TypeTableFn::class_instantiation(),
      TypeTableFn::class_template_argument(),
      TypeTableFn::class_template_argument_value(),
      TypeTableFn::template_template_instantiation(),
      TypeTableFn::template_template_argument(),
      TypeTableFn::type_template_type_constraint(),
      TypeTableFn::nontype_template_parameters(),
      TypeTableFn::derivations(),
      TypeTableFn::derspecifiers(),
      TypeTableFn::direct_base_offsets(),
      TypeTableFn::virtual_base_offsets(),
      TypeTableFn::arbor_layout_provenance(),
      TypeTableFn::arbor_record_layout_traits(),
      TypeTableFn::arbor_direct_base_layout_traits(),
      // Concept Tables
      ConceptTableFn::concept_templates(),
      ConceptTableFn::concept_instantiation(),
      ConceptTableFn::is_type_constraint(),
      ConceptTableFn::concept_template_argument(),
      ConceptTableFn::concept_template_argument_value(),
      TypeTableFn::enumconstants(),
      TypeTableFn::typedefbase(),
      TypeTableFn::arraysizes(),
      TypeTableFn::pointerishsize(),
      // Stmt Tables
      StmtTableFn::stmts(),
      StmtTableFn::if_initialization(),
      StmtTableFn::if_then(),
      StmtTableFn::if_else(),
      StmtTableFn::constexpr_if_initialization(),
      StmtTableFn::constexpr_if_then(),
      StmtTableFn::constexpr_if_else(),
      StmtTableFn::consteval_if_then(),
      StmtTableFn::consteval_if_else(),
      // StmtTableFn::stmt_for_or_range_based_for(),
      StmtTableFn::for_initialization(),
      StmtTableFn::for_condition(),
      StmtTableFn::for_update(),
      StmtTableFn::for_body(),
      StmtTableFn::while_body(),
      StmtTableFn::do_body(),
      StmtTableFn::switch_body(),
      StmtTableFn::switch_initialization(),
      StmtTableFn::switch_case(),
      // Expr Tables;
      ExprTableFn::exprs(),
      ExprTableFn::exprparents(),
      ExprTableFn::exprconv(),
      ExprTableFn::expr_types(),
      ExprTableFn::expr_isload(),
      ExprTableFn::compgenerated(),
      ExprTableFn::conversionkinds(),
      ExprTableFn::expr_allocator(),
      ExprTableFn::expr_deallocator(),
      ExprTableFn::new_allocated_type(),
      ExprTableFn::new_array_allocated_type(),
      ExprTableFn::funbind(),
      ExprTableFn::iscall(),
      ExprTableFn::varbind(),
      ExprTableFn::values(),
      ExprTableFn::valuetext(),
      ExprTableFn::valuebind(),
      ExprTableFn::aggregatearrayinit(),
      ExprTableFn::aggregatefieldinit(),
      ExprTableFn::sizeofbind(),
      // Initialization Tables
      InitializationTableFn::initialisers(),
      InitializationTableFn::braced_initialisers(),
      // Lambda Tables
      LambdaTableFn::lambdas(),
      LambdaTableFn::lambda_capture(),
      // Element Tables;
      ElementTableFn::parameterized_element(),
      // Class Table;
      ClassTableFn::memeber(),
      // Specifier Tables
      SpecifierTableFn::specifiers(),
      SpecifierTableFn::typespecifiers(),
      SpecifierTableFn::funspecifiers(),
      SpecifierTableFn::varspecifiers(),
      // Attribute Tables
      AttributeTableFn::attributes(),
      AttributeTableFn::attribute_args(),
      AttributeTableFn::attribute_arg_value(),
      AttributeTableFn::attribute_arg_type(),
      AttributeTableFn::attribute_arg_constant(),
      AttributeTableFn::attribute_arg_expr(),
      AttributeTableFn::attribute_arg_name(),
      AttributeTableFn::typeattributes(),
      AttributeTableFn::funcattributes(),
      AttributeTableFn::varattributes(),
      AttributeTableFn::stmtattributes(),
      // Preprocessor Tables
      PreprocessorTableFn::preprocdirects(),
      PreprocessorTableFn::preprocpair(),
      PreprocessorTableFn::preproctrue(),
      PreprocessorTableFn::preprocfalse(),
      PreprocessorTableFn::preproctext(),
      PreprocessorTableFn::includes(),
      PreprocessorTableFn::macroinvocations(),
      PreprocessorTableFn::macroparent(),
      PreprocessorTableFn::macrolocationbind(),
      PreprocessorTableFn::macro_argument_unexpanded(),
      PreprocessorTableFn::macro_argument_expanded()
    );
  // clang-format on
}

#endif // _TABLE_INIT_H_
