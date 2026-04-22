#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import rclpy
from rclpy.node import Node
import serial
import struct
from geometry_msgs.msg import Twist

# 定义消息格式常量
FRAME_HEAD = 0x52  # ROS发送给STM32的帧头
FRAME_LEN = 25     # 帧总长度
DATA_START = 1     # 数据起始位置
DATA_LEN = 24      # 数据长度 (6个float * 4字节)

class STM32Sender(Node):
    def __init__(self):
        super().__init__('stm32_sender_node')
        
        # 声明参数
        self.declare_parameter('port', '/dev/ttyUSB0')
        self.declare_parameter('baudrate', 230400)
        self.declare_parameter('send_rate', 20)  # 发送频率（Hz）
        
        port = self.get_parameter('port').value
        baudrate = self.get_parameter('baudrate').value
        send_rate = self.get_parameter('send_rate').value
        
        # 打开串口
        try:
            self.serial_port = serial.Serial(
                port=port,
                baudrate=baudrate,
                timeout=0.1
            )
            self.get_logger().info(f"✅ 串口 {port} 已打开 @ {baudrate}bps")
        except serial.SerialException as e:
            self.get_logger().error(f"❌ 无法打开串口 {port}: {e}")
            raise e
        
        # 当前速度命令（默认停止）
        self.current_twist = Twist()
        
        self.send_count = 0
        
        # 订阅 Nav2 发布的 /cmd_vel 话题
        self.create_subscription(
            Twist,
            '/cmd_vel',           # Nav2 发布的速度指令
            self.cmd_vel_callback,
            10
        )
        
        # 定时发送
        self.timer = self.create_timer(1.0 / send_rate, self.timer_callback)
        
        self.get_logger().info("=" * 60)
        self.get_logger().info("📤 STM32发送节点（单向，只发送速度指令）")
        self.get_logger().info(f"📡 串口: {port} @ {baudrate}bps")
        self.get_logger().info(f"🎮 订阅: /cmd_vel → 发送到 STM32")
        self.get_logger().info("=" * 60)
    
    def cmd_vel_callback(self, msg):
        """接收 Nav2 的速度指令"""
        self.current_twist = msg
    
    def timer_callback(self):
        """定时发送速度指令到 STM32"""
        try:
            # 构建发送帧
            frame = bytearray(FRAME_LEN)
            frame[0] = FRAME_HEAD
            
            # 打包6个float数据 (小端格式)
            struct.pack_into('<ffffff', frame, DATA_START,
                           self.current_twist.linear.x,
                           self.current_twist.linear.y,
                           self.current_twist.linear.z,
                           self.current_twist.angular.x,
                           self.current_twist.angular.y,
                           self.current_twist.angular.z)
            
            # 发送到STM32
            self.serial_port.write(frame)
            self.send_count += 1
            
            # 每50帧显示一次
            if self.send_count % 50 == 0:
                self.get_logger().info(
                    f"已发送 {self.send_count} 帧 | "
                    f"v={self.current_twist.linear.x:.2f} m/s, "
                    f"ω={self.current_twist.angular.z:.2f} rad/s"
                )
            
        except Exception as e:
            self.get_logger().error(f"❌ 发送失败: {e}")
    
    def destroy_node(self):
        """关闭串口"""
        if hasattr(self, 'serial_port') and self.serial_port.is_open:
            self.serial_port.close()
            self.get_logger().info("串口已关闭")
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = STM32Sender()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        print("\n👋 程序被用户中断")
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
