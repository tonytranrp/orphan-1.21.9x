#pragma once
//
// Created by vastrakai on 7/7/2024.
//


class ExceptionHandler {
public:
    static void init();
    static void makeCrashLog(const std::string& text, DWORD exceptionCode);
    static void makeCrashConfig(const std::string& name);
};