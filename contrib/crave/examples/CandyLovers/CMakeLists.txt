add_definitions(-std=c++11)

add_executable(CandyLovers sc_main.cpp verification/subscribers/jelly_bean_sb_subscriber.cpp verification/subscribers/jelly_bean_scoreboard.cpp verification/subscribers/jelly_bean_fc_subscriber.cpp)
target_link_libraries(CandyLovers ${SystemC_LIBRARIES} ${CRAVE_LIBRARIES} 
  ${UVM_SystemC_LIBRARIES} ${SystemC_LIBRARIES})