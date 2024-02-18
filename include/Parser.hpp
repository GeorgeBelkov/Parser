#pragma once

#include <regex>
#include <string>
#include <format>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>

// Путь к Лог-файлу.
constexpr std::string_view filename = "person.log";

using std::chrono::system_clock;
using std::chrono::time_point;


// Класс Парсер. С помощью основного метода parse()
// Анализирует протокол за период и выводит в удобном для
// Просмотра виде статистику.
class Parser
{
private:
    std::unordered_map<std::string, std::pair<std::chrono::seconds, std::size_t>> data; // Хранит статистику
    time_point<system_clock, std::chrono::system_clock::duration> tp_from;              // Начало анализируемого периода
    time_point<system_clock, std::chrono::system_clock::duration> tp_to;                // Конец анализируемого периода
    std::size_t activation_counter;                                                     // Счетчик активаций оборудования
    std::ifstream file;                                                                 // Анализируемый файл
public:
    ~Parser() { file.close(); }
    Parser(std::filesystem::path filename);

    void input();
    void get_info();
    void parse();
    void append_data(std::chrono::seconds working_time, const std::string key = "");
    time_point<system_clock, std::chrono::system_clock::duration> parse_datetime(const std::string& date);
};
