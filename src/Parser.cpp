#include "../include/Parser.hpp"


Parser::Parser(std::filesystem::path filename)
{
    data.clear();
    file.open(filename, std::ios::in);
    activation_counter = 0;
}


void Parser::input()
{
    // Вводим с консоли две даты, для определения временных границ
    // Необходимых для дальнейего поиска.
    std::tm start, end;
    std::string date_from, date_to;

    std::cout << "Введите левую границу поиска: ";
    std::getline(std::cin, date_from);
    std::cout << "Введите правую границу поиска: ";
    std::getline(std::cin, date_to);

    // Сохраняем временные границы в переменные типа time_point.
    std::istringstream sstream_start(date_from), sstream_end(date_to);
    sstream_end >> std::get_time(&end, "%d.%m.%y %H:%M:%S");
    sstream_start >> std::get_time(&start, "%d.%m.%y %H:%M:%S");

    tp_from = std::chrono::system_clock::from_time_t(std::mktime(&start));
    tp_to = std::chrono::system_clock::from_time_t(std::mktime(&end));
}


void Parser::get_info()
{
    using namespace std::chrono_literals;

    std::cout << std::endl;

    if (data.empty())
        std::cout << "Под введенные вами временные рамки не попал ни 1 лог." << std::endl;
    
    for (const auto [name, info] : data)
    {
        auto hours = std::chrono::duration_cast<std::chrono::hours>(info.first);
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(info.first % 1h);
        auto second = std::chrono::duration_cast<std::chrono::seconds>(info.first % 1min);
        activation_counter += info.second;

        if (!name.compare(""))
        {
            std::cout << "Время работы устройства без работника: " << hours.count()
                      << " часов, " << minutes.count() << " минут, " << second.count()
                      << " секунд; Количество включений: " << info.second << std::endl;
        }
        else
        {
            std::cout << "Работник: " << name << " проработал на установке: " << hours.count()
                      << " часов, " << minutes.count() << " минут, " << second.count()
                      << " секунд; Количество включений: " << info.second << std::endl;
        }
    }
    std::cout << "Общее число включений: " << activation_counter << std::endl;
}


// Принимает на вход дату в виде строки
// Возвращает дату объект std::chrono::time_point
time_point<system_clock, std::chrono::system_clock::duration> Parser::parse_datetime(const std::string& date)
{
    std::tm time;
    std::istringstream sstream(date);
    sstream >> std::get_time(&time, "%d.%m.%y %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&time));
}


// Метод позволяет добавить в контейнер
// Данные, собранные с очередного интересующего
// Нас лога. 
void Parser::append_data(std::chrono::seconds working_time, const std::string key)
{
    // Если такого Работника еще не было - инициализируем
    if (!data.contains(key))
    {
        data[key].first = working_time;
        data[key].second = 1;
    }
    // Иначе добавляем к уже имеющимся данным - вновь полученные
    else
    {
        data[key].first += working_time;
        data[key].second += 1;
    }
}


void Parser::parse()
{
    // Запрашиваем у пользователя даты
    // Для которых будет происходить обработка.
    input();

    std::string log_item;
    while (std::getline(file, log_item, '\n'))
    {
        if (std::regex_search(log_item, std::regex("\\*+", std::regex_constants::optimize)))
        {
            // Считываем из файла очередной лог.
            std::getline(file, log_item, '*');
            std::regex datetime(R"(\d+.\d+.\d+\s\d+:\d+:\d+)", std::regex_constants::optimize);
            std::vector<std::string> time_points;
            
            // Ищем время начала и конца работы устройства и проверяем
            // Попадает ли этот лог в диапазон интересующих нас.
            for(auto i = std::sregex_iterator(log_item.begin(), log_item.end(), datetime); i != std::sregex_iterator(); ++i)
                time_points.push_back(i->str());

            // Если время начала работы >= правой границы
            // И <= левой, следовательно, данный лог актуален.
            auto start_time = parse_datetime(time_points[0]);
            auto end_time = parse_datetime(time_points[1]);

            // Если время активации попадает в требуемые временные рамки
            // Вносим данные в поле data
            if (start_time >= tp_from && start_time <= tp_to)
            {
                // Узнаем имя оператора, если оно есть и
                // Определяем время работы за оборудованием
                std::regex name_format("Оператор\\s-\\s{2}(.+)", std::regex_constants::optimize);
                std::smatch matched_sequence;
                auto working_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
                if (std::regex_search(log_item, matched_sequence, name_format))
                {
                    auto name = matched_sequence[1];
                    append_data(working_time, name);
                }
                else
                    append_data(working_time);
            }
        }
    }
    // Выводим собранную статистику
    // В консоль в читаемом виде
    get_info();
}