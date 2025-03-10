CREATE TABLE scan_task (
    `task_id`                                               VARCHAR(255)                        NOT NULL,               -- 任务ID
    `task_name`                                             TEXT                                NOT NULL,               -- 任务名称
    `scan_task_dir`                                         TEXT                                NOT NULL,               -- 扫描文件夹，以 {] 分割
    `scan_task_dir_exception`                               TEXT                                NOT NULL,               -- 扫描例外文件夹，以 {] 分割
    `scan_task_file_ext`                                    TEXT                                NOT NULL,               -- 扫描文件类型，以 {] 分割
    `scan_task_file_ext_exception`                          TEXT                                NOT NULL,               -- 扫描例外文件类型，以 {] 分割
    `start_time`                                            INTEGER             DEFAULT 0       NOT NULL,               -- 扫描开始时间
    `stop_time`                                             INTEGER             DEFAULT 0       NOT NULL,               -- 扫描结束时间
    `total_file`                                            INTEGER             DEFAULT 0       NOT NULL,               -- 需要扫描文件总数
    `finished_file`                                         INTEGER             DEFAULT 0       NOT NULL,               -- 已经扫描完成的文件数
    `task_status`                                           TINYINT             DEFAULT 0       NOT NULL,               -- 任务状态： 0 - 未开始，1 - 扫描中， 2 - 已停止， 3 - 已完成， 4 - 已暂停， 5 - 扫描发生错误
    `scan_mode`                                             TINYINT             DEFAULT 0       NOT NULL,               -- 扫描模式： 0 - 默认模式， 1 - 免打扰模式，2 - 快速模式
    `is_first`                                              TINYINT             DEFAULT 0       NOT NULL,               -- 是否第一次扫描：0 - 不是， 1 - 是
    PRIMARY KEY (task_id)
);

CREATE TABLE scan_result (
    `file_path`                                             TEXT                                NOT NULL,               -- 文件绝对路径
    `file_md5`                                              TEXT                                NOT NULL,               -- 文件内容MD5
    `policy_id`                                             TEXT                DEFAULT ''      NOT NULL,               -- 命中的策略，以 {] 分割
    `scan_finished_time`                                    INTEGER             DEFAULT 0       NOT NULL,               -- 扫描结束时间
    `file_type`                                             VARCHAR             DEFAULT ''      NOT NULL,               -- 文件类型
    `file_ext_name`                                         VARCHAR                             NOT NULL,               -- 文件扩展名
    `file_size`                                             INTEGER                             NOT NULL,               -- 文件大小
    UNIQUE (file_path)
);

-- 每个任务一张表
CREATE TABLE task_id (
    `file_path`                                             TEXT                                NOT NULL,               -- 文件绝对路径
    `file_md5`                                              TEXT                                NOT NULL,               -- 文件内容 MD5
    `is_finished`                                           TINYINT             DEFAULT 0       NOT NULL,               -- 是否完成扫描：0 - 没完成，1 - 完成
    PRIMARY KEY (file_path)
);