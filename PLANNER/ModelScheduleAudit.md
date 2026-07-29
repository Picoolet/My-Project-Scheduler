# ModelSchedule 设计审查报告（ModelScheduleAudit）

## 1. 总体评价

当前 ModelSchedule 设计整体具有较好的面向对象设计意识：

- 使用 TaskId / ResourceId 强类型 ID，避免不同实体 ID 混用；
- 使用组合（Strategy Pattern）代替 BasicTask / MilestoneTask 继承体系，解决 Task 类型动态转换问题；
- 使用 ID 引用代替裸指针，降低对象生命周期耦合；
- 使用 Project 作为聚合根管理 Task、Dependency、Resource 和 Allocation；
- 使用 ScheduleResult 独立保存 CPM 计算结果，避免污染 Project 模型；
- 使用 Editor 作为业务操作层，将界面操作与领域模型隔离。

整体架构方向正确。

但目前设计仍存在以下需要明确或优化的问题：

1. ID 类型设计不完善；
2. Task策略创建职责划分不合理；
3. Project、Editor、Task之间的权限边界需要重新定义；
4. 数据一致性维护责任需要明确；
5. Dependency索引和唯一性约束需要完善；
6. ScheduleResult构建方式需要统一；
7. 错误处理和验证策略缺失。

---

## 2. TaskId / ResourceId设计问题

## 2.1 问题

当前设计：

```cpp
struct TaskId
{
    unsigned int value;
};
````

虽然避免了裸整数的问题，但仍存在：

- 外部可以随意修改value；
- 缺少比较操作；
- 无法直接用于unordered_map；
- 缺少无效ID表示。

---

## 2.2 修改方案

将ID设计为值对象（Value Object）：

```cpp
class TaskId
{
private:
    unsigned int value_;

public:
    TaskId(unsigned int value);

    unsigned int value() const;

    bool operator==(const TaskId&) const;

    static TaskId Invalid();
};
```

ResourceId同理。

需要补充：

- operator==
- operator!=
- std::hash特化
- Invalid ID

---

## 2.3 不推荐方案

不建议设计：

```text
Id
 |
 +-- TaskId
 |
 +-- ResourceId
```

原因：

继承表达"is-a"关系。

TaskId和ResourceId只是共享实现，不共享语义。

推荐：

- 独立class；
- 或StrongId模板。

---

## 3. Task设计问题

## 3.1 Task纯净性的理解

Task应保持领域纯净，但：

纯净 ≠ 没有行为。

Task可以拥有：

- name
- duration
- canAllocateResource()
- setDuration()

因为这些属于任务自身状态。

不应该包含：

- CPM计算；
- 文件解析；
- DAG验证；
- UI逻辑。

---

## 3.2 策略模式创建职责问题

当前：

```cpp
Task::setDuration()
{
    if(duration==0)
        policy_=MilestonePolicy;

    else
        policy_=NormalPolicy;
}
```

问题：

Task知道所有策略类型。

违反开闭原则：

未来新增：

- SummaryTaskPolicy
- ExternalTaskPolicy

需要修改Task。

---

## 修改方案

Task只负责持有策略：

```cpp
Task
 |
unique_ptr<IResourceAllocationPolicy>
```

策略创建由外部负责：

例如：

```text
Editor
 |
PolicyFactory
 |
Task
```

Task只执行：

```cpp
canAllocateResource()
```

---

## 4. Dependency设计问题

## 4.1 lag语义问题

Coding Agent建议Dependency封装lag计算逻辑。

但当前设计不采纳。

原因：

Dependency职责：

> 描述任务之间的约束。

CPMCalculator职责：

> 解释约束并计算时间。

因此：

Dependency保持数据对象即可。

要求：

在CPM设计文档中明确：

- FS
- SS
- FF
- SF
- Lag正负含义。

---

## 4.2 Dependency唯一性约束

项目要求：

不能存在：

相同：

predecessor
successor

的Dependency。

因此需要定义：

唯一性由：

Project维护。

Project内部：

```text
unordered_set<pair<TaskId,TaskId>>
```

或类似索引。

---

## 4.3 DependencyId问题

项目要求：

显示Dependency序号；
删除Dependency支持容器索引。

因此建议：

内部引入：

```cpp
DependencyId
```

同时保留：

外部显示序号。

即：

内部：

DependencyId

外部：

vector index

---

## 5. Allocation设计问题

## 5.1 命名问题

当前：

ResourceAllocation

容易产生误解：

像一个资源管理动作。

实际上它表示：

Task和Resource之间的关系。

建议：

改为：

Allocation

或：

ResourceAssignment

---

## 5.2 数据职责

Allocation可以保持简单：

```cpp
TaskId
ResourceId
quantity
```

不需要复杂行为。

但是需要明确：

以下规则由Project保证：

- quantity > 0；
- 同一Task-Resource组合是否允许重复；
- 删除Task时自动解除Allocation。

---

## 6. Project设计问题

## 6.1 Project定位

不推荐：

Project = 纯vector容器。

也不推荐：

Project = God Object。

推荐：

Project作为：

> 领域聚合根（Aggregate Root）

负责：

- 管理实体生命周期；
- 保证模型一致性；
- 提供查询接口；
- 提供受控修改接口。

---

## 6.2 Project与Editor关系

最终采用：

Menu
 |
Editor
 |
Project
 |
Task

---

## 6.3 不推荐Project完全开放修改

例如：

```cpp
project.tasks.push_back()
```

会破坏封装。

---

## 6.4 推荐方式

Project提供受控接口：

例如：

```cpp
addTask()
removeTask()
addDependency()
removeDependency()
```

Editor负责：

- 判断用户操作是否合法；
- 组合多个Project操作。

---

## 7. friend设计问题

## 7.1 原问题

设计：

```cpp
friend class ProjectEditor;
friend class PpmImporter;
```

风险：

- Project依赖外部类；
- 新增业务类需要修改Project声明；
- 封装边界被穿透。

---

## 7.2 最终方案

课程项目中：

不必强制使用Passkey。

推荐：

Editor通过Project公开接口完成操作。

原则：

Editor可以调用：

```cpp
project.removeTask(id);
```

但不能访问：

```cpp
project.tasks_;
```

---

## 8. Task删除和级联删除问题

## 8.1 问题

删除Task：

要求：

- 删除Task；
- 删除相关Dependency；
- 删除Allocation。

这是否属于业务逻辑？

---

## 8.2 结论

属于Project内部一致性维护。

原因：

Dependency和Allocation依赖Task存在。

因此：

Project负责：

removeTask()

内部：

1. 删除Task；
2. 删除关联Dependency；
3. 删除Allocation；
4. 更新索引。

Editor负责：

调用和交互。

---

## 9. Project索引设计问题

## 9.1 原设计风险

禁止：

```cpp
unordered_map<TaskId, vector<Dependency*>>
```

原因：

vector扩容导致：

Dependency地址变化。

产生悬空指针。

---

## 9.2 修改方案

索引保存：

DependencyId或者vector下标。

例如：

```cpp
unordered_map<TaskId, vector<DependencyId>>
```

---

## 10. 数据重复和一致性维护

## 10.1 问题

谁保证：

- Task名称唯一；
- TaskId唯一；
- Resource名称唯一；
- Dependency唯一。

---

## 10.2 最终责任划分

### Importer

负责：

输入文件检查。

例如：

发现：

重复Task

直接拒绝导入。

---

### Editor

负责：

用户修改检查。

例如：

新增Task：

检查：

name是否存在

---

### Project

负责：

最终一致性。

例如：

禁止：

两个Task拥有同一个ID

---

三者关系：

Importer
    |
Editor
    |
Project

Project永远保证内部状态合法。

---

## 11. ScheduleResult设计问题

## 11.1 职责

保持独立：

Project
   |
CPMCalculator
   |
ScheduleResult

ScheduleResult不是Project属性。

---

## 11.2 内部结构建议

采用：

```cpp
unordered_map<TaskId, TaskScheduleInfo>
```

其中：

```cpp
struct TaskScheduleInfo
{
    int ES;
    int EF;
    int LS;
    int LF;
    bool critical;
};
```

---

## 11.3 构建方式

统一：

不使用friend。

采用：

构造函数或者builder。

例如：

```cpp
ScheduleResult result(data);
```

避免：

CPMCalculator侵入。

---

## 12. 错误处理策略

当前缺失。

建议统一：

## 可预期错误

使用：

- bool
- optional
- enum error code

例如：

```cpp
bool addTask();
```

---

## 不可恢复错误

使用：

exception。

例如：

内部逻辑错误。

---

## 13. 最终推荐架构

```markdown
             Menu
               |
               v
        SchedulerController
               |
               v
          ProjectEditor
               |
               v

          Project
       /     |      \
    Task Dependency Resource
              |
          Allocation


          CPMCalculator
               |
               v
        ScheduleResult
```
