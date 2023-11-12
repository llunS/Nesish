#include "web_utils.hpp"

#include <emscripten.h>

namespace sh
{

void
web_sync_fs_async()
{
    /* unmount db */
    EM_ASM(({
               FS.syncfs(
                   false, function(err) {
                       if (err) {
                           console.error("Failed to sync back user directory");
                       }
                   });
           }),
           0);
}

} // namespace sh
