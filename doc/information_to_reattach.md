# attaching to a running workflow from behind The API^tm

After starting your scoped_runtime_system, you can get a serialized information representation by constructing a `gspc::information_to_reattach (rts)` and calling `to_string()`. That string can then be used to create a `gspc::information_to_reattach` again. This information can be used for a second constructor of `gspc::client` which takes the information instead of the scoped rts:

    gspc::scoped_runtime_system const drts (...);
    gspc::information_to_reattach info (drts);
    fhg::util::write_file (state_dir / "rts", info.to_string());
    fhg::util::write_file (state_dir / "job", job_id);

    gspc::information_to_reattach info
      (fhg::util::read_file<std::string> (state_dir / "rts"));
    gspc::client client (info);
    client.wait (fhg::util::read_file<std::string> (state_dir / "job"));

(src/drts/information_to_reattach.hpp, src/drts/client.hpp)
