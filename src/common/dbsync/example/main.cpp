#include "dbsync.h"
#include <chrono>
#include <cstdio>
#include <ctime>
#include <iostream>

int main()
{
    const std::string sql {
        "CREATE TABLE processes(`pid` BIGINT, `name` TEXT, `path` TEXT, `cmdline` TEXT, `state` TEXT, `cwd` TEXT, "
        "`root` TEXT, `uid` BIGINT, `gid` BIGINT, `euid` BIGINT, `egid` BIGINT, `suid` BIGINT, `sgid` BIGINT, "
        "`on_disk` INTEGER, `wired_size` BIGINT, `resident_size` BIGINT, `total_size` BIGINT, `user_time` BIGINT, "
        "`system_time` BIGINT, `disk_bytes_read` BIGINT, `disk_bytes_written` BIGINT, `start_time` BIGINT, `parent` "
        "BIGINT, `pgroup` BIGINT, `threads` INTEGER, `nice` INTEGER, `is_elevated_token` INTEGER, `elapsed_time` "
        "BIGINT, `handle_count` BIGINT, `percent_processor_time` BIGINT, `upid` BIGINT HIDDEN, `uppid` BIGINT HIDDEN, "
        "`cpu_type` INTEGER HIDDEN, `cpu_subtype` INTEGER HIDDEN, `phys_footprint` BIGINT HIDDEN, PRIMARY KEY (`pid`)) "
        "WITHOUT ROWID;"};
    const std::string insert_sql {
        "{\"table\":\"processes\",\"data\":[{\"pid\":4,\"name\":\"System\",\"path\":\"\",\"cmdline\":\"\",\"state\":"
        "\"\",\"cwd\":\"\",\"root\":\"\",\"uid\":-1,\"gid\":-1,\"euid\":-1,\"egid\":-1,\"suid\":-1,\"sgid\":-1,\"on_"
        "disk\":-1,\"wired_size\":-1,\"resident_size\":-1,\"total_size\":-1,\"user_time\":-1,\"system_time\":-1,\"disk_"
        "bytes_read\":-1,\"disk_bytes_written\":-1,\"start_time\":-1,\"parent\":0,\"pgroup\":-1,\"threads\":164,"
        "\"nice\":-1,\"is_elevated_token\":false,\"elapsed_time\":-1,\"handle_count\":-1,\"percent_processor_time\":-1}"
        ",{\"pid\":5,\"name\":\"System\",\"path\":\"\",\"cmdline\":\"\",\"state\":\"\",\"cwd\":\"\",\"root\":\"\","
        "\"uid\":-1,\"gid\":-1,\"euid\":-1,\"egid\":-1,\"suid\":-1,\"sgid\":-1,\"on_disk\":-1,\"wired_size\":-1,"
        "\"resident_size\":-1,\"total_size\":-1,\"user_time\":-1,\"system_time\":-1,\"disk_bytes_read\":-1,\"disk_"
        "bytes_written\":-1,\"start_time\":-1,\"parent\":0,\"pgroup\":-1,\"threads\":164,\"nice\":-1,\"is_elevated_"
        "token\":false,\"elapsed_time\":-1,\"handle_count\":-1,\"percent_processor_time\":-1},{\"pid\":6,\"name\":"
        "\"System\",\"path\":\"\",\"cmdline\":\"\",\"state\":\"\",\"cwd\":\"\",\"root\":\"\",\"uid\":-1,\"gid\":-1,"
        "\"euid\":-1,\"egid\":-1,\"suid\":-1,\"sgid\":-1,\"on_disk\":-1,\"wired_size\":-1,\"resident_size\":-1,\"total_"
        "size\":-1,\"user_time\":-1,\"system_time\":-1,\"disk_bytes_read\":-1,\"disk_bytes_written\":-1,\"start_time\":"
        "-1,\"parent\":0,\"pgroup\":-1,\"threads\":164,\"nice\":-1,\"is_elevated_token\":false,\"elapsed_time\":-1,"
        "\"handle_count\":-1,\"percent_processor_time\":-1}]}"};
    const std::string update_sql {
        "{\"table\":\"processes\",\"data\":[{\"pid\":4,\"name\":\"Systemsss\",\"path\":\"/var/"
        "etc\",\"cmdline\":\"44\",\"state\":\"\",\"cwd\":\"aa\",\"root\":\"\",\"uid\":-1,\"gid\":-1,\"euid\":-1,"
        "\"egid\":-1,\"suid\":-1,\"sgid\":-1,\"on_disk\":-1,\"wired_size\":-1,\"resident_size\":-1,\"total_size\":-1,"
        "\"user_time\":-1,\"system_time\":-1,\"disk_bytes_read\":-1,\"disk_bytes_written\":-1,\"start_time\":-1,"
        "\"parent\":0,\"pgroup\":-1,\"threads\":164,\"nice\":-1,\"is_elevated_token\":false,\"elapsed_time\":-1,"
        "\"handle_count\":-1,\"percent_processor_time\":-1},{\"pid\":53,\"name\":\"System\",\"path\":\"\",\"cmdline\":"
        "\"\",\"state\":\"\",\"cwd\":\"\",\"root\":\"\",\"uid\":-1,\"gid\":-1,\"euid\":-1,\"egid\":-1,\"suid\":-1,"
        "\"sgid\":-1,\"on_disk\":-1,\"wired_size\":-1,\"resident_size\":-1,\"total_size\":-1,\"user_time\":-1,\"system_"
        "time\":-1,\"disk_bytes_read\":-1,\"disk_bytes_written\":-1,\"start_time\":-1,\"parent\":0,\"pgroup\":-1,"
        "\"threads\":164,\"nice\":-1,\"is_elevated_token\":false,\"elapsed_time\":-1,\"handle_count\":-1,\"percent_"
        "processor_time\":-1},{\"pid\":5,\"name\":\"Systemaa\",\"path\":\"\",\"cmdline\":\"\",\"state\":\"\",\"cwd\":"
        "\"\",\"root\":\"\",\"uid\":-1,\"gid\":-1,\"euid\":-1,\"egid\":-1,\"suid\":-1,\"sgid\":-1,\"on_disk\":-1,"
        "\"wired_size\":-1,\"resident_size\":-1,\"total_size\":-1,\"user_time\":-1,\"system_time\":-1,\"disk_bytes_"
        "read\":-1,\"disk_bytes_written\":-1,\"start_time\":-1,\"parent\":0,\"pgroup\":-1,\"threads\":164,\"nice\":-1,"
        "\"is_elevated_token\":false,\"elapsed_time\":-1,\"handle_count\":-1,\"percent_processor_time\":-1}]}]}"};
    cJSON* json_insert {cJSON_Parse(insert_sql.c_str())};
    cJSON* json_update {cJSON_Parse(update_sql.c_str())};
    cJSON* json_result {nullptr};
    auto handle {dbsync_create(HostType::AGENT, DbEngineType::SQLITE3, "temp.db", sql.c_str())};

    if (nullptr != handle)
    {
        if (0 == dbsync_insert_data(handle, json_insert))
        {
            do
            {
                auto t_start {std::chrono::high_resolution_clock::now()};

                if (0 == dbsync_update_with_snapshot(handle, json_update, &json_result))
                {
                    auto t_end {std::chrono::high_resolution_clock::now()};
                    std::cout << "duration: "
                              << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count() << '\n';
                    char* result_print = cJSON_Print(json_result);
                    std::cout << result_print << '\n';
                    cJSON_free(result_print);
                    dbsync_free_result(&json_result);
                }
                else
                {
                    std::cout << "Error updating with snapshot." << '\n';
                }
            } while (getc(stdin) != 'q');
        }

        dbsync_teardown();
    }

    cJSON_Delete(json_update);
    cJSON_Delete(json_insert);

    return 0;
}
