import re
import argparse

def count_tables(file_path):
    # 定义用于匹配两种表格格式的正则表达式
    pattern1 = r'^[a-zA-Z_]+\s*\('  # 匹配形式1: 字符串(可以带下划线) + (
    pattern2 = r'^@[a-zA-Z_]+'      # 匹配形式2: @ + 字符串(可以带下划线)
    
    tables = []
    in_comment = False
    
    with open(file_path, 'r', encoding='utf-8') as file:
        for line in file:
            line = line.strip()
            
            # 检查是否进入注释区域
            if "/*" in line:
                in_comment = True
            
            # 检查是否退出注释区域
            if "*/" in line:
                in_comment = False
                continue
            
            # 如果在注释区域内，跳过该行
            if in_comment:
                continue
            
            # 匹配形式1
            match1 = re.match(pattern1, line)
            if match1:
                table_name = line.split('(')[0].strip()
                tables.append(table_name)
                continue
            
            # 匹配形式2
            match2 = re.match(pattern2, line)
            if match2 and '=' in line:
                table_name = line.split('=')[0].strip()
                tables.append(table_name)
    
    return tables

def main():
    # 设置命令行参数解析
    parser = argparse.ArgumentParser(description="统计dbscheme文档中的数据表数量")
    parser.add_argument("file_path", help="dbscheme文档的路径")
    parser.add_argument("--list", "-l", action="store_true", help="显示所有找到的表名")
    args = parser.parse_args()
    
    # 调用函数统计表
    tables = count_tables(args.file_path)
    
    print(f"找到的数据表总数: {len(tables)}")
    
    # 如果指定了--list参数，则显示所有表名
    if args.list:
        for i, table in enumerate(tables, 1):
            print(f"{i}. {table}")

if __name__ == "__main__":
    main()
