## The Installation object in the RTS API was removed
In order to simplify usage of the API, GPI-Space now automatically
determines the installation location, which frees the user from
providing a "gspc-home"/`gspc::installation` when starting a runtime
system.

This is an API change, requiring applications to

- Remove the construction of a `gspc::installation` object.
- Remove the `installation` argument from `gspc::scoped_rifd` or
  `gspc::rifds` constructors.
- Remove the `installation` argument from
  `gspc::scoped_runtime_system` constructors.

A diff usually looks like

```diff
-  gspc::installation const gspc_installation (GPISPACE_INSTALL_DIR);
-
```

or

```diff
-  gspc::installation const gspc_installation (vm);
-
```

and

```diff
   gspc::scoped_rifds const rifds
     ( gspc::rifd::strategy {vm}
     , gspc::rifd::hostnames {vm}
     , gspc::rifd::port {vm}
-    , gspc_installation
     );

   gspc::scoped_runtime_system drts
     ( vm
-    , gspc_installation
     , topology_description
     , rifds.entry_points()
     );
```

but might slightly differ based on the overloads used or the method
used to determine the GPI-Space installation directory.
