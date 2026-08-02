# Service 层设计审计报告 (ServiceSchedule_Audit)

下面是综合审查后的 `ServiceSchedule_Audit.md`。文档定位为**设计审计与改进决策记录**，不是重新设计方案。重点保留原设计中合理的架构思想，同时修正职责划分、封装性和可复用性问题。

## 1. 审计目标

本文档用于审查 `ServiceSchedule.md` 中业务层（Service Layer）的设计合理性。

审查重点：

- 是否符合面向对象设计原则
- 是否满足高内聚、低耦合
- 是否具备可复用性和扩展性
- 是否符合 MVC 分层思想
- 是否存在职责过度集中、过度设计或设计不足的问题

审查原则：

- 保留满足当前作业需求且具有长期价值的设计。
- 避免为了体现设计模式而引入无实际收益的抽象。
- 优先保证职责单一和模块可替换性。

---

# 2. 总体评价

原 Service 层设计整体方向正确：

- `ProjectValidator`、`CPMCalculator` 作为无状态业务算法类，职责清晰。
- Importer/Exporter 使用接口隔离文件格式，符合开闭原则。
- Controller 作为 Service 层入口符合当前作业 MVC 架构要求。
- Model 层保持纯领域模型，没有被 UI 或业务逻辑污染。
- ScheduleResult 从 Model 中独立出来，避免调度结果反向污染领域对象。

主要问题集中于：

1. `ProjectController` 职责过重。
2. DTO 构造逻辑不应属于 Controller。
3. 导入流程缺少统一错误反馈机制。
4. 部分设计存在为了未来扩展而提前抽象的问题。
5. Controller 中部分纯算法职责归属不合理。

---

# 3. ProjectController 职责审查

## 3.1 原设计问题

原设计：

```

ProjectController
├── 项目管理
├── Task编辑
├── Dependency编辑
├── Resource编辑
├── DTO生成
├── 导入导出
├── 验证调用
├── 调度调用

```

其中：

- 项目生命周期管理属于 Controller。
- Service 调度属于 Controller。
- Model 修改调用属于 Controller。

这些职责合理。

但是：

- Task 编辑规则
- Dependency 编辑规则
- Resource 编辑规则

全部进入 Controller 后，会导致 Controller 成为 God Object。

## 3.2 ProjectEditor 必须从 Controller 中剥离

结论：

**应增加 ProjectEditor。**

原因：

编辑操作不是简单的 Model setter 调用，而包含：

- 输入合法性检查
- 名称唯一性检查
- ID/index 转换
- 循环依赖检查
- 资源分配规则检查

例如：

```

AddDependency()

=
参数检查
+
Task存在性检查
+
重复检查
+
环检测
+
Project修改

```

这已经属于独立业务逻辑。

推荐结构：

```

ProjectController
|
|
v
ProjectEditor
|
|
v
Project(Model)

```

Controller 职责：

- 管理当前 Project
- 调用 Service
- 向 View 提供接口

ProjectEditor 职责：

- 修改 Project
- 保证修改过程符合业务规则

---

# 4. ProjectDTOBuilder 审查

## 4.1 是否需要剥离？

结论：

**建议保留 ProjectDTOBuilder。**

原因：

虽然当前只有：

```

ProjectController
|
v
ProjectDTOBuilder
|
v
TaskDTO / ResourceDTO

```

看似增加了一层，但其具有长期价值。

原因：

Controller 不应该负责：

- 遍历 Model
- 查询关系
- 组合展示数据
- 处理 Model ID 与 View 数据之间转换

否则 Controller 会同时承担：

```

Controller =
业务协调
+
数据转换
+
UI适配

```

违反单一职责。

## 4.2 DTO命名调整

原：

```

TaskDisplayInfo
DependencyDisplayInfo
ResourceDisplayInfo

```

建议：

```

TaskDTO
DependencyDTO
ResourceDTO
ProjectStatisticsDTO

```

原因：

这些对象不是 UI 专属展示对象，而是：

- Service 与 View 的数据交换对象
- Model 数据投影

DTO 命名更加准确。

---

# 5. Import / Export 设计审查

## 5.1 ImporterFactory 是否必要？

结论：

**不引入 ImporterFactory。**

原因：

当前需求：

- 仅要求 PPM 格式。

Factory：

```

file extension
|
v
ImporterFactory
|
+---- PpmImporter
+---- JsonImporter
+---- XxxImporter

```

虽然体现设计模式，但当前收益有限。

更合理：

由上层界面决定：

```

Console/UI
|
| 用户选择格式
|
v

PpmImporter
JsonImporter

```

Service 不负责文件格式选择。

因此：

```

Controller
|
v
具体Importer

````

即可。

未来格式大量增加时，再引入 Factory。

---

# 6. ImportResult 引入审查

## 6.1 原问题

原接口：

```cpp
unique_ptr<Project> Import();
````

问题：

失败只能返回：

```
nullptr
```

无法区分：

- 文件不存在
- 格式错误
- 字段缺失
- ID重复
- 数据非法

错误处理职责被迫进入 Controller。

---

## 6.2 修改方案

引入：

```
ImportResult
```

职责：

封装导入结果。

包含：

```
ImportResult
{
    optional<Project>
    vector<string> errors
    vector<string> warnings
}
```

语义：

成功：

```
project != nullptr
errors.empty()
```

失败：

```
project == nullptr
errors contains reason
```

优势：

- 导入器负责报告错误。
- Controller 只负责转发结果。
- 保持错误信息完整。

---

# 7. ProjectValidator 审查

## 7.1 是否需要 IValidator？

结论：

**不需要。**

原因：

当前需求：

```
ProjectValidator
```

已经满足：

- 无状态
- 可测试
- 可替换

增加：

```
IValidator
      |
      +-- ProjectValidator
      +-- ScheduleValidator
```

属于提前抽象。

只有当：

- 多种验证体系同时存在
- 用户动态选择验证规则

时才有价值。

当前不引入。

---

# 8. WouldCreateCycle 归属审查

## 8.1 原设计

```
ProjectController
    |
    private:
       WouldCreateCycle()
```

问题：

该函数：

- 不涉及 Controller 状态。
- 是纯图算法。
- 只依赖 Project。

因此不属于 Controller。

---

## 8.2 调整方案

移动到：

```
ProjectValidator
```

形式：

```
ProjectValidator
    |
    +-- Validate()
    |
    +-- WouldCreateCycle()
```

原因：

统一图算法职责：

```
DAG检查
+
环检测
+
合法性验证
```

同时方便：

- AddDependency 前调用
- 单元测试

---

# 9. TaskId 与 index 设计审查

## 9.1 问题本质

问题不是：

```
vector vs map
```

而是：

```
TaskId -> Task定位效率
```

以及：

```
View index 与 Model ID 的关系
```

---

## 9.2 保留 vector 作为唯一数据源

不建议：

```
unordered_map<TaskId, Task>
```

原因：

- 双容器维护复杂。
- 删除关系处理复杂。
- 破坏原有顺序。

保留：

```
vector<Task> tasks_
```

作为唯一数据。

---

## 9.3 增加位置索引

推荐：

```
vector<Task> tasks_

unordered_map<TaskId,size_t>
        taskPositionIndex_
```

作用：

```
TaskId
 |
 v
vector index
 |
 v
Task
```

优势：

- 保留 vector 优点。
- FindTask O(1)。
- 不改变数据结构。

删除：

由于项目规模：

```
几十~几百 Task
```

删除后：

直接重建：

```
taskPositionIndex_
```

成本：

O(n)

可接受。

---

# 10. GetStatistics 设计审查

## 10.1 原问题

原设计：

```
GetStatistics()

如果未验证:
 totalDuration=-1
```

问题：

统计接口不应该依赖调用顺序。

---

## 10.2 推荐修改

ProjectStatistics 增加：

```
bool isValid;
```

流程：

```
GetStatistics()

 |
 +-- Validate()
 |
 +-- CPM Calculate()
 |
 +-- 返回统计结果
```

结果：

```
isValid=false
totalDuration=undefined
```

或者：

```
totalDuration=-1
```

优势：

调用者无需知道：

“是否必须先 Validate”。

---

# 11. Controller 单例设计审查

## 结论

保留。

原因：

这是作业核心要求。

同时：

Controller 单例不会影响 Model 复用。

正确关系：

```
Controller(singleton)

        使用

Project(Model)
```

Model：

- 不知道 Controller。
- 可独立测试。
- 可被其他程序复用。

---

# 12. 最终推荐 Service 架构

```
                    View/UI
                       |
                       v

             ProjectController
              (Singleton)
                       |
       --------------------------------
       |              |               |
       v              v               v

 ProjectEditor   DTOBuilder     Import/Export

       |              |               |
       v              v               v

              Project(Model)


       |
       |
       +----------------+
       |                |
       v                v

ProjectValidator   CPMCalculator


       |
       v

 ScheduleResult
```

---

# 13. 最终修改清单

## 保留

- ProjectController Singleton
- ProjectValidator
- CPMCalculator
- Importer/Exporter接口
- vector作为Model主要容器
- ScheduleResult独立设计

## 修改

### 必须修改

1. Controller 中剥离 ProjectEditor。
2. 引入 ProjectDTOBuilder。
3. ImportResult 替代裸 nullptr 错误传递。
4. WouldCreateCycle 移入 ProjectValidator。

### 建议修改

1. DisplayInfo 改名 DTO。
2. ProjectStatistics 增加有效状态。

### 不采用

1. ImporterFactory。
2. IValidator虚基类。
3. Task/Resource Editor 继续细分为多个类。

---

# 14. 总结

最终 Service 层设计应遵循：

> Controller负责协调，Editor负责修改，Builder负责转换，Validator负责检查，Calculator负责计算，Importer负责解析。

这样既满足 MVC，又避免过度设计。

该架构：

- 面向对象职责清晰。
- Model保持纯净。
- Service具备复用能力。
- 可自然扩展GUI、Console或其他控制器。
- 不为了体现设计模式而增加无价值抽象。
