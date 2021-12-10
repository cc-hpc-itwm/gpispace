# Information to Reattach

GPI-Space allows sharing the distributed runtime system (DRTS) between multiple programs over the network.
In order to achieve this, reattachment information can be extracted from the DRTS and converted into a string.
The extracted string can then be shared with other programs via a variety of different technologies (e.g. database, NFS, ...).
Applications using a shared DRTS are free to launch their own workflows or they can even interact with an already running job started elsewhere.

> ---
> **NOTE:**
>
> There are no guarantees that interactions with a remote DRTS will finish.
> Remote DRTS won't communicate any state information with connected clients.
>
> ---

## Usage

After starting your DRTS (`gspc::scoped_runtime_system`), the serialized information representation is gained by constructing a `gspc::information_to_reattach (drts)` and calling `to_string()`.
That string is then stored so it can be shared with other applications.
The code snippet below demonstrates what this would look like with a shared filesystem approach:

```c++
gspc::scoped_runtime_system const drts (...);
gspc::information_to_reattach info (drts);
std::ofstream shared_drts ("shared/drts.info");
shared_drts << info.to_string() << std::endl;
shared_drts.close();
```

Another application can now acquire the serialized information string and use it to create a `gspc::information_to_reattach` object.
That object is then passed to the second constructor of `gspc::client` to interface with the remote DRTS.
The extract below shows this for the shared filesystem approach used above:

```c++
std::string info_string;
std::ifstream shared_drts ("shared/drts.info");
std::getline (shared_drts, info_string).close();

gspc::information_to_reattach info (info_string);
gspc::client client (info);
client.put_and_run (...);
```
