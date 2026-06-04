#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import PoseWithCovarianceStamped
import tf_transformations
import math
import time

class AutoInitializer(Node):
    """自动初始化机器人位姿的节点"""
    
    def __init__(self):
        super().__init__('auto_initializer')
        
        # 创建发布器，发布到 /initialpose 话题
        self.initial_pose_pub = self.create_publisher(
            PoseWithCovarianceStamped, 
            '/initialpose', 
            10
        )
        
        # 日志输出
        self.get_logger().info('🤖 自动初始化节点已启动')
        self.get_logger().info('等待Nav2系统准备就绪...')
        
        # 等待3秒后执行初始化（确保Nav2完全启动）
        self.timer = self.create_timer(3.0, self.initialize_robot)
    
    def initialize_robot(self):
        """执行机器人初始化"""
        # 取消定时器，只执行一次
        self.timer.cancel()
        
        # 设置初始位姿参数（根据你的实际地图修改）
        # 格式: (x坐标, y坐标, 朝向角度)
        initial_x = 0.0      # X坐标（米）
        initial_y = 0.0      # Y坐标（米）
        initial_yaw = 0.0    # 朝向（度，0度面向x轴正方向）
        
        # 发布初始位姿
        self.publish_initial_pose(initial_x, initial_y, initial_yaw)
        
        # 等待AMCL收敛
        self.get_logger().info('等待AMCL粒子滤波收敛...')
        time.sleep(2.0)
        
        # 再次发布一次，确保被接收
        self.publish_initial_pose(initial_x, initial_y, initial_yaw)
        
        self.get_logger().info('✅ 机器人初始位姿设置完成！')
        self.get_logger().info(f'   位置: ({initial_x}, {initial_y})')
        self.get_logger().info(f'   朝向: {initial_yaw}°')
        
        # 保持节点运行，不退出（以便后续扩展）
        self.get_logger().info('📌 初始化完成，节点继续运行中...')
    
    def publish_initial_pose(self, x, y, yaw_deg):
        """发布初始位姿消息"""
        pose_msg = PoseWithCovarianceStamped()
        
        # 设置消息头
        pose_msg.header.stamp = self.get_clock().now().to_msg()
        pose_msg.header.frame_id = 'map'  # 重要：必须使用map坐标系
        
        # 设置位置
        pose_msg.pose.pose.position.x = float(x)
        pose_msg.pose.pose.position.y = float(y)
        pose_msg.pose.pose.position.z = 0.0
        
        # 将欧拉角（度）转换为四元数
        yaw_rad = math.radians(yaw_deg)
        quat = tf_transformations.quaternion_from_euler(0, 0, yaw_rad)
        pose_msg.pose.pose.orientation.x = quat[0]
        pose_msg.pose.pose.orientation.y = quat[1]
        pose_msg.pose.pose.orientation.z = quat[2]
        pose_msg.pose.pose.orientation.w = quat[3]
        
        # 设置协方差矩阵（表示初始位姿的不确定性）
        # 较小的值表示较高的置信度
        pose_msg.pose.covariance = [
            0.25, 0.0, 0.0, 0.0, 0.0, 0.0,   # x
            0.0, 0.25, 0.0, 0.0, 0.0, 0.0,   # y
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,    # z
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,    # roll
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,    # pitch
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0685  # yaw
        ]
        
        # 发布消息
        self.initial_pose_pub.publish(pose_msg)
        self.get_logger().debug(f'已发布初始位姿: ({x}, {y}, {yaw_deg}°)')


def main(args=None):
    rclpy.init(args=args)
    
    node = AutoInitializer()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info('用户中断')
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
