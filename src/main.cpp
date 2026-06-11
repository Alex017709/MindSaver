#include "../include/crow_all.h"
#include "user/user_data.h"
#include "data/input_data.h"
#include "date/date.h"
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <Windows.h>
#include <sstream>

int current_logged_in_user_id = -1;
std::string current_logged_in_login = ""; 

date parseHtmlDate(const std::string& d_str) {
    date d = {0, 0, 0};
    char sep1, sep2;
    std::stringstream ss(d_str);
    if (ss >> d.year >> sep1 >> d.month >> sep2 >> d.day) return d;
    return {0, 0, 0};
}

date parseBirthday(const std::string& d_str) {
    date d = {0, 0, 0};
    char sep1, sep2;
    std::stringstream ss(d_str);
    
    // Читаємо у форматі браузера: РІК-МІСЯЦЬ-ДЕНЬ
    if (ss >> d.year >> sep1 >> d.month >> sep2 >> d.day) {
        return d;
    }
    return {0, 0, 0};
}

bool isValidDateServer(int day, int month, int year) {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    int currentYear = now->tm_year + 1900;
    if (year < 1900 || year > currentYear || month < 1 || month > 12) return false;
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 400 == 0 || (year % 100 != 0 && year % 4 == 0)) daysInMonth[1] = 29;
    return day > 0 && day <= daysInMonth[month - 1];
}

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    crow::SimpleApp app;

    // ========================================================= 
    // === МАРШРУТИ HTML СТОРІНОК                            === 
    // ========================================================= 
    CROW_ROUTE(app, "/")([]() { return crow::mustache::load("login.html").render(); });
    CROW_ROUTE(app, "/login.html")([]() { return crow::mustache::load("login.html").render(); });
    CROW_ROUTE(app, "/registr.html")([]() { return crow::mustache::load("registr.html").render(); });
    CROW_ROUTE(app, "/calendar")([]() { return crow::mustache::load("calendar.html").render(); });
    CROW_ROUTE(app, "/stats")([]() { return crow::mustache::load("statistics.html").render(); });
    CROW_ROUTE(app, "/profile")([]() { return crow::mustache::load("profile.html").render(); });
    CROW_ROUTE(app, "/settings")([]() { return crow::mustache::load("settings.html").render(); });
    CROW_ROUTE(app, "/reset")([]() { return crow::mustache::load("reset.html").render(); });
    CROW_ROUTE(app, "/privacy")([]() { return crow::mustache::load("privacy.html").render(); });
    CROW_ROUTE(app, "/system_oc")([]() { return crow::mustache::load("system_oc.html").render(); });

    // ========================================================= 
    // === API ЕНДПОЇНТИ                                      === 
    // ========================================================= 
    
    CROW_ROUTE(app, "/api/logout").methods(crow::HTTPMethod::Post)([]() {
        current_logged_in_user_id = -1;
        current_logged_in_login = "";
        crow::json::wvalue res; 
        res["status"] = "success";
        return crow::response(res);
    });

    CROW_ROUTE(app, "/api/register").methods(crow::HTTPMethod::Post)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        crow::json::wvalue res;
        if (!body) { res["status"] = "error"; res["message"] = "Порожній запит"; return crow::response(400, res); }

        std::string name = body["name"].s();
        std::string password = body["password"].s();
        std::string birthday_str = body["birthday"].s();

        if (name.length() > 250) {
            res["status"] = "error"; res["message"] = "Ім'я перевищує ліміт у 250 символів!"; return crow::response(400, res);
        }
        if (password.length() > 100) {
            res["status"] = "error"; res["message"] = "Пароль перевищує ліміт у 100 символів!"; return crow::response(400, res);
        }

        date dob = parseBirthday(birthday_str);
        if (!isValidDateServer(dob.day, dob.month, dob.year)) {
            res["status"] = "error"; res["message"] = "Некоректна дата народження! Перевірте правильність введених даних."; return crow::response(400, res);
        }

        user U;
        // Перевірка унікальності логіну (імені) перед реєстрацією
        if (U.isLoginTaken(name.c_str())) {
            res["status"] = "error"; 
            res["message"] = "Користувач з таким іменем вже існує! Будь ласка, оберіть інше ім'я."; 
            return crow::response(400, res);
        }

        U.IDgenerate();
        U.singUP(name.c_str(), dob, password.c_str());
        res["status"] = "success";
        return crow::response(200, res);
    });

    CROW_ROUTE(app, "/api/login").methods(crow::HTTPMethod::Post)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        user U;
        std::string login = body["login"].s();
        bool isValid = U.singIN(login.c_str(), std::string(body["password"].s()).c_str());

        crow::json::wvalue res;
        if (isValid) {
            current_logged_in_user_id = U.getID(); 
            current_logged_in_login = login; 
            res["status"] = "success";
        } else {
            res["status"] = "error";
            res["message"] = "Невірний логін або пароль";
        }
        return crow::response(res);
    });

    CROW_ROUTE(app, "/api/change_password").methods(crow::HTTPMethod::Put)
    ([](const crow::request& req) {
        crow::json::wvalue res;
        if (current_logged_in_user_id == -1) return crow::response(401);
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        std::string new_pass = body["password"].s();
        if (new_pass.length() > 100) {
            res["status"] = "error"; res["message"] = "Новий пароль занадто довгий!"; return crow::response(400, res);
        }
        
        user U;
        bool success = U.changePassword(current_logged_in_login.c_str(), new_pass.c_str()); 
        res["status"] = success ? "success" : "error";
        return crow::response(res);
    });

    CROW_ROUTE(app, "/api/reset_password_unauth").methods(crow::HTTPMethod::Put)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        crow::json::wvalue res;
        if (!body) return crow::response(400);

        std::string login = body["login"].s();
        std::string name = body["name"].s();
        std::string birthday_str = body["birthday"].s();
        std::string new_pass = body["new_password"].s();

        if (name.length() > 250 || new_pass.length() > 100) {
            res["status"] = "error"; res["message"] = "Перевищено ліміт символів!"; return crow::response(400, res);
        }

        date dob = parseBirthday(birthday_str);
        if (!isValidDateServer(dob.day, dob.month, dob.year)) {
            res["status"] = "error"; res["message"] = "Некоректна дата народження!"; return crow::response(400, res);
        }

        user U;
        // Двофакторна перевірка: Ім'я (Логін) + Дата народження
        bool isValidData = U.findUser(name.c_str(), dob);
        
        bool success = false;
        if (isValidData) {
            success = U.changePassword(login.c_str(), new_pass.c_str());
        }

        res["status"] = success ? "success" : "error";
        if (!success) res["message"] = "Акаунт з такими даними не знайдено або дані не збігаються!";
        return crow::response(res);
    });

    CROW_ROUTE(app, "/api/update_birthday").methods(crow::HTTPMethod::Put)
    ([](const crow::request& req) {
        crow::json::wvalue res;
        if (current_logged_in_user_id == -1) return crow::response(401);
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        std::string birthday_str = body["birthday"].s();
        date dob = parseBirthday(birthday_str);
        if (!isValidDateServer(dob.day, dob.month, dob.year)) {
            res["status"] = "error"; res["message"] = "Некоректна дата!"; return crow::response(400, res);
        }

        bool success = true; 
        res["status"] = success ? "success" : "error";
        return crow::response(res);
    });

    CROW_ROUTE(app, "/api/delete_account").methods(crow::HTTPMethod::Delete)
    ([](const crow::request& req) {
        crow::json::wvalue res;
        if (current_logged_in_user_id == -1) return crow::response(401);
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        std::string pass = body["password"].s();
        user U; bool success = false;
        if (U.singIN(current_logged_in_login.c_str(), pass.c_str())) {
            success = U.deleteUser(pass.c_str());
        }

        if (success) {
            current_logged_in_user_id = -1; current_logged_in_login = ""; res["status"] = "success";
        } else {
            res["status"] = "error"; res["message"] = "Невірний пароль";
        }
        return crow::response(res);
    });

    CROW_ROUTE(app, "/api/record").methods(crow::HTTPMethod::Put)
    ([](const crow::request& req) {
        if (current_logged_in_user_id == -1) return crow::response(401);
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        date record_date = {0, 0, 0};
        if (body.has("day") && body.has("month") && body.has("year")) {
            record_date.day = (short)body["day"].i(); record_date.month = (short)body["month"].i(); record_date.year = (short)body["year"].i();
        } else {
            time_t t = time(nullptr); tm* now = localtime(&t);
            record_date.day = now->tm_mday; record_date.month = now->tm_mon + 1; record_date.year = now->tm_year + 1900;
        }

        short mood = body["mood"].i(); short energy = body["energy"].i(); short stress = body["stress"].i();
        int hashtag = body["hashtag"].i(); std::string note = body["note"].s();

        input_data I;
        bool edited = I.edit_record(record_date, mood, energy, stress, note.c_str(), hashtag, current_logged_in_user_id);
        if (!edited) {
            I.input_with_date(mood, energy, stress, note.c_str(), hashtag, current_logged_in_user_id, record_date);
        }

        crow::json::wvalue res; res["status"] = "success";
        return crow::response(res);
    });

    CROW_ROUTE(app, "/api/records").methods(crow::HTTPMethod::Get)
    ([]() {
        crow::json::wvalue res;
        if (current_logged_in_user_id == -1) return crow::response(401);

        input_data I; std::vector<FullRecord> records = I.get_all_data(current_logged_in_user_id);
        res["status"] = "success";
        for (size_t i = 0; i < records.size(); ++i) {
            res["records"][i]["nastry"] = records[i].nastry; res["records"][i]["energy"] = records[i].energy; res["records"][i]["stress"] = records[i].stress;
            res["records"][i]["note"] = records[i].note; res["records"][i]["hashtag"] = records[i].hetch;
            res["records"][i]["entry_date"]["day"] = records[i].entry_date.day; res["records"][i]["entry_date"]["month"] = records[i].entry_date.month; res["records"][i]["entry_date"]["year"] = records[i].entry_date.year;
        }
        return crow::response(res);
    });

    CROW_ROUTE(app, "/api/delete").methods(crow::HTTPMethod::Delete)
    ([](const crow::request& req) {
        if (current_logged_in_user_id == -1) return crow::response(401);
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        date d = { (short)body["day"].i(), (short)body["month"].i(), (short)body["year"].i() };
        input_data I; bool deleted = I.delete_record(d, current_logged_in_user_id);

        crow::json::wvalue res; res["status"] = deleted ? "success" : "error";
        if (!deleted) res["message"] = "Не вдалося видалити запис у базі.";
        return crow::response(res);
    });

    CROW_ROUTE(app, "/api/stats_data").methods(crow::HTTPMethod::Post)
    ([](const crow::request& req) {
        if (current_logged_in_user_id == -1) return crow::response(401);
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        date start = parseHtmlDate(std::string(body["start"].s())); date end = parseHtmlDate(std::string(body["end"].s()));
        int hashtag = body["hashtag"].i();
        input_data I; crow::json::wvalue res; res["status"] = "success";

        if (hashtag == -1) {
            StatsResult stats = I.get_statistic(start, end, current_logged_in_user_id);
            res["avg_nastry"] = stats.avg_nastry; res["avg_energy"] = stats.avg_energy; res["avg_stress"] = stats.avg_stress; res["count"] = stats.count;
        } else {
            std::vector<FullRecord> records = I.get_by_hashtag(hashtag, current_logged_in_user_id);
            float n = 0, e = 0, s = 0; int count = 0;
            for (const auto& r : records) {
                if (r.entry_date >= start && r.entry_date <= end) { n += r.nastry; e += r.energy; s += r.stress; count++; }
            }
            res["count"] = count; res["avg_nastry"] = count > 0 ? (n / count) : 0; res["avg_energy"] = count > 0 ? (e / count) : 0; res["avg_stress"] = count > 0 ? (s / count) : 0;
        }
        return crow::response(res);
    });

    CROW_ROUTE(app, "/api/profile_data").methods(crow::HTTPMethod::Get)
    ([]() {
        user u;
        if (current_logged_in_user_id == -1) return crow::response(401);
        
        input_data I;
        std::vector<FullRecord> records = I.get_all_data(current_logged_in_user_id);
        
        float n = 0, e = 0, s = 0;
        for (const auto& r : records) {
            n += r.nastry; e += r.energy; s += r.stress;
        }
        int count = records.size();

        crow::json::wvalue res;
        res["status"] = "success";
        res["id"] = current_logged_in_user_id;
        res["total_records"] = count;
        res["avg_mood"] = count > 0 ? (n / count) : 0;
        res["avg_energy"] = count > 0 ? (e / count) : 0;
        res["avg_stress"] = count > 0 ? (s / count) : 0;
        
        res["name"] = current_logged_in_login; 
        res["surname"] = ""; // Прізвище видалено з БД, повертаємо порожній рядок
        res["birthday"] = u.getBirthdayByID(current_logged_in_user_id);
        
        return crow::response(res);
    });

    std::cout << "Сервер MindSaver успішно запустився на http://localhost:8080" << std::endl;
    app.port(8080).multithreaded().run();
    return 0;
}