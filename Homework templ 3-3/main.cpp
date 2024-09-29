#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

enum class LogType {
    Warning,
    Error,
    FatalError,
    Unknown
};

class LogMessage {
public:
    LogMessage(LogType type, const std::string& message)
        : type_(type), message_(message) {}

    LogType type() const { return type_; }
    const std::string& message() const { return message_; }

private:
    LogType type_;
    std::string message_;
};


class LogHandler {
public:
    virtual ~LogHandler() = default;

    void setNext(LogHandler* next) {
        next_ = next;
    }

    virtual void handle(const LogMessage& message) {
        if (next_) {
            next_->handle(message);
        }
    }

protected:
    LogHandler* next_ = nullptr;
};

class FatalErrorHandler : public LogHandler {
public:
    void handle(const LogMessage& message) override {
        if (message.type() == LogType::FatalError) {
            throw std::runtime_error("Fatal Error: " + message.message());
        }
        else {
            LogHandler::handle(message);
        }
    }
};

class ErrorHandler : public LogHandler {
    std::string filepath_;
public:
    ErrorHandler(const std::string& filepath) : filepath_(filepath) {}

    void handle(const LogMessage& message) override {
        if (message.type() == LogType::Error) {
            std::ofstream outfile(filepath_, std::ios::app);
            if (outfile.is_open()) {
                outfile << "Error: " << message.message() << std::endl;
                outfile.close();
            }
            else {
                std::cerr << "Failed to open file: " << filepath_ << std::endl;
            }
        }
        else {
            LogHandler::handle(message);
        }
    }
};

class WarningHandler : public LogHandler {
public:
    void handle(const LogMessage& message) override {
        if (message.type() == LogType::Warning) {
            std::cout << "Warning: " << message.message() << std::endl;
        }
        else {
            LogHandler::handle(message);
        }
    }
};


class UnknownHandler : public LogHandler {
public:
    void handle(const LogMessage& message) override {
        if (message.type() == LogType::Unknown) {
            throw std::runtime_error("Unknown message: " + message.message());
        }
        else {
            LogHandler::handle(message);
        }
    }
};

int main() {
    

    FatalErrorHandler fatalHandler;
    ErrorHandler errorHandler("error_log.txt");
    WarningHandler warningHandler;
    UnknownHandler unknownHandler;


    fatalHandler.setNext(&errorHandler);
    errorHandler.setNext(&warningHandler);
    warningHandler.setNext(&unknownHandler);


    try {
        LogMessage warningMsg(LogType::Warning, "This is a warning message.");
        LogMessage errorMsg(LogType::Error, "This is an error message.");
        LogMessage fatalMsg(LogType::FatalError, "This is a fatal error message.");
        LogMessage unknownMsg(LogType::Unknown, "This is an unknown message.");

        fatalHandler.handle(warningMsg);
        fatalHandler.handle(errorMsg);
        fatalHandler.handle(fatalMsg);
        fatalHandler.handle(unknownMsg);
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }

    return 0;
}
