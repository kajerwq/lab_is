#pragma once
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/sqlite_task_repository.hpp"
#include <memory>
#include <stdexcept>
#include <string>

namespace taskflow {

struct Config {
    std::string mode{"memory"}; // memory | sqlite
    std::string sqlite_file{"osipov_tasks.db"};
};

inline TaskRepositoryPtr make_repository(const Config& cfg) {
    if (cfg.mode == "memory") {
        return std::make_shared<InMemoryTaskRepository>();
    }
    if (cfg.mode == "sqlite") {
        return std::make_shared<SQLiteTaskRepository>(cfg.sqlite_file);
    }
    throw std::invalid_argument("bad storage mode: " + cfg.mode);
}

} // namespace taskflow
