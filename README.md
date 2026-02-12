# AIPhysicsSimulation (UE5.4)

这是一个 **Unreal Engine 5.4 可直接打开运行** 的 XPBD 布料 Compute Shader Demo 项目。

## 已包含内容

- UE 工程文件：`AIPhysicsSimulation.uproject`
- 游戏主模块：`AIPhysicsSimulation`
- XPBD 运行时模块：`XPBDCloth`
- Compute Shader：`Shaders/Private/XPBDCloth.usf`
- RDG 调度代码：`Source/XPBDCloth/XPBDClothPasses.cpp`
- 可直接拖入场景的演示 Actor：`AXPBDClothDemoActor`

## XPBD 管线

每帧执行三阶段：

1. `PredictPositionsCS`：半隐式积分预测位置
2. `SolveDistanceConstraintsCS`：按 XPBD 公式迭代距离约束
3. `UpdateVelocityCS`：位置差分回写速度和新位置

> 注：约束求解写回是教学实现，仍有并发写冲突风险；生产环境请使用 graph coloring 或 Jacobi 双缓冲。

## 如何在 UE5.4 运行

1. 用 UE5.4 打开 `AIPhysicsSimulation.uproject`。
2. 首次打开时选择编译（或先用 IDE 编译 Editor Target）。
3. 新建/打开一个关卡。
4. 在 Place Actors 中搜索并拖入 `XPBDClothDemoActor`。
5. 点击 Play。

运行后每帧会在渲染线程调度 XPBD 三个 Compute Pass（用于演示完整接线流程）。

## 关键源码入口

- Shader 注册与 RDG 调度：`Source/XPBDCloth/XPBDClothPasses.cpp`
- Shader 目录映射：`Source/XPBDCloth/Private/XPBDClothModule.cpp`
- Demo 组件（生成网格粒子与约束并每帧调度）：`Source/XPBDCloth/Private/XPBDClothSimulationComponent.cpp`
- HLSL：`Shaders/Private/XPBDCloth.usf`

## 后续可扩展建议

- 增加 bending / attachment / collision 约束。
- 添加可视化（ProceduralMesh/Niagara/GPU readback）展示粒子位置。
- 增加子步进、约束分组、Jacobi 求解提升稳定性与并行安全性。
