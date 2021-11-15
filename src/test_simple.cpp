#include "fsm.h"
#include <iostream>
#include <cassert>

class simpleMachine: public fsmlite::fsm<simpleMachine> {
    friend class fsmlite::fsm<simpleMachine>;

public:
    enum states { On, Off };
    explicit simpleMachine(state_type init_state)
    : fsm(init_state)
    {
    }

    struct start{ };
    struct stop { };
    struct noOp { };
protected:
    template<class Event>
    state_type no_transition(const Event&) {
        std::cerr << "Detected unknown transition " << typeid(Event).name() << std::endl;
        return current_state();
    }

private:
    using m = simpleMachine;  // for brevity
    void onStart() { std::cout << "Entered start state" << std::endl; }
    void onStop() { std::cout << "Entered stop state" << std::endl; }

    using transition_table = table<
//       Start    Event        Target   Action              Guard (optional)
//  ----+--------+------------+--------+-------------------+-----------------+-
    row< On,      stop,        Off,     &m::onStop                            >,
    row< Off,     start,       On,      &m::onStart                           >
//  ----+--------+------------+--------+-------------------+-----------------+-
    >;
};


struct unknownEvent{};

int main() {
    simpleMachine m { simpleMachine::Off };
    assert(m.current_state() == simpleMachine::Off);

    m.process_event(simpleMachine::start());
    assert(m.current_state() == simpleMachine::On);

    m.process_event(simpleMachine::noOp());
    assert(m.current_state() == simpleMachine::On);

    m.process_event(simpleMachine::stop());
    assert(m.current_state() == simpleMachine::Off);

    m.process_event(simpleMachine::stop());
    m.process_event(simpleMachine::stop());

    m.process_event(unknownEvent());
    return 0;
}

