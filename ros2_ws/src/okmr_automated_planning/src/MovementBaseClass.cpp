#include "MovementBaseClass.h"

MovementBaseClass::MovementBaseClass(const std::string &name, const BT::NodeConfiguration &config)
    : BT::SyncActionNode(name, config), message_sent_(false) {
 
    node_ = rclcpp::Node::make_shared("_" + name + "_node");

  
    publisher_ = node_->create_publisher<okmr_msgs::msg::MovementCommand>("movement_command", 10);

    
    client_ = node_->create_client<okmr_msgs::srv::Status>("/navigator_status");


    while (!client_->wait_for_service(std::chrono::seconds(1))) {
        RCLCPP_INFO(node_->get_logger(), "Service /navigator_status not available, waiting...");
    } 
}

BT::NodeStatus MovementBaseClass::tick() {
    if (!message_sent_) {
        
        okmr_msgs::msg::MovementCommand msg;
        msg.command = 1;
        msg.header.stamp = node_->get_clock()->now();
        setCommandData(msg);
        publisher_->publish(msg);
        RCLCPP_INFO(node_->get_logger(), "Published MovementCommand with command: %d", command_);

     //   RCLCPP_INFO(node_->get_logger(), "Published MovementCommand with command: %s", command_.c_str());
        message_sent_ = true; 
    }

  
    auto request = std::make_shared<okmr_msgs::srv::Status::Request>();
    bool keepSpinning=true;
    while(keepSpinning){
        auto future = client_->async_send_request(request);
        if (rclcpp::spin_until_future_complete(node_, future) == rclcpp::FutureReturnCode::SUCCESS) {
            auto response = future.get();
            if (response->ongoing) {
                continue;
            } else {
                message_sent_ = false;
                keepSpinning = false;
                return response->success ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
            }
        } else {
            RCLCPP_ERROR(node_->get_logger(), "Service call failed");
            return BT::NodeStatus::FAILURE;
        }
    }
}

void MovementBaseClass::setCommandData(okmr_msgs::msg::MovementCommand &msg) {
    throw std::runtime_error("setCommandData must be overridden in a derived class");
}

//MovementBaseClass::~MovementBaseClass() {
 //   RCLCPP_INFO(node_->get_logger(), "Destroying MovementBehaviour node");
//}


