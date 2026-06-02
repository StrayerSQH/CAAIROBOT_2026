from setuptools import setup
from setuptools import find_packages
import os
from glob import glob

package_name = 'navigation'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        
        # 安装 launch 文件
        (os.path.join('share', package_name, 'launch'), 
         glob('launch/*.launch.py')),
        
        # 安装 config 文件
        (os.path.join('share', package_name, 'config'), 
         glob('config/*.yaml')),
        
        # 安装 maps 文件
        (os.path.join('share', package_name, 'maps'), 
         glob('maps/*.yaml') + glob('maps/*.pgm')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='strayer',
    maintainer_email='strayer@local.com',
    description='Navigation for LD14 robot',
    license='Apache-2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
        ],
    },
)
