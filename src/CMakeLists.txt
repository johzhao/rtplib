add_subdirectory(utils)

add_library(rtplib STATIC
        rtp_packet_parser.cpp
        rtp_stream_manager.cpp
        jitter_buffer.cpp
        sequence_extender.cpp
        payload_depacketizer/h264_depacketizer.cpp
        payload_depacketizer/h265_depacketizer.cpp
)
target_link_libraries(rtplib PUBLIC
        Threads::Threads
        spdlog::spdlog
        utils
)
