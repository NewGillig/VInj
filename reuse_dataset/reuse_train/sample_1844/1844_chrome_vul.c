int main(int argc, char **argv)
{
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    global_base = event_init();
    test_registerfds();
    test_evutil_strtoll();
    test_priorities(1);
    test_priorities(2);
    test_priorities(3);
    test_evbuffer();
    test_evbuffer_find();
    test_bufferevent();
    test_bufferevent_watermarks();
    test_free_active_base();
    test_event_base_new();
    http_suite();
    test_simplewrite();
    test_multiple();
    test_persistent();
    test_combined();
    test_simpletimeout();
    test_multiplesignal();
    test_immediatesignal();
    test_loopbreak();
    test_loopexit_multiple();
    test_multiple_events_for_same_fd();
    test_want_only_once();
    evtag_test();
    test_signal_dealloc();
    test_signal_pipeloss();
    test_signal_switchbase();
    test_signal_restore();
    test_signal_assert();
    test_signal_while_processing();
}