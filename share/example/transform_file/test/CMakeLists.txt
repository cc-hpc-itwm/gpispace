# mirko.rahn@itwm.fraunhofer.de

add_executable (transform_file_generator gen.cpp)

fhg_add_application_test (example_transform_file_we-exec SCRIPT test.we-exec)
