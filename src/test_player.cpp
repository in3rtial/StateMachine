#include "fsm.h"

#include <cassert>
#include <iostream>
#include <string>

class player: public fsmlite::fsm<player> {
    friend class fsmlite::fsm<player>;  // base class needs access to transition_table

    std::string cd_title;
    bool autoplay = false;

public:
    enum states { Stopped, Open, Empty, Playing, Paused };

    player(state_type init_state = Empty) : fsm(init_state) { }

    void set_autoplay(bool f) { autoplay = f; }

    bool is_autoplay() const { return autoplay; }

    const std::string& get_cd_title() const { return cd_title; }

    struct play {};
    struct open_close {};
    struct cd_detected {
        std::string title;
        bool bad() const { return title.empty(); }
    };
    struct stop {};
    struct pause {};

private:
    void start_playback();
    void start_autoplay(const cd_detected& cd);
    void open_drawer();
    void close_drawer();
    void store_cd_info(const cd_detected& cd);
    void stop_playback();
    void pause_playback();
    void resume_playback();
    void stop_and_open();

private:
    using m = player;  // for brevity

    using transition_table = table<
//       Start    Event        Target   Action              Guard (optional)
//  ----+--------+------------+--------+-------------------+-----------------+-
    row< Stopped, play,        Playing, &m::start_playback                    >,
    row< Stopped, open_close,  Open,    &m::open_drawer                       >,
    row< Open,    open_close,  Empty,   &m::close_drawer                      >,
    row< Empty,   open_close,  Open,    &m::open_drawer                       >,
    row< Empty,   cd_detected, Open,    &m::open_drawer,    &cd_detected::bad >,
    row< Empty,   cd_detected, Playing, &m::start_autoplay, &m::is_autoplay   >,
    row< Empty,   cd_detected, Stopped, &m::store_cd_info   /* fallback */    >,
    row< Playing, stop,        Stopped, &m::stop_playback                     >,
    row< Playing, pause,       Paused,  &m::pause_playback                    >,
    row< Playing, open_close,  Open,    &m::stop_and_open                     >,
    row< Paused,  play,        Playing, &m::resume_playback                   >,
    row< Paused,  stop,        Stopped, &m::stop_playback                     >,
    row< Paused,  open_close,  Open,    &m::stop_and_open                     >
//  ----+--------+------------+--------+-------------------+-----------------+-
    >;
};

void player::start_playback()
{
    std::cout << "Starting playback\n";
}

void player::start_autoplay(const cd_detected& cd)
{
    std::cout << "Starting playback of '" << cd.title << "'\n";
    cd_title = cd.title;
}

void player::open_drawer()
{
    std::cout << "Opening drawer\n";
    cd_title.clear();
}

void player::close_drawer()
{
    std::cout << "Closing drawer\n";
}

void player::store_cd_info(const cd_detected& cd)
{
    std::cout << "Detected CD '" << cd.title << "'\n";
    cd_title = cd.title;
}

void player::stop_playback()
{
    std::cout << "Stopping playback\n";
}

void player::pause_playback()
{
    std::cout << "Pausing playback\n";
}

void player::stop_and_open()
{
    std::cout << "Stopping and opening drawer\n";
    cd_title.clear();
}

void player::resume_playback()
{
    std::cout << "Resuming playback\n";
}

void test_player()
{
    player p;
    assert(p.current_state() == player::Empty);
    assert(!p.is_autoplay());
    assert(p.get_cd_title().empty());
    p.process_event(player::open_close());
    assert(p.current_state() == player::Open);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Empty);
    p.process_event(player::cd_detected{"louie, louie"});
    assert(p.current_state() == player::Stopped);
    assert(p.get_cd_title() == "louie, louie");
    p.process_event(player::play());
    assert(p.current_state() == player::Playing);
    p.process_event(player::pause());
    assert(p.current_state() == player::Paused);
    p.process_event(player::play());
    assert(p.current_state() == player::Playing);
    p.process_event(player::stop());
    assert(p.current_state() == player::Stopped);
    p.process_event(player::play());
    assert(p.current_state() == player::Playing);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Open);
    assert(p.get_cd_title().empty());
    p.process_event(player::open_close());
    assert(p.current_state() == player::Empty);
    assert(p.get_cd_title().empty());
    p.process_event(player::play());
    assert(p.current_state() == player::Empty);
    assert(p.get_cd_title().empty());
}

void test_bad_cd()
{
    player p;
    assert(p.current_state() == player::Empty);
    assert(!p.is_autoplay());
    assert(p.get_cd_title().empty());
    p.process_event(player::open_close());
    assert(p.current_state() == player::Open);
    assert(p.get_cd_title().empty());
    p.process_event(player::open_close());
    assert(p.current_state() == player::Empty);
    assert(p.get_cd_title().empty());
    p.process_event(player::cd_detected{});
    assert(p.current_state() == player::Open);
    assert(p.get_cd_title().empty());
}

void test_autoplay()
{
    player p;
    p.set_autoplay(true);
    assert(p.current_state() == player::Empty);
    assert(p.is_autoplay());
    assert(p.get_cd_title().empty());
    p.process_event(player::open_close());
    assert(p.current_state() == player::Open);
    assert(p.get_cd_title().empty());
    p.process_event(player::open_close());
    assert(p.current_state() == player::Empty);
    assert(p.get_cd_title().empty());
    p.process_event(player::cd_detected{"louie, louie"});
    assert(p.current_state() == player::Playing);
    assert(p.get_cd_title() == "louie, louie");
}

int main()
{
    test_player();
    test_bad_cd();
    test_autoplay();
    return 0;
}
