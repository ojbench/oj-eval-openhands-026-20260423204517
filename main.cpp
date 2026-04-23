
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <any>

// Forward declarations
class Expression;

// Visitor interface - this is what the test framework expects
class visitor {
public:
    virtual ~visitor() = default;
    virtual std::any visitBinary(class Binary& expr) = 0;
    virtual std::any visitLiteral(class Literal& expr) = 0;
    virtual std::any visitGrouping(class Grouping& expr) = 0;
    virtual std::any visitUnary(class Unary& expr) = 0;
};

// Base Expression class using visitor pattern
class Expression {
public:
    virtual ~Expression() = default;
    virtual std::any accept(visitor& visitor) = 0;
};

// Binary expression (left op right)
class Binary : public Expression {
public:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    char op;

    Binary(std::unique_ptr<Expression> left, char op, std::unique_ptr<Expression> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    std::any accept(visitor& visitor) override {
        return visitor.visitBinary(*this);
    }
};

// Literal expression (number)
class Literal : public Expression {
public:
    double value;

    Literal(double value) : value(value) {}

    std::any accept(visitor& visitor) override {
        return visitor.visitLiteral(*this);
    }
};

// Grouping expression (expression in parentheses)
class Grouping : public Expression {
public:
    std::unique_ptr<Expression> expression;

    Grouping(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) {}

    std::any accept(visitor& visitor) override {
        return visitor.visitGrouping(*this);
    }
};

// Unary expression (-expression)
class Unary : public Expression {
public:
    char op;
    std::unique_ptr<Expression> right;

    Unary(char op, std::unique_ptr<Expression> right)
        : op(op), right(std::move(right)) {}

    std::any accept(visitor& visitor) override {
        return visitor.visitUnary(*this);
    }
};

// Calculator class that implements visitor interface
class calculator : public visitor {
public:
    std::any visitBinary(Binary& expr) override {
        double left = std::any_cast<double>(expr.left->accept(*this));
        double right = std::any_cast<double>(expr.right->accept(*this));

        switch (expr.op) {
            case '+': return left + right;
            case '-': return left - right;
            case '*': return left * right;
            case '/': 
                if (right == 0) throw std::runtime_error("Division by zero");
                return left / right;
            default:
                throw std::runtime_error("Unknown operator");
        }
    }

    std::any visitLiteral(Literal& expr) override {
        return expr.value;
    }

    std::any visitGrouping(Grouping& expr) override {
        return expr.expression->accept(*this);
    }

    std::any visitUnary(Unary& expr) override {
        double right = std::any_cast<double>(expr.right->accept(*this));
        
        switch (expr.op) {
            case '-': return -right;
            default:
                throw std::runtime_error("Unknown unary operator");
        }
    }

    // Public interface for evaluating expressions
    double evaluate(std::unique_ptr<Expression> expr) {
        auto result = expr->accept(*this);
        return std::any_cast<double>(result);
    }
};
