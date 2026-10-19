#pragma once

#include <QString>
#include <utility>

namespace monthmint {

struct ServiceResult {
    bool ok = false;
    QString error;
    qint64 id = 0;

    [[nodiscard]] static ServiceResult success(qint64 id = 0) { return {true, {}, id}; }
    [[nodiscard]] static ServiceResult failure(QString e) { return {false, std::move(e), 0}; }
};

} // namespace monthmint
