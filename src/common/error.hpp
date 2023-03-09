#pragma once

namespace nh {

enum class Error {
    OK = 0,

    UNINITIALIZED,    // Uninitialized.
    INVALID_ARGUMENT, // Invalid argument.
    CORRUPTED,        // Invalid or corrupted resource.
    UNAVAILABLE,      // Requested resource is unavailable.
    UNIMPLEMENTED,    // TODO marker, shouldn't be exposed to production code.
    PROGRAMMING,      // Someone wrote a bug, an unexpected behavior.
    READ_ONLY,        // Attempted to write to read only area.
    WRITE_ONLY,       // Attempted to read from write only area.

    RENDERING_API, // Rendering API error.
};

} // namespace nh

#define LN_FAILED(i_err) ((i_err) != nh::Error::OK)
