function(rerun_watch)
    set_property(
        DIRECTORY
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS ${ARGV}
    )
endfunction()
