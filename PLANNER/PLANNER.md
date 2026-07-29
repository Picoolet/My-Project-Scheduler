# 规划文档

## 简化类图

Project (纯数据载体)
 ├─ vector<Task>               (Task 内部组合 IAllocationPolicy)
 ├─ vector<Dependency>         (Dependency 存前后置 TaskId)
 ├─ vector<Resource>
 ├─ vector<ResourceAllocation>
 └─ 内部邻接索引 (predecessors_, successors_)

Task
 ├─ id, name, duration
 └─ unique_ptr<IAllocationPolicy> policy_   // 根据 duration 自动切换

IAllocationPolicy (接口)
 ├─ NormalPolicy   : canAllocate() = true
 └─ MilestonePolicy: canAllocate() = false

ScheduleResult (纯数据)
 └─ 提供 getEarlyStart(id) 等访问器

CPMCalculator
 └─ ScheduleResult calculate(const Project&) const;

ProjectEditor (门面)
 ├─ TaskEditor
 ├─ DependencyEditor
 ├─ ResourceEditor
 └─ 内部调用 Validator / 图更新

## 我的笔记

- ResourceAllocationPolicy更名为TaskBehavior
- 考虑ProjectEditor成为Project友元的必要性
  - 只让Project提供充足的接口，ProjectEditor通过接口安全操作
- map索引问题：使用DependencyID
- 保持ScheduleResult纯净

- 用模板template写Task ID和Resource ID 并提供拷贝和无效值
