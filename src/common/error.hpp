#ifndef LN_COMMON_ERROR_HPP
#define LN_COMMON_ERROR_HPP

namespace ln {

enum class Error
{
    OK = 0,

    INVALID_ARGUMENT,
    CORRUPTED,     // Invalid or corrupted data.
    UNAVAILABLE,   // Requested resource is unavailable.
    UNIMPLEMENTED, // TODO marker, shouldn't be exposed to production code.
    PROGRAMMING,   // Someone wrote a bug, an unexpected behavior.
};

} // namespace ln

#define LN_FAILED(i_err) ((i_err) != ln::Error::OK)

#endif // LN_COMMON_ERROR_HPP
