#include "gui/application.hpp"

#ifdef SH_TGT_WEB
#include <emscripten.h>
#include "nhbase/config.hpp"
#include <cstdio>

EM_JS(int, js_db_inited, (), {return Module['nh_db_inited']});
EM_JS(int, js_db_init_failed, (), { return Module['nh_db_init_failed']; });
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
                       if (err) {
                           Module['nh_db_init_failed'] = 1;
                       } else {
                           Module['nh_db_inited'] = 1;
                       }
                   });

               // Change working directory as well, for imgui saving to work
               FS.chdir(user_dir);
           }),
           NB_WEB_USER_DIR);
    // busy poll
    while (!js_db_inited() && !js_db_init_failed()) {
        emscripten_sleep(100);
    }
    if (js_db_init_failed()) {
        return 1;
    }
    std::printf("User directory loaded\n");
#endif

    int err = 0;

    // Heap allocation so that on Web we can continue to reference it.
    sh::Application *app = new sh::Application();
    if (!app->init()) {
        err = 1;
        goto l_end;
    }

    err = app->run();

#ifndef SH_EXPLICIT_RAF
    app->release();

l_end:
    if (app) {
        delete app;
    }
    return err;
#else
// Let Emscripten runtime collect resources
l_end:
    return err;
#endif
}
