USE mysql_review;
-- 只读观察。各值会随运行状态变化；不修改全局配置。
SELECT VERSION(), @@transaction_isolation, @@autocommit;
SHOW VARIABLES WHERE Variable_name IN (
    'innodb_page_size', 'innodb_buffer_pool_size',
    'innodb_flush_log_at_trx_commit', 'sync_binlog', 'log_bin'
);
SHOW GLOBAL STATUS WHERE Variable_name IN (
    'Innodb_buffer_pool_read_requests', 'Innodb_buffer_pool_reads',
    'Innodb_buffer_pool_pages_dirty', 'Innodb_os_log_written'
);

-- 锁实验中，从第三个终端执行此文件。
SELECT ENGINE_TRANSACTION_ID, OBJECT_SCHEMA, OBJECT_NAME,
       INDEX_NAME, LOCK_TYPE, LOCK_MODE, LOCK_STATUS, LOCK_DATA
FROM performance_schema.data_locks
WHERE OBJECT_SCHEMA = 'mysql_review';
SELECT REQUESTING_ENGINE_TRANSACTION_ID, BLOCKING_ENGINE_TRANSACTION_ID
FROM performance_schema.data_lock_waits;
SHOW ENGINE INNODB STATUS;
