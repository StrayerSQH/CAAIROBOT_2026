# CAAIROBOT_2026

## 问题解决思路整理
### 1.Nav2启动成功后出现卡顿，初始化2D位资失败等问题与nav2_params.yaml参数关系
这个问题大概率来自`base_frame_id: ""`在各个组件下面选取的TF坐标系不同导致的。

### 2.nav2_params.yaml仿真时间问题
如果启用Gazebo等虚拟仿真软件时需要将仿真时间设置为`true`，如果是使用实体机器人的话则需要统一设置为`false`。

### 3.在添加自动初始化机器人位姿时需要安装  tf_transformations 模块
```
pip3 install tf_transformations
```
