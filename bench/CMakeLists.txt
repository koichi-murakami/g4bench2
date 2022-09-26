# copy benchmark scripts to build directory

set(BENCH_SCRIPTS bench.sh bench_all.sh bench_score.sh)

file(COPY ${BENCH_SCRIPTS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

install(PROGRAMS ${BENCH_SCRIPTS} DESTINATION ${CMAKE_INSTALL_PREFIX}/bench)
