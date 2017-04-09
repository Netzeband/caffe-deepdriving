# function to create a tool target
function(compile_tool Name Source)
  
  # caffe target already exits
  if(Name MATCHES "caffe")
    set(Name ${Name}.bin)
  endif()

  message(STATUS "Build tool ${Name}")
  
  foreach(File ${Source})
    message(STATUS " * ${File}")
  endforeach(File)

  # target
  add_executable(${Name} ${Source})
  target_link_libraries(${Name} ${Caffe_LINK})
  caffe_default_properties(${Name})

  # set back RUNTIME_OUTPUT_DIRECTORY
  caffe_set_runtime_directory(${Name} "${PROJECT_BINARY_DIR}/tools")
  caffe_set_solution_folder(${Name} tools)

  # restore output name without suffix
  if(Name MATCHES "caffe.bin")
    set_target_properties(${Name} PROPERTIES OUTPUT_NAME caffe)
  endif()

  # Install
  install(TARGETS ${Name} DESTINATION bin)  
endfunction(compile_tool)

compile_tool(caffe caffe.cpp)
compile_tool(compute_image_mean compute_image_mean.cpp)
compile_tool(convert_imageset convert_imageset.cpp)
compile_tool(device_query device_query.cpp)
compile_tool(extract_features extract_features.cpp)
compile_tool(finetune_net finetune_net.cpp)
compile_tool(net_speed_benchmark net_speed_benchmark.cpp)
compile_tool(test_net test_net.cpp)
compile_tool(train_net train_net.cpp)
compile_tool(upgrade_net_proto_binary upgrade_net_proto_binary.cpp)
compile_tool(upgrade_net_proto_text upgrade_net_proto_text.cpp)
compile_tool(upgrade_solver_proto_text upgrade_solver_proto_text.cpp)

# Torcs tools are here
set(torcs_library_source
  torcs/Arguments.cpp
  torcs/Database.cpp
  torcs/Indicators.cpp
  torcs/Semantic.cpp
  torcs/Image.cpp
  torcs/SharedMemory.cpp
  torcs/DriveController.cpp
  torcs/NeuralNet.cpp
  torcs/ErrorMeasurement.cpp
)

set(torcs_visualize_database_source
  torcs_visualize_database.cpp
  ${torcs_library_source}
)
compile_tool(torcs_visualize_database "${torcs_visualize_database_source}")

set(torcs_run_source
  torcs_run.cpp
  ${torcs_library_source}
)
compile_tool(torcs_run "${torcs_run_source}")

set(torcs_verify_source
  torcs_verify.cpp
  ${torcs_library_source}
)
compile_tool(torcs_verify "${torcs_verify_source}")

set(torcs_verify_fast_source
  torcs_verify_fast.cpp
  ${torcs_library_source}
)
compile_tool(torcs_verify_fast "${torcs_verify_fast_source}")

set(torcs_record_source
  torcs_record.cpp
  ${torcs_library_source}
)
compile_tool(torcs_record "${torcs_record_source}")


