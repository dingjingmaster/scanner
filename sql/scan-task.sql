CREATE TABLE scan_task (
    `task_id`                                               VARCHAR(255)                        NOT NULL,               -- 任务ID
    `task_name`                                             TEXT                                NOT NULL,               -- 任务名称
    `scan_task_dir`                                         TEXT                                NOT NULL,               -- 扫描文件夹，以 0x01 分割
    `scan_task_dir_exception`                               TEXT                                NOT NULL,               -- 扫描例外文件夹，以 0x01 分割
    `scan_task_file_ext`                                    TEXT                                NOT NULL,               -- 扫描文件类型，以 0x01 分割
    `scan_task_file_ext_exception`                          TEXT                                NOT NULL,               -- 扫描例外文件类型，以 0x01 分割
    `policy_ids`                                            TEXT                                NOT NULL,               -- 策略 IDs
    `start_time`                                            INTEGER             DEFAULT 0       NOT NULL,               -- 扫描开始时间
    `stop_time`                                             INTEGER             DEFAULT 0       NOT NULL,               -- 扫描结束时间
    `total_file`                                            INTEGER             DEFAULT 0       NOT NULL,               -- 需要扫描文件总数
    `finished_file`                                         INTEGER             DEFAULT 0       NOT NULL,               -- 已经扫描完成的文件数
    `is_scheduled`                                          TINYINT             DEFAULT 0       NOT NULL,               -- 是否调度： 0 - 非调度，1 - 调度
    `times`                                                 INT                 DEFAULT 0       NOT NULL,               -- 扫描伦次，服务器启动了几次
    `scheduling_cron`                                       VARCHAR             DEFAULT ''      NOT NULL,               -- 定时任务
    `task_status`                                           TINYINT             DEFAULT 0       NOT NULL,               -- 任务状态： 0 - 未开始，1 - 扫描中， 2 - 已停止， 3 - 已完成， 4 - 已暂停， 5 - 扫描发生错误
    `scan_mode`                                             TINYINT             DEFAULT 0       NOT NULL,               -- 扫描模式： 0 - 默认模式， 1 - 免打扰模式，2 - 快速模式
    `round`                                                 INT                 DEFAULT 0       NOT NULL,               -- 第几次扫描
    PRIMARY KEY (task_id)
);

CREATE TABLE scan_result (
    `file_path`                                             TEXT                                NOT NULL,               -- 文件绝对路径
    `file_md5`                                              TEXT                                NOT NULL,               -- 文件内容MD5
    `policy_id`                                             TEXT                DEFAULT ''      NOT NULL,               -- 命中的策略, 以 0x01 分割
    `scan_finished_time`                                    INTEGER             DEFAULT 0       NOT NULL,               -- 扫描结束时间
    `file_type`                                             TEXT                DEFAULT ''      NOT NULL,               -- 文件类型
    `file_ext_name`                                         VARCHAR                             NOT NULL,               -- 文件扩展名
    `file_size`                                             INTEGER                             NOT NULL,               -- 文件大小
    `content`                                               TEXT                DEFAULT ''      NOT NULL,               -- 敏感上下文, 要包含 policy_id 0x01 key 0x01 content 0x02
    PRIMARY KEY (file_path)
);

CREATE TABLE policy_id (
    `policy_id`                                             VARCHAR                             NOT NULL,               -- 策略ID
    `is_checked`                                            TINYINT             DEFAULT 1       NOT NULL,               -- 是否检查过：0 - 不存在，1 - 存在
    `dirty`                                                 TINYINT             DEFAULT 0       NOT NULL,               --
    PRIMARY KEY (policy_id)
);

-- 每个任务一张表
-- CREATE TABLE task_id (
--     `file_path`                                             TEXT                                NOT NULL,               -- 文件绝对路径
--     `file_md5`                                              TEXT                                NOT NULL,               -- 文件内容 MD5
--     `is_finished`                                           TINYINT             DEFAULT 0       NOT NULL,               -- 是否完成扫描：0 - 没完成，1 - 完成
--     PRIMARY KEY (file_path)
-- );