import re
import sys

def renumber_markdown_headers(input_file, output_file=None):
    """
    处理Markdown文档，重新排序四级标题的序号
    
    Args:
        input_file (str): 输入Markdown文件路径
        output_file (str, optional): 输出Markdown文件路径，默认为None（覆盖原文件）
    """
    # 如果未指定输出文件，则覆盖原文件
    if output_file is None:
        output_file = input_file
    
    # 读取文件内容
    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"读取文件时出错: {e}")
        return
    
    # 定义匹配四级标题的正则表达式
    header_pattern = re.compile(r'^(### )(\d+)(\. \*\*)(.*?)(\*\*)(.*)$')
    
    # 计数器，用于新的序号
    counter = 1
    new_lines = []
    
    # 逐行处理
    for line in lines:
        match = header_pattern.match(line)
        if match:
            # 如果是四级标题行，替换序号
            prefix, number, dot_star, title, suffix, rest = match.groups()
            new_line = f"{prefix}{counter}{dot_star}{title}{suffix}{rest}\n"
            counter += 1
            new_lines.append(new_line)
        else:
            # 如果不是四级标题行，保持不变
            new_lines.append(line)
    
    # 写入文件
    try:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.writelines(new_lines)
        print(f"处理完成，已将结果保存至 {output_file}")
    except Exception as e:
        print(f"写入文件时出错: {e}")

if __name__ == "__main__":
    # 检查命令行参数
    if len(sys.argv) < 2:
        print("使用方法: python script.py 输入文件 [输出文件]")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else sys.argv[1]
    
    renumber_markdown_headers(input_file, output_file)

