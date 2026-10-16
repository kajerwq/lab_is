// Учебный HTTP-сервер (ЛР4), без сторонних библиотек
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/priority_strategy.hpp"
#include "taskflow/task_service.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string esc(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"' || c == '\\') out.push_back('\\');
        out.push_back(c);
    }
    return out;
}

std::string to_json(const taskflow::Task& t) {
    std::ostringstream o;
    o << "{\"id\":" << t.id
      << ",\"title\":\"" << esc(t.title) << "\""
      << ",\"assignee\":\"" << esc(t.assignee) << "\""
      << ",\"priority\":" << t.priority
      << ",\"status\":\"" << taskflow::to_string(t.status) << "\""
      << ",\"description\":\"" << esc(t.description) << "\""
      << ",\"days_left\":" << t.days_left
      << "}";
    return o.str();
}

std::string list_json(const std::vector<taskflow::Task>& items) {
    std::ostringstream o;
    o << "[";
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i) o << ",";
        o << to_json(items[i]);
    }
    o << "]";
    return o.str();
}

std::string field_str(const std::string& body, const std::string& key) {
    const auto key_pos = body.find("\"" + key + "\"");
    if (key_pos == std::string::npos) return "";
    auto q1 = body.find('"', body.find(':', key_pos));
    if (q1 == std::string::npos) return "";
    auto q2 = body.find('"', q1 + 1);
    if (q2 == std::string::npos) return "";
    return body.substr(q1 + 1, q2 - q1 - 1);
}

int field_int(const std::string& body, const std::string& key, int def) {
    const auto key_pos = body.find("\"" + key + "\"");
    if (key_pos == std::string::npos) return def;
    auto colon = body.find(':', key_pos);
    if (colon == std::string::npos) return def;
    try {
        return std::stoi(body.substr(colon + 1));
    } catch (...) {
        return def;
    }
}

std::string reply(int code, const std::string& reason, const std::string& body) {
    std::ostringstream o;
    o << "HTTP/1.1 " << code << " " << reason << "\r\n"
      << "Content-Type: application/json\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Connection: close\r\n\r\n"
      << body;
    return o.str();
}

} // namespace

int main() {
    using namespace taskflow;

    int port = 8080;
    if (const char* p = std::getenv("PORT")) {
        try { port = std::stoi(p); } catch (...) {}
    }

    auto repo = std::make_shared<InMemoryTaskRepository>();
    auto strategy = std::make_shared<DueDatePriorityStrategy>();
    TaskService service(repo, strategy);
    service.create_task("Seed", "osipov", 3, "demo", 10);

    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 1;
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) return 1;
    if (listen(fd, 16) < 0) return 1;

    std::cout << "osipov TaskFlow server on " << port << '\n';

    while (true) {
        int client = accept(fd, nullptr, nullptr);
        if (client < 0) continue;

        std::string req;
        char buf[2048];
        while (true) {
            ssize_t n = recv(client, buf, sizeof(buf), 0);
            if (n <= 0) break;
            req.append(buf, static_cast<std::size_t>(n));
            auto hend = req.find("\r\n\r\n");
            if (hend == std::string::npos) continue;
            std::size_t clen = 0;
            auto cl = req.find("Content-Length:");
            if (cl != std::string::npos && cl < hend) {
                try { clen = static_cast<std::size_t>(std::stoul(req.substr(cl + 15))); }
                catch (...) { clen = 0; }
            }
            if (req.size() >= hend + 4 + clen) break;
        }
        if (req.empty()) {
            close(client);
            continue;
        }

        std::string method, path;
        {
            std::istringstream iss(req);
            iss >> method >> path;
        }

        std::string out;
        if (method == "GET" && path == "/health") {
            out = reply(200, "OK", "{\"ok\":true,\"service\":\"taskflow-osipov\"}");
        } else if (method == "GET" && path.rfind("/tasks", 0) == 0) {
            out = reply(200, "OK", list_json(service.list_by_priority()));
        } else if (method == "POST" && path == "/tasks") {
            auto pos = req.find("\r\n\r\n");
            std::string body = pos == std::string::npos ? "" : req.substr(pos + 4);
            try {
                auto t = service.create_task(
                    field_str(body, "title"),
                    field_str(body, "assignee"),
                    field_int(body, "priority", 3),
                    field_str(body, "description"),
                    field_int(body, "days_left", 7));
                out = reply(201, "Created", to_json(t));
            } catch (const std::exception& e) {
                out = reply(400, "Bad Request",
                            std::string("{\"error\":\"") + esc(e.what()) + "\"}");
            }
        } else {
            out = reply(404, "Not Found", "{\"error\":\"not found\"}");
        }

        send(client, out.data(), out.size(), 0);
        close(client);
    }
}
