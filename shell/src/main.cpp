#include "gui/application.hpp"

#ifdef SH_TGT_WEB
#include <emscripten.h>
#include "nhbase/config.hpp"
#include <cstdio>

EM_JS(int, js_db_inited, (), {return Module['nh_db_inited']});
EM_JS(int, js_db_init_failed, (), { return Module['nh_db_init_failed']; });
EM_JS(int, js_db_uninit_done, (), { return Module['nh_db_uninit_done']; });
#endif

int
main(int, char **)
{
#ifdef SH_TGT_WEB
    /* mount db */
    EM_ASM(({
               var user_dir = UTF8ToString($0);
               // Make
               FS.mkdir(user_dir);
               // Mount
               FS.mount(IDBFS, {}, user_dir);
               // Sync from persistent storage to memory
               Module['nh_db_inited'] = 0;
               Module['nh_db_init_failed'] = 0;
               FS.syncfs(
                   true, function(err) {
                       if (err)
                       {
                           Module['nh_db_init_failed'] = 1;
                       }
                       else
                       {
                           Module['nh_db_inited'] = 1;
                       }
                   });
           }),
           NB_WEB_USER_DIR);
    // busy poll
    while (!js_db_inited() && !js_db_init_failed())
    {
        emscripten_sleep(100);
    }
    if (js_db_init_failed())
    {
        return 1;
    }
    std::printf("User directory loaded\n");
#endif

    sh::Application app;
    if (!app.init())
    {
        return 1;
    }

    int err = app.run();

    app.release();

#if defined(SH_TGT_WEB) && 0 /* Seems not working */
    /* unmount db */
    EM_ASM(({
               Module['nh_db_uninit_done'] = 0;
               FS.syncfs(
                   false, function(err) {
                       if (err)
                       {
                           console.error("Failed to sync back user directory");
                       }
                       else
                       {
                           console.log("User directory unloaded");
                       }
                       Module['nh_db_uninit_done'] = 1;
                   });
           }),
           0);
    // busy poll
    while (!js_db_uninit_done())
    {
        emscripten_sleep(100);
    }
#endif

    return err;
}
