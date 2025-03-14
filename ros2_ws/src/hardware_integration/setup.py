from setuptools import find_packages, setup

package_name = 'hardware_integration'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='jetson',
    maintainer_email='erykhalicki0@gmail.com',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            "serial_output=hardware_integration.serial_output_basic:main",
            "gui=hardware_integration.gui_tester:main",
            "cli=hardware_integration.cli:main",
            "pid_cli=hardware_integration.cli_pid:main",
            "qual=hardware_integration.qualification:main",
            "dvl_driver=hardware_integration.dvl_driver:main",
            "dvl_dummy_driver=hardware_integration.dvl_dummy:main",
            "temp_sensor=hardware_integration.temp_sensor:main",
        ],
    },
)
