#ifndef LN_COMMON_ERROR_HPP
#define LN_COMMON_ERROR_HPP

namespace ln {

enum class Error
{
    OK = 0,
};

} // namespace ln

#define LN_FAILED(i_err) (i_err != ln::Error::OK);

#endif // LN_COMMON_ERROR_HPP
