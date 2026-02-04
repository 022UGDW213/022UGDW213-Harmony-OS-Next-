# Windows NT Features Implementation

## Overview
Implemented the **NT Object Manager**, the central resource management subsystem of the Windows NT kernel. Unlike Unix which treats everything as a file, NT treats everything as an **Object** related by a uniform object header and managed via **Handles**.

## Components

### [Object Manager](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/nt/ob_manager.c)
- **Concept**: Unified header (`object_header_t`) for all kernel resources.
- **Reference Counting**: Objects stay alive as long as `ref_count > 0`.
- **API**:
    - `ob_create_object()`: Allocates Header + Body.
    - `ob_reference_object()`: Increments ref count.
    - `ob_dereference_object()`: Decrements ref count, frees if 0.

### [Handle Table](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/nt/handle_table.c)
- **Concept**: Process-specific table mapping integer `HANDLE`s to Object Pointers.
- **Security**: Handles represent capabilities (though permissions are simplified here).
- **API**:
    - `ob_create_handle(obj)`: Adds entry, refs object.
    - `ob_close_handle(h)`: Removes entry, derefs object.

### [Registry](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/nt/registry.c)
- **Concept**: Hierarchical configuration database.
- **Keys**: Implemented as NT Objects (`OB_TYPE_KEY`).
- **Values**: Simple Key-Value store within key objects.
- **API**: `nt_create_key`, `nt_set_value_key`, `nt_query_value_key`.

### [Executive Services](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/nt/executive.c)
- **Concept**: The "Native API" (`Nt...`) exposing kernel services.
- **Implementation**: Wrappers around Object Manager and Subsystems.
- **Functions**: `NtCreateFile`, `NtCreateKey`, `NtClose`, etc.

## Verification
`test_nt` validates:
1.  **Lifecycle**: Validated Init(1) -> Handle(2) -> Lookup(3) -> Close(1) -> Deref(0). -- ✅ PASS
2.  **Lookup**: `ob_reference_object_by_handle` securely resolves handles. -- ✅ PASS

`test_nt_exec` validates:
1.  **Registry**: Key creation, value setting/querying. -- ✅ PASS
2.  **Exec API**: API wrapper correctness. -- ✅ PASS

