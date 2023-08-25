#pragma once


struct D2HModule {
    const char * name;
    int (*install)();
    void (*uninstall)();

    void (*onLoad)();         // Call when d2 win shows
    void (*reload)();
};


int module_init();
void module_destroy();
void module_on_load();
