# Hiperf目录文件说明
TraceStreamer支持hiperf数据的解析，引用了openharmony原生的hiperf，但由于TraceStreamer是独立编译，需要使用独立的BUILD.gn，本目录下的BUILD.gn会用来替换下载的hiperf目录下的BUILD.gn。   
所有的操作，在代码的根目录下执行pare_third_party.sh会自动完成。  
TraceStreamer使用的perf文件列表如下： 
```
├── include
│   ├── callstack.h
│   ├── debug_logger.h
│   ├── dwarf_encoding.h
│   ├── elf_parser.h
│   ├── hashlist.h
│   ├── hashlist.hpp
│   ├── linux
│   │   ├── types.h
│   ├── mem_map_item.h
│   ├── noncopyable.h
│   ├── option.h
│   ├── perf_event_record.h
│   ├── perf_events.h
│   ├── perf_file_format.h
│   ├── perf_file_reader.h
│   ├── perf_record_format.h
│   ├── register.h
│   ├── report.h
│   ├── ring_buffer.h
│   ├── subcommand_dump.h
│   ├── subcommand.h
│   ├── subcommand_help.h
│   ├── symbols_file.h
│   ├── tracked_command.h
│   ├── utilities.h
│   ├── virtual_runtime.h
│   └── virtual_thread.h
└── src
    ├── callstack.cpp
    ├── debug_logger.cpp
    ├── dwarf_encoding.cpp
    ├── elf_symbol.cpp
    ├── option.cpp
    ├── perf_event_record.cpp
    ├── perf_file_format.cpp
    ├── perf_file_reader.cpp
    ├── program_header.cpp
    ├── register.cpp
    ├── report.cpp
    ├── section_header.cpp
    ├── subcommand.cpp
    ├── subcommand_help.cpp
    ├── symbols_file.cpp
    ├── utilities.cpp
    ├── virtual_runtime.cpp
    └── virtual_thread.cpp
```