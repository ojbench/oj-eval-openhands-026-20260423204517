
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <any>

// Forward declarations
class Visitor;
class Expression;

// Base Expression class using visitor pattern
class Expression {
public:
    virtual ~Expression() = default;
    virtual std::any accept(Visitor& visitor) = 0;
};

// Visitor interface
class Visitor {
public:
    virtual ~Visitor() = default;
    virtual std::any visitBinary(class Binary& expr) = 0;
    virtual std::any visitLiteral(class Literal& expr) = 0;
    virtual std::any visitGrouping(class Grouping& expr) = 0;
    virtual std::any visitUnary(class Unary& expr) = 0;
};

// Binary expression (left op right)
class Binary : public Expression {
public:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    char op;

    Binary(std::unique_ptr<Expression> left, char op, std::unique_ptr<Expression> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    std::any accept(Visitor& visitor) override {
        return visitor.visitBinary(*this);
    }
};

// Literal expression (number)
class Literal : public Expression {
public:
    double value;

    Literal(double value) : value(value) {}

    std::any accept(Visitor& visitor) override {
        return visitor.visitLiteral(*this);
    }
};

// Grouping expression (expression in parentheses)
class Grouping : public Expression {
public:
    std::unique_ptr<Expression> expression;

    Grouping(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) {}

    std::any accept(Visitor& visitor) override {
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

    std::any accept(Visitor& visitor) override {
        return visitor.visitUnary(*this);
    }
};

// Evaluator visitor
class Evaluator : public Visitor {
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
};

// Token class for parsing
class Token {
public:
    enum Type { NUMBER, PLUS, MINUS, STAR, SLASH, LPAREN, RPAREN, EOF_TOKEN };

    Type type;
    double value;
    char lexeme;

    Token(Type type, double value = 0, char lexeme = '\0') 
        : type(type), value(value), lexeme(lexeme) {}
};

// Lexer/Tokenizer
class Lexer {
private:
    std::string text;
    size_t pos;

    char current() {
        if (pos >= text.length()) return '\0';
        return text[pos];
    }

    char peek() {
        if (pos + 1 >= text.length()) return '\0';
        return text[pos + 1];
    }

    void advance() {
        pos++;
    }

    void skipWhitespace() {
        while (current() && std::isspace(current())) {
            advance();
        }
    }

public:
    Lexer(const std::string& text) : text(text), pos(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        
        while (true) {
            skipWhitespace();
            
            char c = current();
            if (!c) {
                tokens.emplace_back(Token::EOF_TOKEN);
                break;
            }

            if (std::isdigit(c) || c == '.') {
                // Number
                std::string num_str;
                while (current() && (std::isdigit(current()) || current() == '.')) {
                    num_str += current();
                    advance();
                }
                double value = std::stod(num_str);
                tokens.emplace_back(Token::NUMBER, value);
            } else {
                switch (c) {
                    case '+': tokens.emplace_back(Token::PLUS, 0, '+'); advance(); break;
                    case '-': tokens.emplace_back(Token::MINUS, 0, '-'); advance(); break;
                    case '*': tokens.emplace_back(Token::STAR, 0, '*'); advance(); break;
                    case '/': tokens.emplace_back(Token::SLASH, 0, '/'); advance(); break;
                    case '(': tokens.emplace_back(Token::LPAREN, 0, '('); advance(); break;
                    case ')': tokens.emplace_back(Token::RPAREN, 0, ')'); advance(); break;
                    default:
                        throw std::runtime_error(std::string("Unexpected character: ") + c);
                }
            }
        }
        
        return tokens;
    }
};

// Parser
class Parser {
private:
    std::vector<Token> tokens;
    size_t current;

    Token& peek() {
        return tokens[current];
    }

    Token& advance() {
        if (!isAtEnd()) current++;
        return tokens[current - 1];
    }

    bool isAtEnd() {
        return peek().type == Token::EOF_TOKEN;
    }

    bool match(Token::Type type) {
        if (peek().type == type) {
            advance();
            return true;
        }
        return false;
    }

    Token& consume(Token::Type type, const std::string& message) {
        if (peek().type == type) return advance();
        throw std::runtime_error(message);
    }

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

    std::unique_ptr<Expression> parse() {
        return expression();
    }

private:
    std::unique_ptr<Expression> expression() {
        return term();
    }

    std::unique_ptr<Expression> term() {
        std::unique_ptr<Expression> expr = factor();

        while (match(Token::PLUS) || match(Token::MINUS)) {
            Token op = tokens[current - 1];
            std::unique_ptr<Expression> right = factor();
            expr = std::make_unique<Binary>(std::move(expr), op.lexeme, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expression> factor() {
        std::unique_ptr<Expression> expr = unary();

        while (match(Token::STAR) || match(Token::SLASH)) {
            Token op = tokens[current - 1];
            std::unique_ptr<Expression> right = unary();
            expr = std::make_unique<Binary>(std::move(expr), op.lexeme, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expression> unary() {
        if (match(Token::MINUS)) {
            Token op = tokens[current - 1];
            std::unique_ptr<Expression> right = unary();
            return std::make_unique<Unary>(op.lexeme, std::move(right));
        }

        return primary();
    }

    std::unique_ptr<Expression> primary() {
        if (match(Token::NUMBER)) {
            return std::make_unique<Literal>(tokens[current - 1].value);
        }

        if (match(Token::LPAREN)) {
            std::unique_ptr<Expression> expr = expression();
            consume(Token::RPAREN, "Expect ')' after expression.");
            return std::make_unique<Grouping>(std::move(expr));
        }

        throw std::runtime_error("Expect expression.");
    }
};

// Calculator class
class Calculator {
public:
    double evaluate(const std::string& expression) {
        try {
            Lexer lexer(expression);
            auto tokens = lexer.tokenize();
            
            Parser parser(tokens);
            auto expr = parser.parse();
            
            Evaluator evaluator;
            auto result = expr->accept(evaluator);
            return std::any_cast<double>(result);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Error: ") + e.what());
        }
    }
};

int main() {
    Calculator calculator;
    std::string line;
    
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        
        try {
            double result = calculator.evaluate(line);
            std::cout << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
    
    return 0;
}
