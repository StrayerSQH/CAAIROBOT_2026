#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient
from geometry_msgs.msg import PoseStamped, PoseWithCovarianceStamped
from nav2_msgs.action import NavigateToPose
import math
import time

class AutoNavigator(Node):
    """自动初始化并导航到目标点"""
    
    def __init__(self):
        super().__init__('auto_navigator')
        
        # 创建Action客户端
        self.nav_client = ActionClient(self, NavigateToPose, 'navigate_to_pose')
        
        # 创建初始化位姿发布器
        self.initial_pose_pub = self.create_publisher(
            PoseWithCovarianceStamped, 
            '/initialpose', 
            10
        )
        
        self.get_logger().info('🤖 自动导航节点已启动')
        
        # 状态机
        self.state = 'INITIALIZING'  # INITIALIZING -> NAVIGATING -> DONE
        self.nav_goal_handle = None
        
        # 等待Nav2完全启动后开始
        self.timer = self.create_timer(5.0, self.start_mission)
    
    def start_mission(self):
        """开始任务序列"""
        self.timer.cancel()
        self.get_logger().info('开始执行任务序列...')
        
        # 步骤1: 设置初始位姿
        self.set_initial_pose(0.0, 0.0, 0.0)
        
        # 等待2秒让AMCL收敛
        self.create_timer(3.0, self.navigate_to_first_goal)
    
    def set_initial_pose(self, x, y, yaw_deg):
        """设置机器人初始位姿"""
        pose_msg = PoseWithCovarianceStamped()
        pose_msg.header.stamp = self.get_clock().now().to_msg()
        pose_msg.header.frame_id = 'map'
        
        pose_msg.pose.pose.position.x = float(x)
        pose_msg.pose.pose.position.y = float(y)
        pose_msg.pose.pose.position.z = 0.0
        
        # 转换为四元数
        yaw_rad = math.radians(yaw_deg)
        qz = math.sin(yaw_rad / 2.0)
        qw = math.cos(yaw_rad / 2.0)
        pose_msg.pose.pose.orientation.z = qz
        pose_msg.pose.pose.orientation.w = qw
        
        # 协方差
        pose_msg.pose.covariance[0] = 0.25
        pose_msg.pose.covariance[7] = 0.25
        pose_msg.pose.covariance[35] = 0.0685
        
        # 发布多次确保接收
        for i in range(3):
            self.initial_pose_pub.publish(pose_msg)
            self.get_logger().info(f'发布初始位姿 {i+1}/3: ({x}, {y}, {yaw_deg}°)')
            time.sleep(0.5)
        
        self.get_logger().info('✅ 初始位姿设置完成')
    
    def navigate_to_first_goal(self):
        """导航到第一个目标点"""
        self.get_logger().info('🗺️ 准备导航到目标点...')
        
        # 等待Action服务器就绪
        if not self.nav_client.wait_for_server(timeout_sec=10.0):
            self.get_logger().error('导航服务器未就绪')
            return
        
        # 创建目标点（根据你的地图修改这里！）
        goal_msg = NavigateToPose.Goal()
        goal_msg.pose.header.frame_id = 'map'
        goal_msg.pose.header.stamp = self.get_clock().now().to_msg()
        
        # ========== 修改这里的坐标！ ==========
        # 第一个目标点的坐标（米）和朝向（度）
        goal_x = 5.0      # X坐标
        goal_y = 5.0      # Y坐标  
        goal_yaw = 0.0    # 朝向（0度朝东）
        # =====================================
        
        goal_msg.pose.pose.position.x = goal_x
        goal_msg.pose.pose.position.y = goal_y
        goal_msg.pose.pose.position.z = 0.0
        
        # 设置朝向
        yaw_rad = math.radians(goal_yaw)
        goal_msg.pose.pose.orientation.z = math.sin(yaw_rad / 2.0)
        goal_msg.pose.pose.orientation.w = math.cos(yaw_rad / 2.0)
        
        self.get_logger().info(f'🎯 发送导航目标: ({goal_x}, {goal_y}, {goal_yaw}°)')
        
        # 发送目标点
        send_goal_future = self.nav_client.send_goal_async(
            goal_msg, 
            feedback_callback=self.feedback_callback
        )
        send_goal_future.add_done_callback(self.goal_response_callback)
        
        self.state = 'NAVIGATING'
    
    def feedback_callback(self, feedback_msg):
        """导航反馈回调"""
        feedback = feedback_msg.feedback
        distance = feedback.distance_remaining
        self.get_logger().info(f'📍 剩余距离: {distance:.2f}米', throttle_duration_sec=2.0)
    
    def goal_response_callback(self, future):
        """目标点响应回调"""
        self.nav_goal_handle = future.result()
        
        if not self.nav_goal_handle.accepted:
            self.get_logger().error('目标点被拒绝')
            self.state = 'DONE'
            return
        
        self.get_logger().info('✅ 目标点已被接受，开始导航...')
        
        # 等待结果
        result_future = self.nav_goal_handle.get_result_async()
        result_future.add_done_callback(self.result_callback)
    
    def result_callback(self, future):
        """导航结果回调"""
        result = future.result().result
        
        if result.code == NavigateToPose.Result.SUCCEEDED:
            self.get_logger().info('🎉 成功到达目标点！')
        else:
            self.get_logger().error(f'❌ 导航失败，错误码: {result.code}')
        
        self.state = 'DONE'
        self.get_logger().info('任务完成，节点继续运行...')


def main(args=None):
    rclpy.init(args=args)
    node = AutoNavigator()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info('用户中断')
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
