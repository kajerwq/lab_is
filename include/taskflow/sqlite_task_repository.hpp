#pragma once
#include "taskflow/repository.hpp"
#include <stdexcept>
#include <string>

namespace taskflow {

// ЛР2: учебная заглушка адаптера SQLite
class SQLiteTaskRepository final : public ITaskRepository {
public:
    explicit SQLiteTaskRepository(std::string path) : path_(std::move(path)) {}

    Task create(Task) override { fail(); }
    std::optional<Task> find_by_id(std::int64_t) const override { fail(); }
    std::vector<Task> list() const override { fail(); }
    bool update(const Task&) override { fail(); }

    const std::string& path() const { return path_; }

private:
    std::string path_;
    [[noreturn]] void fail() const {
        throw std::runtime_error("SQLite adapter stub, path=" + path_);
    }
};

} // namespace taskflow
