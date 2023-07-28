#include "terminal.h"

#include <sstream>
#include "session_context.h"
#include "dir_node.h"
#include "func_node.h"

using namespace std;

namespace {
    const string MOVE_LEFT_KEY("\033[D");
    const string MOVE_RIGHT_KEY("\033[C");
    const size_t HISTORY_MAX_SIZE(20);
}

void Terminal::onChar(SessionContext *s, char ch)
{
    if (s->cursor == s->curr_input.size())
        s->curr_input.push_back(ch);
    else
        s->curr_input.insert(s->cursor, 1, ch);
    s->cursor++;

    if (s->options & kEnableEcho) {
        s->send(&ch);

        stringstream ss;
        ss  << s->curr_input.substr(s->cursor)
            << string((s->curr_input.size() - s->cursor), '\b');

        auto refresh_str = ss.str();
        if (!refresh_str.empty())
            s->send(refresh_str.c_str());
    }
}

// void Terminal::onEnterKey(SessionContext *s)
// {
//     if (s->options & kEnableEcho)
//         s->send("\r\n");

//     if (execute(s)) {
//         s->history.push_back(s->curr_input);
//         if (s->history.size() > HISTORY_MAX_SIZE)
//             s->history.pop_front();
//     }

//     printPrompt(s);

//     s->curr_input.clear();
//     s->cursor = 0;
//     s->history_index = 0;
// }

void Terminal::onBackspaceKey(SessionContext *s)
{
    if (s->cursor == 0)
        return;

    if (s->cursor == s->curr_input.size())
        s->curr_input.pop_back();
    else
        s->curr_input.erase((s->cursor - 1), 1);

    s->cursor--;

    if (s->options & kEnableEcho) {
        stringstream ss;
        ss  << '\b' << s->curr_input.substr(s->cursor) << ' '
            << string((s->curr_input.size() - s->cursor + 1), '\b');

        s->send(ss.str());
    }
}

void Terminal::onDeleteKey(SessionContext *s)
{
    if (s->cursor >= s->curr_input.size())
        return;

    s->curr_input.erase((s->cursor), 1);

    if (s->options & kEnableEcho) {
        stringstream ss;
        ss  << s->curr_input.substr(s->cursor) << ' '
            << string((s->curr_input.size() - s->cursor + 1), '\b');

        s->send(ss.str());
    }
}

void Terminal::onTabKey(SessionContext *s)
{
    //后面做
    (void)s;
}

namespace {
void CleanupInput(SessionContext *s)
{
    while (s->cursor < s->curr_input.size()) {
        s->send(MOVE_RIGHT_KEY);
        s->cursor++;
    }

    while (s->cursor--)
        s->send("\b \b");
}
}

void Terminal::onMoveUpKey(SessionContext *s)
{
    if (s->history_index == s->history.size())
        return;

    CleanupInput(s);

    s->history_index++;
    s->curr_input = s->history[s->history.size() - s->history_index];
    s->cursor = s->curr_input.size();

    s->send(s->curr_input);
}

void Terminal::onMoveDownKey(SessionContext *s)
{
    if (s->history_index == 0)
        return;

    CleanupInput(s);

    s->history_index--;
    if (s->history_index > 0) {
        s->curr_input = s->history[s->history.size() - s->history_index];
        s->cursor = s->curr_input.size();
    } else {
        s->curr_input.clear();
        s->cursor = 0;
    }

    s->send(s->curr_input);
}

void Terminal::onMoveLeftKey(SessionContext *s)
{
    if (s->cursor == 0)
        return;

    s->cursor--;
    s->send(MOVE_LEFT_KEY);
}

void Terminal::onMoveRightKey(SessionContext *s)
{
    if (s->cursor >= s->curr_input.size())
        return;

    s->cursor++;
    s->send(MOVE_RIGHT_KEY);
}

void Terminal::onHomeKey(SessionContext *s)
{
    while (s->cursor != 0) {
        s->send(MOVE_LEFT_KEY);
        s->cursor--;
    }
}

void Terminal::onEndKey(SessionContext *s)
{
    while (s->cursor < s->curr_input.size()) {
        s->send(MOVE_RIGHT_KEY);
        s->cursor++;
    }
}

