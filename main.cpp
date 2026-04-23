
#include <any>

// Forward declarations
class num_node;
class add_node;
class sub_node;
class mul_node;
class div_node;
class node;
class visitor;

// Calculator class declaration
class calculator : public visitor {
public:
    std::any visit_num(num_node* node);
    std::any visit_add(add_node* node);
    std::any visit_sub(sub_node* node);
    std::any visit_mul(mul_node* node);
    std::any visit_div(div_node* node);
    std::any visit(node* n);
};

// Implementation of calculator methods
std::any calculator::visit_num(num_node* node) {
    // Return the value from the num_node
    return node->val;
}

std::any calculator::visit_add(add_node* node) {
    // For addition, visit left and right nodes and add their values
    auto left_val = node->lnode->accept(this);
    auto right_val = node->rnode->accept(this);
    
    // Handle both long long and double types
    if (left_val.type() == typeid(long long) && right_val.type() == typeid(long long)) {
        return std::any_cast<long long>(left_val) + std::any_cast<long long>(right_val);
    } else {
        // Convert to double for mixed types or double operations
        double left = left_val.type() == typeid(long long) ? 
                     static_cast<double>(std::any_cast<long long>(left_val)) : 
                     std::any_cast<double>(left_val);
        double right = right_val.type() == typeid(long long) ? 
                      static_cast<double>(std::any_cast<long long>(right_val)) : 
                      std::any_cast<double>(right_val);
        return left + right;
    }
}

std::any calculator::visit_sub(sub_node* node) {
    auto left_val = node->lnode->accept(this);
    auto right_val = node->rnode->accept(this);
    
    if (left_val.type() == typeid(long long) && right_val.type() == typeid(long long)) {
        return std::any_cast<long long>(left_val) - std::any_cast<long long>(right_val);
    } else {
        double left = left_val.type() == typeid(long long) ? 
                     static_cast<double>(std::any_cast<long long>(left_val)) : 
                     std::any_cast<double>(left_val);
        double right = right_val.type() == typeid(long long) ? 
                      static_cast<double>(std::any_cast<long long>(right_val)) : 
                      std::any_cast<double>(right_val);
        return left - right;
    }
}

std::any calculator::visit_mul(mul_node* node) {
    auto left_val = node->lnode->accept(this);
    auto right_val = node->rnode->accept(this);
    
    if (left_val.type() == typeid(long long) && right_val.type() == typeid(long long)) {
        return std::any_cast<long long>(left_val) * std::any_cast<long long>(right_val);
    } else {
        double left = left_val.type() == typeid(long long) ? 
                     static_cast<double>(std::any_cast<long long>(left_val)) : 
                     std::any_cast<double>(left_val);
        double right = right_val.type() == typeid(long long) ? 
                      static_cast<double>(std::any_cast<long long>(right_val)) : 
                      std::any_cast<double>(right_val);
        return left * right;
    }
}

std::any calculator::visit_div(div_node* node) {
    auto left_val = node->lnode->accept(this);
    auto right_val = node->rnode->accept(this);
    
    // Division should always return double for precision
    double left = left_val.type() == typeid(long long) ? 
                 static_cast<double>(std::any_cast<long long>(left_val)) : 
                 std::any_cast<double>(left_val);
    double right = right_val.type() == typeid(long long) ? 
                  static_cast<double>(std::any_cast<long long>(right_val)) : 
                  std::any_cast<double>(right_val);
    
    if (right == 0.0) {
        throw std::runtime_error("Division by zero");
    }
    
    return left / right;
}

std::any calculator::visit(node* n) {
    // Dispatch to the appropriate visit method based on node type
    return n->accept(this);
}
