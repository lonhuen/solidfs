#pragma once

#include <execinfo.h>
#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <system_error>

#include "utils/string_utils.h"

// mainly taken from a private repo called "nova"

namespace solid {
    class fs_exception : public std::system_error{
        
        const std::string msg;
    public:
        explicit fs_exception(std::errc code, const std::string& msg) : std::system_error(
            std::make_error_code(code),
            construct_exception_message(msg)
        ) {
        }

        template<typename ... ArgT>
        explicit fs_exception(std::errc code, ArgT&& ... args) : std::system_error(
            std::make_error_code(code),
            construct_exception_message(String::of(std::forward<ArgT>(args)...))) {
        }

        const char* what() const noexcept override {
            return std::system_error::what();
        }

    private:
        static std::string construct_exception_message(const std::string& msg) {
            void* stack_trace[100];
            auto size = backtrace(stack_trace, 100);
            char** trace = backtrace_symbols(stack_trace, size);
            std::ostringstream oss;
            oss << msg << '\n' << "Stack Trace:\n";
            for (unsigned i = 0; i < size; i++) {
            oss << "  " << trace[i] << '\n';
            }
            free(trace);
            return oss.str();
        }
    };
    class fs_error : public std::exception {
        
        const std::string msg;
    public:
        explicit fs_error(const std::string& msg) : 
            msg(construct_exception_message(msg)){
        }

        template<typename ... ArgT>
        explicit fs_error(ArgT&& ... args) :
            msg(construct_exception_message(String::of(std::forward<ArgT>(args)...))) {
        }

        const char* what() const noexcept override {
            return msg.data();
        }

    private:
        static std::string construct_exception_message(const std::string& msg) {
            void* stack_trace[100];
            auto size = backtrace(stack_trace, 100);
            char** trace = backtrace_symbols(stack_trace, size);
            std::ostringstream oss;
            oss << msg << '\n' << "Stack Trace:\n";
            for (unsigned i = 0; i < size; i++) {
            oss << "  " << trace[i] << '\n';
            }
            free(trace);
            return oss.str();
        }
    };
    class fs_warning: public std::exception {
        
        const std::string msg;
    public:
        explicit fs_warning(const std::string& msg) : 
            msg(construct_exception_message(msg)){
        }

        template<typename ... ArgT>
        explicit fs_warning(ArgT&& ... args) :
            msg(construct_exception_message(String::of(std::forward<ArgT>(args)...))) {
        }

        const char* what() const noexcept override {
            return msg.data();
        }

    private:
        static std::string construct_exception_message(const std::string& msg) {
            void* stack_trace[100];
            auto size = backtrace(stack_trace, 100);
            char** trace = backtrace_symbols(stack_trace, size);
            std::ostringstream oss;
            oss << msg << '\n' << "Stack Trace:\n";
            for (unsigned i = 0; i < size; i++) {
            oss << "  " << trace[i] << '\n';
            }
            free(trace);
            return oss.str();
        }
    };
};