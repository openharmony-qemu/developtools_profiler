# Sql 分析和 Metrics 说明

Sql 功能是方便使用者查询 sql 语句查看相关业务，Metrics 是更高级别的查询接口，无需手动键入任何 SQL 语句，只需要选择定制好的查询接口，就能获得想要跟踪的结果。

## Sql 分析功能介绍

点击 Query(SQL)，输入需要查询的 sql 语句，如 select \* from process，可以看到进程表数据。
![GitHub Logo](../../figures/Metrics/Sql.jpg)

## Metrics 功能介绍

Metrics 是更高级别的查询接口，无需手动键入任何 SQL 语句，只需要选择定制好的查询接口，就能获得想要跟踪的结果。

### Metrics 查询接口展示

如下图，查询接口在下拉框中，如选择 trace_task_names，点击 run，就能展示线程和进程相关数据。
![GitHub Logo](../../figures/Metrics/metrics.jpg)

## Info 和 stats 功能

点击 Info and stats，能查看到 meta 表和 stats 表信息。
![GitHub Logo](../../figures/Metrics/infoandstats.jpg)

## Download 功能

点击 DownLoad 按钮，会将在线抓取的文件下载到本地。
![GitHub Logo](../../figures/Metrics/download.jpg)
