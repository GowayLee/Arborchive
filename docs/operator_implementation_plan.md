### C语言核心运算符解析实现计划

本文档定义了第一阶段需要解析和存入数据库的C语言核心运算符。所有数据表名和层级关系均遵循 `docs/op-list` 文档的规范。

#### 总览：顶层运算符 (`@op_expr`)

在初步实现中，`@op_expr` 将包含以下四类核心表达式：

```
@op_expr = @un_op_expr
         | @bin_op_expr
         | @assign_expr
         | @conditionalexpr
         ;
```

---

#### 1. 一元运算符 (`@un_op_expr`)

`@un_op_expr` 表将聚合所有一元操作。

```
@un_op_expr = @address_of         // & (取地址)
            | @indirect           // * (解引用)
            | @un_arith_op_expr   // 一元算术运算
            | @un_bitwise_op_expr // 一元位运算
            | @un_log_op_expr     // 一元逻辑运算
            ;
```

**1.1. 一元算术运算 (`@un_arith_op_expr`)**
```
@un_arith_op_expr = @arithnegexpr     // - (负号)
                  | @crement_expr     // ++, -- (自增/自减)
                  ;
```
*   **1.1.1. 自增/自减运算 (`@crement_expr`)**
    ```
    @crement_expr = @increment_expr | @decrement_expr;
    ```
*   **1.1.2. 自增 (`@increment_expr`)**
    ```
    @increment_expr = @preincrexpr | @postincrexpr;
    ```
*   **1.1.3. 自减 (`@decrement_expr`)**
    ```
    @decrement_expr = @predecrexpr | @postdecrexpr;
    ```

**1.2. 一元位运算 (`@un_bitwise_op_expr`)**
```
@un_bitwise_op_expr = @complementexpr;  // ~ (按位取反)
```

**1.3. 一元逻辑运算 (`@un_log_op_expr`)**
```
@un_log_op_expr = @notexpr;           // ! (逻辑非)
```

---

#### 2. 二元运算符 (`@bin_op_expr`)

`@bin_op_expr` 表将聚合所有二元操作。

```
@bin_op_expr = @bin_arith_op_expr   // 二元算术运算
             | @bin_bitwise_op_expr // 二元位运算
             | @cmp_op_expr         // 比较运算
             | @bin_log_op_expr     // 二元逻辑运算
             ;
```

**2.1. 二元算术运算 (`@bin_arith_op_expr`)**
```
@bin_arith_op_expr = @addexpr           // + (加)
                   | @subexpr           // - (减)
                   | @mulexpr           // * (乘)
                   | @divexpr           // / (除)
                   | @remexpr           // % (取模)
                   | @p_arith_op_expr   // 指针算术
                   ;
```
*   **2.1.1. 指针算术 (`@p_arith_op_expr`)**
    ```
    @p_arith_op_expr = @paddexpr         // 指针+整数
                     | @psubexpr         // 指针-整数
                     | @pdiffexpr        // 指针-指针
                     ;
    ```

**2.2. 二元位运算 (`@bin_bitwise_op_expr`)**
```
@bin_bitwise_op_expr = @lshiftexpr      // << (左移)
                     | @rshiftexpr      // >> (右移)
                     | @andexpr         // & (按位与)
                     | @orexpr          // | (按位或)
                     | @xorexpr         // ^ (按位异或)
                     ;
```

**2.3. 比较运算 (`@cmp_op_expr`)**
```
@cmp_op_expr = @eq_op_expr | @rel_op_expr;
```
*   **2.3.1. 相等比较 (`@eq_op_expr`)**
    ```
    @eq_op_expr = @eqexpr | @neexpr;    // ==, !=
    ```
*   **2.3.2. 关系比较 (`@rel_op_expr`)**
    ```
    @rel_op_expr = @gtexpr             // >
                 | @ltexpr             // <
                 | @geexpr             // >=
                 | @leexpr             // <=
                 ;
    ```

**2.4. 二元逻辑运算 (`@bin_log_op_expr`)**
```
@bin_log_op_expr = @andlogicalexpr | @orlogicalexpr; // &&, ||
```

---

#### 3. 赋值运算符 (`@assign_expr`)

`@assign_expr` 表聚合所有赋值操作。

```
@assign_expr = @assignexpr | @assign_op_expr;
```
*   `@assignexpr` 对应基础赋值 `=`。

**3.1. 复合赋值 (`@assign_op_expr`)**
```
@assign_op_expr = @assign_arith_expr    // 算术复合赋值
                | @assign_bitwise_expr  // 位运算复合赋值
                ;
```
*   **3.1.1. 算术复合赋值 (`@assign_arith_expr`)**
    ```
    @assign_arith_expr = @assignaddexpr     // +=
                       | @assignsubexpr     // -=
                       | @assignmulexpr     // *=
                       | @assigndivexpr     // /=
                       | @assignremexpr     // %=
                       ;
    ```
*   **3.1.2. 位运算复合赋值 (`@assign_bitwise_expr`)**
    ```
    @assign_bitwise_expr = @assignandexpr      // &=
                         | @assignorexpr      // |=
                         | @assignxorexpr     // ^=
                         | @assignlshiftexpr  // <<=
                         | @assignrshiftexpr  // >>=
                         ;
    ```

---

#### 4. 条件运算符 (`@conditionalexpr`)

此运算符 (`? :`) 作为独立的顶级运算符处理，直接归入 `@op_expr`。

---

#### 补充说明

*   **`sizeof` 运算符**: `sizeof` 是C语言的关键部分。在Clang中，它通常被解析为 `UnaryExprOrTypeTraitExpr`。在实现过程中，需要确保此类表达式能够被正确识别和处理，并将其相关信息存入数据库，即使它没有在`op-list`中被赋予独立的 `@` 标签。

请按照此计划进行开发。
