# 本文件是用户实际使用后的反馈

## View层

- 将“序号”（指代index）更名为“索引”，减少歧义
- list tasks 和 list resources 时在序号后，名称前，展示其ID
- show task 时最好在依赖前展示其序号、ID、名称，依赖后展示其资源列表。这样更符合show task的指令命名
- 仿照“[FAIL] 未知命令：……”，validate 和 schedule 报错时先[FAIL]再汇报错误

## Service层

- 导出项目时在不同内容板块之间加一个换行
- 导出项目时 Allocation unitCost 实为浮点型，整数也应显示“.0”
